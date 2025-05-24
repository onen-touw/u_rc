#pragma once

#include "u_sys/config.h"
#include "u_drivers/i2c/UFO_I2C_Interface.h"
#include "u_sys/trace.h"
#include "u_sys/error.h"
#include "u_sys/utils.h"
#include "u_sys/btflg.h"

class pcf8575_t : private ufo::UFO_I2C_Interface
{
public:
    using state_t = ufo::bit_flag_t<uint16_t>;
private:
    state_t _state;
public:
    pcf8575_t(ufo::drv::UFO_I2C_Driver* driver, uint8_t addr = 0x22){
        esp_err_t err = this->Init(addr, driver);
        if (err != ESP_OK)
        {
            ufo::Trace_t::log("driver initializing priblem\n");
            // SetCritical;      //think about it
        }
    }
    ~pcf8575_t(){}

    void InitSensor(){
        Write16_noreg(0);
    }
    
    void Update()
    {
        uint16_t r = Read16();

        _state.upd();
    }
    const state_t& Get() const
    {
        return _state;
    }
};
