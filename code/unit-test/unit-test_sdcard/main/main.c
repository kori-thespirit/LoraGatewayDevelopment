#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <stdint.h>
#include "esp_log.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "driver/sdmmc_types.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"

static const char *TAG = "SD_CARD";

void app_main(void) {
  printf("LOG START\r\n");
  esp_err_t ret;

    // SDMMC host configuration
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    // Slot configuration with your custom pins
    sdmmc_slot_config_t slot_config = {
        .clk = 14,   // default pin for CLK
        .cmd = 15,                  // GPIO15 → SDIO_CMD
        .d0  = 12,                  // GPIO12 → SDIO_DATA0
        .d1  = 13,                  // GPIO13 → SDIO_DATA1
        .d2  = 4,                   // GPIO4  → SDIO_DATA2
        .d3  = 2,                   // GPIO2  → SDIO_DATA3
        .cd  = 45,                  // GPIO45 → SDIO_DET (Card Detect)
        .wp  = -1,                   // no write-protect pin
        .width = 4,                  // 4-bit bus width
    };

    // Mount point
    const char mount_point[] = "/sdcard";
    ESP_LOGI(TAG, "Mounting SD card...");

    sdmmc_card_t *card;
    const esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    esp_err_t ret = sdmmc_host_init();
    if (ret == ESP_OK) {
        ret = sdmmc_host_init_slot(SDMMC_HOST_SLOT_1, &slot_config);
    }
    if (ret == ESP_OK) {
        ret = sdmmc_card_init(&host, &card);
    }

    if (ret == ESP_OK) {
        sdmmc_card_print_info(stdout, &card);
    } else {
        ESP_LOGE(TAG, "Card not detected or init failed: %s", esp_err_to_name(ret));
    }

    ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SD card (%s)", esp_err_to_name(ret));
        return;
    }


    // Print card info
    sdmmc_card_print_info(stdout, card);

    // Example: create a file
    FILE *f = fopen("/sdcard/hello.txt", "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "Hello SD card!\n");
    fclose(f);
    ESP_LOGI(TAG, "File written successfully");

    // Unmount when done
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "Card unmounted");

    printf("LOG END\r\n");
}
