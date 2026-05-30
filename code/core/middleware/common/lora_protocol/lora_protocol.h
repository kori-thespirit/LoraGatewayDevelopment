#ifndef _LORA_PROTOCOL_H_
#define _LORA_PROTOCOL_H_

#include <stdint.h>

#define M_LORA_ASSERT_DEFINE(_cond, _ret)                                       \
    do {                                                                        \
        if (!(_cond)) {                                                         \
            return _ret;                                                        \
        }                                                                       \
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

typedef void (* p_lora_protocol_pack_complete_cb)(void *pvParameters);
typedef void (* p_lora_protocol_parse_complete_cb)(void *pvParameters);

typedef enum lora_function {
  LORA_FUNCTION_REQUEST_ADDRESS, // NOTE:change to LORA_FUNC
  LORA_FUNCTION_REQUEST_DATA,
  LORA_FUNCTION_RESPONSE_DATA,
} e_lora_function_t;

typedef struct lora_node_info {
  uint8_t node_id;
  uint8_t length; // NOTE: remove this field
  uint8_t readwrite;
  char node_name[10]; // NOTE: rename to "name", change to pointer of char
  void *payload; // NOTE: remove this fiel
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

e_lora_protocol_err_t m_lora_protocol_frame_pack(uint8_t *out_buf, //trả về giá trị cho e_lora_protocol_err_t
                                                uint16_t out_buf_size, //nhận thgêm node_id(=1), length pay;load
                                                void* payload, //cái gateway nhận được từ node từ accs ngoại vi như biến tần, sht20
                                                uint16_t payload_size, 
                                                uint8_t node_id,
                                                uint8_t request_data);
void test();
uint8_t test1();
e_lora_protocol_err_t m_lora_protocol_register_callback(
                    void (*p_lora_protocol_pack_complete_cb)(void *),
                    void (*p_lora_protocol_parse_complete_cb)(void *)) ;


#endif // _LORA_PROTOCOL_H_
