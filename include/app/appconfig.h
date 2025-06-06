#pragma once

#include <stdint.h>
#include "soc/gpio_num.h"
// ===================== minimal requared ===================== 

#define UFO_WIFI
#define UFO_WIFI_DEFAULT_START_AP
// #define UFO_WIFI_DEFAULT_START_STA

// #define UFO_I2C_SOFT     // use i2c second driver
#define UFO_SPI             // use spi
#define UFO_SPI_CNT 1       // 2 max
#define UFO_UART_CNT 2      // 3 max
// ============================================================ 



// place it here because it is not app-types, but it will use in appconffig
// =========================== io ===========================

    enum class rc_digital_io_t : uint8_t {
        swb =   0b0000,     // binary code of pcf-board pads
        swa =   0b0001,
        swc1 =  0b0010,
        swd =   0b0011,
        swc2 =  0b0100,
        max
    };

    enum class rc_analog_io_t : uint8_t {
        vra,
        vrb,
        max
    };
    
// ==========================================================


namespace app
{
    class appconfig
    {
    private:
        /* data */
    public:
        // !(swa) | !(swb) |    | O(VRA) | O(VRB) |     | !3(swc) | !(swd) |
        static constexpr uint16_t io_perif_cnt = 
            static_cast<uint16_t>(rc_digital_io_t::max) + 
            static_cast<uint16_t>(rc_analog_io_t::max);

        static constexpr int16_t gimb_throt_min = 0; 
        static constexpr int16_t gimb_throt_max = 2048; 
        static constexpr int16_t gimb_angle_min = -1024; 
        static constexpr int16_t gimb_angle_max = 1024; 

        static constexpr int16_t gimb_throt_invert = 1; 
        static constexpr int16_t gimb_pitch_invert = -1;    // -1 if inverted
        static constexpr int16_t gimb_roll_invert = -1; 
        static constexpr int16_t gimb_yaw_invert = 1; 

        static constexpr gpio_num_t pin_display _dc = gpio_num_t::GPIO_NUM_26;
        static constexpr gpio_num_t pin_display _rst = gpio_num_t::GPIO_NUM_27;
        static constexpr gpio_num_t pin_display _cs = gpio_num_t::GPIO_NUM_12;

    };

} // namespace app
