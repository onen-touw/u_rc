#pragma once

#include "u_sys/config.h"
#include "u_drivers/i2c/UFO_I2C_Interface.h"
#include "u_sys/trace.h"
#include "u_sys/error.h"
#include "u_sys/utils.h"
#include "u_sys/btflg.h"

class pcf8575_t : private ufo::UFO_I2C_Interface
{
private:
    uint32_t _osc_freq = 0;

private:

    enum Mode_e : uint8_t
    {
        mode1 = 0b0000'0000,
        mode2 = 0b0000'0001,
    }

    enum Mode1_Mask_e : uint8_t
    {
        ALL_CALL        = 0x01,
        SUB3            = 0x02,
        SUB2            = 0x04,
        SUB1            = 0x08,
        SLEEP           = 0x10,
        AUTO_INC        = 0x20
        EXT_CLK         = 0x40,
        RESTART         = 0x80,
    };
    enum Mode2_Mask_e : uint8_t
    {
        OUTNE_0 =   0x01,
        OUTNE_1 =   0x02,
        OUTDRV =    0x04,
        OCH =       0x08,
        INVRT =     0x10,
    };

    static constexpr uint8_t CH0 = 0x06;
    static constexpr uint8_t resolution = 12;   // DS
    static constexpr uint16_t max_pwm = 4096;
    static constexpr uint16_t min_pwm = 0;

public:
    enum class pca_mode_e
    {
        open_drain,
        push_pull,
    };

    static constexpr uint32_t ds_osc_dreq = 25000000;

public:
    pcf8575_t(ufo::drv::UFO_I2C_Driver *driver, uint8_t addr = 0x40)
    {
        esp_err_t err = this->Init(addr, driver);
        if (err!= ESP_OK)
        {
            //SetCritical;      //think about it
            ufo::Trace_t::log("driver initializing priblem\n");
        }
    }


    void InitSensor()
    {
        reset();
    }


    void reset() {
        write8(Mode_e::mode1, Mode1_Mask_e::RESTART);
        ufo::utl::sleep_for(10);
    }

    void setOscillatorFrequency(uint32_t freq) {
        if (freq > 25'000'000)
        {
            _osc_freq = ds_osc_dreq;
            return;
        }
        _osc_freq  = freq;
    }

    /// @brief 
    /// @param  pca_mode_t - enum : open_drain, push_pull
    /// open_drain - for generate signal only (no load on lines)
    /// push_pull  - PWM used as Power generator
    void set_out_mode(pca_mode_e mode)
    {
        uint8_t oldmode = this->Read8(Mode_e::mode2);
        uint8_t newmode = 0;
        if (mode == pca_mode_e::push_pull) {
            newmode = oldmode | Mode2_Mask_e::OUTDRV;
        } else {
            newmode = oldmode & ~Mode2_Mask_e::OUTDRV;
        }
        this->write8(Mode_e::mode2, newmode);
    }

    /// @brief 
    /// @param num 
    /// @param  off At what point in the 4096-part cycle to turn the PWM output OFF   
    uint16_t getPWM(uint8_t num, bool off)
    {
        uint8_t reg = uint8_t(CH0 + 4 * num);
        uint8_t buffer[2] = {};
        if (off)
            buffer[0] += 2;
        this->Read(reg, buffer, 2);
        return uint16_t(buffer[0]) | (uint16_t(buffer[1]) << 8);
    }
    /// @brief Sets the PWM output of one of the PCA9685 pins
    /// @param  num One of the PWM output pins, from 0 to 15
    /// @param  on At what point in the 4096-part cycle to turn the PWM output ON
    /// @param  off At what point in the 4096-part cycle to turn the PWM output OFF   
    void setPWM(uint8_t num, uint16_t on, uint16_t off)
    {
        if (num > 15)
        {
            return;
        }
        

        uint8_t buffer[5] = {};
        buffer[0] = CH0 + 4 * num;
        buffer[1] = on;
        buffer[2] = on >> 8;
        buffer[3] = off;
        buffer[4] = off >> 8;

        this->write(buffer, 5)
    }

    void setPin(uint8_t num, uint16_t val)
    {
        // Clamp value between 0 and 4095 inclusive.
        val = std::min(val, max_pwm);
        if (val == 4095)
        {
            // Special value for signal fully on.
            setPWM(num, 4096, 0);
        }
        else if (val == 0)
        {
            // Special value for signal fully off.
            setPWM(num, 0, 4096);
        }
        else
        {
            setPWM(num, 0, val);
        }
    }
}