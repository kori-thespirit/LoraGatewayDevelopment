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
    LORA_PROTOCOL_OK,
    LORA_PROTOCOL_ERR_PAYLOAD_OVERSIZE,
    LORA_PROTOCOL_ERR_CALLBACK_NULL,
    LORA_PROTOCOL_ERR_ALLOCATE_FAIL,
    LORA_PROTOCOL_ERR_ARGS_EXISTED,
    LORA_PROTOCOL_ERR_ARGS_INVALID,
    LORA_PROTOCOL_ERR_ARGS_EMPTY,
    LORA_PROTOCOL_ERR_CRC_INVALID,
    LORA_PROTOCOL_ERR_ARGS_LEN_NOT_MATCH,
} e_lora_protocol_err_t ;


typedef enum lora_function {
    LORA_FUNC_BROADCAST_ONLY,
    LORA_FUNC_ACTIVE_TRANSMIT,
    LORA_FUNC_RELAY_MESSAGE,
    LORA_FUNC_LISTEN_ONLY,
} e_lora_function_t;

/* For gateway only */
typedef struct lora_node_info {
    uint8_t address;
    e_lora_function_t func;
    char *node_name;
    void *pnext;
} st_lora_node_info_t;

typedef struct lora_protocol_header {
    uint8_t dest_addr; // Destination Lora address
    uint8_t src_addr; // Source Lora address device ()
    uint8_t payload_length;
    uint8_t request_data : 1;
    uint8_t function : 7;
} st_lora_protocol_header_t;

typedef struct lora_frame_data {
    st_lora_protocol_header_t header;
    void *payload;
} st_lora_frame_data_t;

typedef void (* p_lora_protocol_pack_complete_cb)(void *pvParameters);
typedef void (* p_lora_protocol_parse_complete_cb)(void *pvParameters, st_lora_protocol_header_t header);

e_lora_protocol_err_t m_lora_protocol_frame_parse(uint8_t *in_buf,
        uint16_t in_buf_size);
e_lora_protocol_err_t m_lora_protocol_frame_pack(uint8_t *out_buf, 
        uint16_t out_buf_size, 
        void* payload, 
        uint16_t payload_size, 
        uint8_t src_addr,
        uint8_t dest_addr,
        uint8_t request_data);
e_lora_protocol_err_t m_lora_protocol_register_callback(
        void (*p_lora_protocol_pack_complete_cb)(void *),
        void (*p_lora_protocol_parse_complete_cb)(void *, st_lora_protocol_header_t)) ;

e_lora_protocol_err_t m_set_lora_addr_to(uint8_t addr);
uint8_t m_get_lora_reply_addr();

#endif // _LORA_PROTOCOL_H_
