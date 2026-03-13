#pragma once

#include "u_drivers/spi/uspi.h"

#include "u_sys/utils.h"
#include "u_sys/gpio.h"

#include "u_dev/st7789/st7789d.h"
#include "u_dev/st7789/tests.h"

#include "esp_spiffs.h"

class display_t
{
public:
    static constexpr gpio_num_t dc_pin = gpio_num_t::GPIO_NUM_26;
    static constexpr gpio_num_t rst_pin = gpio_num_t::GPIO_NUM_27;
    static constexpr gpio_num_t cs_pin = gpio_num_t::GPIO_NUM_12;

public:
    display_t(ufo::drv::uspi_t* spi_driver) 
    {
        {
            esp_vfs_spiffs_conf_t conf = {
                .base_path = "/spiffs",
                .partition_label = NULL,
                .max_files = 2,
                .format_if_mount_failed = false
            };
        
            // Use settings defined above to initialize and mount SPIFFS filesystem.
            // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
            esp_err_t ret = esp_vfs_spiffs_register(&conf);
        
            if (ret != ESP_OK) {
                if (ret ==ESP_FAIL) {
                    printf("Failed to mount or format filesystem\n");
                } else if (ret== ESP_ERR_NOT_FOUND) {
                    printf("Failed to find SPIFFS partition\n");
                } else {
                    printf("Failed to initialize SPIFFS (%s)\n",esp_err_to_name(ret));
                }
            }
            printf("Performing SPIFFS_check().\n");
            ret = esp_spiffs_check(conf.partition_label);
            if (ret != ESP_OK) {
                printf("SPIFFS_check() failed\n");
            }
            ufo::utl::sleep_for(1);

            size_t total = 0, used = 0;
            ret = esp_spiffs_info(conf.partition_label, &total, &used);
            if (ret != ESP_OK) {
                printf("Failed to get SPIFFS partition\n");
            }
            printf("Mount %s success\n", conf.base_path);
            printf("Partition size: total: %u, used: %u, free %u\n", total, used, total - used);
        }

        printf("display start\n");
        st7789_t _display(spi_driver, dc_pin, rst_pin, cs_pin);

        font_t font("/spiffs/ILGH24XB.FNT");
        font.open();
        font.log_struct();

        _display.init();
        
        ufo::utl::sleep_for(200);
        // DISPLAY_TEST_fill_screen(&_display);

        // _display.fill_screen(st7789_t::basic_RED);
        // _display.display();
        // printf("draw char dir 0\n\n");
        // _display.draw_char(&font, 0, 0, 'A', st7789_t::basic_GREEN, st7789_t::direction_t::DIRECTION0);
        _display.draw_string(&font, 80, 50 , "string", 6, st7789_t::basic_GREEN, st7789_t::direction_t::DIRECTION0);
        _display.display();
        ufo::utl::sleep_for(1000);
    
        _display.draw_line(0, 0, 240, 320, st7789_t::basic_GREEN);
        ufo::utl::sleep_for(1000);

    }
    ~display_t() {}
};