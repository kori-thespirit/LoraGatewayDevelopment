#include "lora_protocol.h"

#include "esp_log.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static const char *TAG = "lora_protocol";
static p_lora_protocol_pack_complete_cb _g_p_pack_cb;
static p_lora_protocol_parse_complete_cb _g_p_parse_cb;

e_lora_protocol_err_t m_lora_protocol_frame_parse(uint8_t *in_buf, 
                                                  uint16_t in_buf_size
                                                  ) {
  M_LORA_ASSERT(_g_p_parse_cb, LORA_PROTOCOL_ERR_CALLBACK_NULL);
  st_lora_protocol_header_t *header = (st_lora_protocol_header_t *)(in_buf + 0);
  uint8_t request_data = header->request_data;
  uint8_t function = header->function;
  uint8_t payload_length = header->payload_length;
  uint8_t node_id = header->node_id;
  uint8_t *payload = (uint8_t *)calloc(payload_length, sizeof(uint8_t)); 
  if (!request_data) {
    memcpy(payload, in_buf + sizeof(st_lora_protocol_header_t), payload_length);
  }
  memset(in_buf, 0, in_buf_size);
  _g_p_parse_cb((void *)payload);
  free(payload);
  return LORA_PROTOCOL_ERR_OK;
}

e_lora_protocol_err_t m_lora_protocol_frame_pack(uint8_t *out_buf, 
                                                uint16_t out_buf_size, 
                                                void* payload, 
                                                uint16_t payload_size, 
                                                uint8_t node_id,
                                                uint8_t request_data) {

  M_LORA_ASSERT(MIN(out_buf_size, MAX_LORA_PACKET) >
                   sizeof(st_lora_protocol_header_t) 
                   + payload_size 
                   + FRAME_CRC_SIZE,
               LORA_PROTOCOL_ERR_PAYLOAD_OVERSIZE);
  M_LORA_ASSERT(_g_p_pack_cb, LORA_PROTOCOL_ERR_CALLBACK_NULL);
  st_lora_protocol_header_t header;
  uint8_t frame_length = 0;
  memset(out_buf, 0, out_buf_size);
  header.node_id = node_id & 0x07;
  header.request_data = request_data & 0x01;
  header.function = 7 & 0x7F;
  if (request_data)
    header.payload_length = 0;
  else
    header.payload_length = payload_size;
  memcpy(out_buf, (const void *)&header, sizeof(header));
  frame_length += sizeof(header);
  memcpy(out_buf + sizeof(header), (const void *)payload, header.payload_length);
  frame_length += header.payload_length;
  for(uint8_t i = 0; i < frame_length; i++) {
    ESP_LOGI(TAG, "0x%x",*(out_buf + i));
  }
  // frame_length += FRAME_CRC_SIZE;
  _g_p_pack_cb((void *)&frame_length);
  return LORA_PROTOCOL_ERR_OK;
}

e_lora_protocol_err_t m_lora_protocol_register_callback(void (*p_lora_protocol_pack_complete_cb)(void *),
                   void (*p_lora_protocol_parse_complete_cb)(void *)) {
  _g_p_pack_cb = p_lora_protocol_pack_complete_cb;
  _g_p_parse_cb = p_lora_protocol_parse_complete_cb;
  return LORA_PROTOCOL_ERR_OK;
}

