#ifndef _LORA_PROTOCOL_H_
#define _LORA_PROTOCOL_H_

#include "node_device_typedef.h"
#include <stdint.h>

#define M_LORA_ASSERT_DEFINE(_cond, _ret)                                       \
  do {                                                                         \
    if (!(_cond)) {                                                            \
      return _ret;                                                             \
    }                                                                          \
  } while (0)

#define M_LORA_ASSERT(_cond, _ret) M_LORA_ASSERT_DEFINE(_cond, _ret)
#define FRAME_CRC_SIZE 2
#define MAX_LORA_PACKET 255
#define MIN(_x_, _y_) (_x_ < _y_ ? _x_ : _y_)


typedef enum lora_protocol_err {
  LORA_PROTOCOL_ERR_OK,
  LORA_PROTOCOL_ERR_PAYLOAD_OVERSIZE,
  LORA_PROTOCOL_ERR_CALLBACK_NULL,
  LORA_PROTOCOL_ERR_ARGS_ID_INVALID,
  LORA_PROTOCOL_ERR_ARGS_EXISTED,
  LORA_PROTOCOL_ERR_ARGS_EMPTY,
  LORA_PROTOCOL_ERR_CRC_INVALID,
  LORA_PROTOCOL_ERR_ARGS_LEN_NOT_MATCH,
} e_lora_protocol_err_t ;

typedef e_lora_protocol_err_t (* p_lora_protocol_pack_complete_cb)(void *pvParameters);
typedef e_lora_protocol_err_t (* p_lora_protocol_parse_complete_cb)(void *pvParameters);

typedef enum lora_function {
  LORA_FUNCTION_REQUEST_ADDRESS,
  LORA_FUNCTION_REQUEST_DATA,
  LORA_FUNCTION_RESPONSE_DATA,
} e_lora_function_t;

typedef struct lora_node_info {
  uint8_t node_id;
  uint8_t length;
  uint8_t readwrite;
  char node_name[10];
  void *payload;
  void *pnext;
} st_lora_node_info_t;

typedef struct lora_protocol_header {
  uint8_t node_id;
  uint8_t payload_length;
  uint8_t request_data : 1;
  uint8_t function : 7;
} st_lora_protocol_header_t;

typedef struct lora_frame_data {
  st_lora_protocol_header_t header;
  void *payload;
} st_lora_frame_data_t;

e_lora_protocol_err_t m_lora_protocol_frame_parse(uint8_t *in_buf,
                                                    uint16_t in_buf_size);

e_lora_protocol_err_t m_lora_protocol_frame_pack(uint8_t *out_buf, 
                                                uint16_t out_buf_size, 
                                                void* payload, 
                                                uint16_t payload_size, 
                                                uint8_t node_id,
                                                uint8_t request_data);

e_lora_protocol_err_t m_lora_protocol_register_callback(
                    e_lora_protocol_err_t (*p_lora_protocol_pack_complete_cb)(void *),
                    e_lora_protocol_err_t (*p_lora_protocol_parse_complete_cb)(void *)) ;


#endif // _LORA_PROTOCOL_H_
