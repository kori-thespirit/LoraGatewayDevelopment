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

    
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.clk = 14;
    slot_config.cmd = 15;
    slot_config.d0 = 12;
    slot_config.d1 = 13;
    slot_config.d2 = 4;
    slot_config.d3 = 2;
    slot_config.cd = 45;
    slot_config.wp = -1;
    slot_config.width = 4;

    const char mount_point[] = "/sdcard";

    // Khai báo con trỏ để nhận kết quả từ hàm mount
    sdmmc_card_t* card;

    const esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false, .max_files = 5, .allocation_unit_size = 16 * 1024};

    ESP_LOGI(TAG, "Mounting SD card...");

    //  HÀM ĐỂ KHỞI TẠO TẤT CẢ
    ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SD card (%s)", esp_err_to_name(ret));
        return;
    }

    // Sau khi mount thành công, biến 'card' đã chứa thông tin thẻ
    sdmmc_card_print_info(stdout, card);

    // Ghi file thử nghiệm
    ESP_LOGI(TAG, "Writing file...");
    FILE* f = fopen("/sdcard/hello.txt", "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
    } else {
        fprintf(f, "Hello SD card!\n");
        fclose(f);
        ESP_LOGI(TAG, "File written successfully");
    }
    // Đọc file vừa ghi
    ESP_LOGI(TAG, "Reading file...");
    f = fopen("/sdcard/hello.txt", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
    } else {
        char line[64];
        // Đọc từng dòng từ file
        if (fgets(line, sizeof(line), f) != NULL) {
            // Loại bỏ ký tự xuống dòng nếu có
            char* pos = strchr(line, '\n');
            if (pos) *pos = '\0';
            ESP_LOGI(TAG, "Read from file: '%s'", line);
        }
        fclose(f);
    }

    // Unmount khi hoàn tất
     esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "Card unmounted");

    printf("LOG END\r\n");
}
