#include "STM32_Slave.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "hal/spi_types.h"
#include "portmacro.h"
#include "stdio.h"
#include <string.h>

static const char *tag = "slave stm32";
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
typedef struct STM32_Slave_Context_t {
  SPI_config cfg;
  SemaphoreHandle_t ready_sem;    // Smeaphore readyy for signal
  spi_device_handle_t spi_handle; ///< SPI device handle
} STM32_Slave_Context_t;

/* Private function prototypes -----------------------------------------------*/
static void printfnumbinary(uint8_t *data, size_t len);
static esp_err_t Check_Pointer(STM32_Slave_handle_t *ctx) {
  if (ctx == NULL || *ctx == NULL) {
    return ESP_ERR_INVALID_ARG;
  }
  return ESP_OK;
}
static void cs_high(STM32_Slave_handle_t *ctx) {

  ESP_LOGI(tag, "CS HIGH");
  gpio_set_level(((*ctx)->cfg.CS), 1);
}
static void cs_low(STM32_Slave_handle_t *ctx) {

  ESP_LOGI(tag, "CS LOW");
  gpio_set_level(((*ctx)->cfg.CS), 0);
}
static esp_err_t SPI_init_bus(STM32_Slave_handle_t *ctx, uint16_t max_transfer);
static esp_err_t SPI_add_device(STM32_Slave_handle_t *ctx, SPI_config *cfg);
/* Public function prototypes -----------------------------------------------*/
esp_err_t STM32_Slave_Deinit(STM32_Slave_handle_t *ctx) {
  ESP_LOGI(tag, "\n### DeINIT SLAVE PIN ###\n\n");
  esp_err_t err = ESP_OK;
  err = Check_Pointer(ctx);
  if (err != ESP_OK)
    return err;
  if ((*ctx)->spi_handle == NULL)
    return ESP_ERR_INVALID_ARG;
  err = spi_bus_remove_device((*ctx)->spi_handle);
  if (err != ESP_OK) {
    return err;
  }
  if ((*ctx)->cfg.host >= SPI1_HOST && (*ctx)->cfg.host <= SPI3_HOST) {
    spi_bus_free((*ctx)->cfg.host);
  } else
    return ESP_ERR_INVALID_ARG;

  free(*ctx);
  *ctx = NULL;
  ESP_LOGI(tag, "\n### DeINIT SLAVE PIN successfully ###\n\n");
  return ESP_OK;
}
esp_err_t STM32_Slave_Init(STM32_Slave_handle_t *ctx, SPI_config *cfg) {
  esp_err_t err = ESP_OK;
  if (ctx == NULL || cfg == NULL) {
    return ESP_ERR_INVALID_ARG;
  }
  if (*ctx != NULL) {
    ESP_LOGE(tag, "Already STM32 Slave Init\n");
    return ESP_ERR_INVALID_ARG;
  }
  *ctx = malloc(sizeof(STM32_Slave_Context_t));
  if (*ctx == NULL) {
    ESP_LOGE(tag, "khong the cap phat bo nho\n");
    return ESP_ERR_NO_MEM;
  }
  // memset((*ctx), 0, sizeof(STM32_Slave_Context_t));
  memcpy(&(*ctx)->cfg, cfg, sizeof(SPI_config));
  ESP_LOGI(tag, "\n### INIT SLAVE PIN ###\n\n");
  ESP_LOGI(tag, "SCLK Pin: %d\n", (*ctx)->cfg.SCK);
  ESP_LOGI(tag, "MOSI Pin: %d\n", (*ctx)->cfg.MOSI);
  ESP_LOGI(tag, "MISO Pin: %d\n", (*ctx)->cfg.MISO);
  ESP_LOGI(tag, "CS Pin: %d\n", (*ctx)->cfg.CS);
  ESP_LOGI(tag, "\n### END INIT SLAVE PIN ###\n\n");

  err = SPI_init_bus(ctx, 32);
  if (err != ESP_OK)
    goto clean;
  ESP_LOGI(tag, "SPI bus initialized successfully\n");
  err = SPI_add_device(ctx, &((*ctx)->cfg));
  if (err != ESP_OK)
    goto clean;
  ESP_LOGI(tag, "SPI device added successfully\n");
  cs_high(ctx);
  return ESP_OK;
clean:
  ESP_LOGE(tag, "Failed to Init Slave device: %s", esp_err_to_name(err));
  spi_bus_remove_device((*ctx)->spi_handle);
  free(*ctx);
  *ctx = NULL;
  return ESP_ERR_INVALID_ARG;
}
static esp_err_t SPI_init_bus(STM32_Slave_handle_t *ctx,
                              uint16_t max_transfer) {

  esp_err_t err = ESP_OK;
  err = Check_Pointer(ctx);
  if (err != ESP_OK)
    return err;
  if (max_transfer > 4092) {
    ESP_LOGE(tag, "Warning Transfer is too high\n");
  }
  ESP_LOGI(tag, "Initializing bus SPI...");
  gpio_config_t io_conf = {.mode = GPIO_MODE_OUTPUT,
                           .pin_bit_mask = (1ULL << (*ctx)->cfg.CS)};
  gpio_config(&io_conf);

  spi_bus_config_t buscfg = {.miso_io_num = (*ctx)->cfg.MISO,
                             .mosi_io_num = (*ctx)->cfg.MOSI,
                             .sclk_io_num = (*ctx)->cfg.SCK,
                             .quadwp_io_num = -1,
                             .quadhd_io_num = -1,
                             .max_transfer_sz = max_transfer};

  err = spi_bus_initialize((*ctx)->cfg.host, &buscfg, SPI_DMA_CH_AUTO);
  return err;
}
static esp_err_t SPI_add_device(STM32_Slave_handle_t *ctx, SPI_config *cfg) {

  esp_err_t err;
  err = Check_Pointer(ctx);
  if (err != ESP_OK)
    return err;
  spi_device_interface_config_t devcfg = {
      .clock_speed_hz = SPI_Frequency,
      .command_bits = 0,
      .queue_size = 1,
      .spics_io_num = -1, //  CS pin
      .dummy_bits = 0,
      .mode = 0,
  };
  err = spi_bus_add_device((*ctx)->cfg.host, &devcfg, &((*ctx)->spi_handle));
  return err;
}
esp_err_t STM32_Begin_Get_Info(STM32_Slave_handle_t *ctx) {
  esp_err_t err;
  err = Check_Pointer(ctx);
  if (err != ESP_OK)
    return err;
  uint8_t tx[2] = {0x37, 0x80};
  uint8_t rx[2] = {0};
  spi_transaction_t trans = {
      .length = 8 * 2,
      .flags = 0,
      .tx_buffer = tx,
      .rx_buffer = rx,
  };

  cs_low(ctx);
  err = spi_device_transmit(((*ctx)->spi_handle), &trans);
  cs_high(ctx);
  // ESP_LOGI(tag, "RX: %02X %02X", rx[0], rx[1]);
  return err;
}
esp_err_t STM32_Slave_Set_Up(STM32_Slave_handle_t *ctx) {
  esp_err_t err;
  STM32_Slave_Register reg;
  memset(&reg, 0, sizeof(STM32_Slave_Register));
  reg.addr_reg = 0x37;
  reg.hummidity = 0x30;
  reg.temperature = 0x40;
  printfnumbinary((uint8_t *)&reg, sizeof(reg));
  err = Send_STM32_Slave_Register(ctx, (uint8_t *)&reg, sizeof(reg));
  return err;
}
esp_err_t Send_STM32_Slave_Register(STM32_Slave_handle_t *ctx, uint8_t *data,
                                    size_t len) {
  esp_err_t err;
  if (data == NULL) {
    ESP_LOGE(tag, "Error NULL DATA");
    return ESP_ERR_INVALID_ARG;
  }
  if (len > (4 * 8)) {
    ESP_LOGE(tag, "Error len > 4");
    return ESP_ERR_INVALID_ARG;
  }

  err = Check_Pointer(ctx);
  if (err != ESP_OK)
    ESP_LOGE(tag, "Error STM32 Slave Handle NULL\n");
  return err;

  // Implementation for sending register data
  spi_transaction_t trans = {
      .length = 8 * len,
      .flags = 0,
      .tx_buffer = data,
      .rx_buffer = NULL,
  };
  printfnumbinary(data, len);
  cs_low(ctx);
  err = spi_device_transmit(((*ctx)->spi_handle), &trans);
  cs_high(ctx);
  return err;
}
static void printfnumbinary(uint8_t *data, size_t len) {
  static char string[(4 * 8)];
  for (int i = 0; i < len; i++) {
    for (int j = 7; j >= 0; j--) {
      int k = *(data + i) >> j;
      if (k & 1) {
        // printf("1");
        strcat(string, "1");
      } else {
        // printf("0");
        strcat(string, "0");
      }
    }
    strcat(string, " ");
  }
  strcat(string, "\n");

  // printf("\n");
  ESP_LOGE(tag, "%s", string);
}
