#pragma once

#include "apptypes.h"
#include "appdata.h"
#include "appconfig.h"
#include "type_traits"

#include "u_sys/sdt.h"

#include "u_sys/utils.h"
#include "u_sys/thread.h"

#include "u_dev/pcf8575.h"
#include "u_drivers/i2c/UFO_I2C_driver.h"

class bind_obj_t
{
public:
    using bind_t = void(*)(uint8_t);
private:
    uint8_t _val = 0;
    uint8_t _min = 0;
    uint8_t _max = 0;
    bind_t _bind = nullptr;

public:
    bind_obj_t(){};
    bind_obj_t(bind_t func, uint16_t min, uint16_t max) 
        : _min(min), _max(max), _bind(func) {}
    ~bind_obj_t() {}

    // io-task call this foo when iterate on io_cnt when pcf-data is ready
    void process(uint8_t val) {
        if (_val != val)
        {
            _val = ufo::utl::constrain(val, _min, _max);
            if (_bind){
                _bind(_val);
            }
        }
    }

    uint16_t get() const {
        return _val;
    }

    bool chk() const {
        return _min != _max && _bind;
    }
};

class rc_io_t
{
public:
    using bind_t = bind_obj_t::bind_t;
private:
    bind_obj_t _bind[app::appconfig::io_perif_cnt] = {};
    ufo::drv::UFO_I2C_Driver* _drv = nullptr;

public:
    rc_io_t(ufo::drv::UFO_I2C_Driver* drv) : _drv(drv) {}
    ~rc_io_t() {}

    // bind_t - func that send app_event in app_queue_cmd 
    void mk_bind(rc_digital_io_t io, bind_t bind, uint8_t min, uint8_t max = UINT8_MAX) {
        if (io == rc_digital_io_t::max)
        {
            return;
        }

        if (!bind)
        {
            return;
        }
        
        _bind[static_cast<uint16_t>(io)] = bind_obj_t(bind, min, max);
    }

    // bind_t - func that send app_event in app_queue_cmd 
    void mk_bind(rc_analog_io_t io, bind_t bind, uint8_t min, uint8_t max = UINT8_MAX) {
        if (io == rc_analog_io_t::max)
        {
            return;
        }

        if (!bind)
        {
            return;
        }
        
        _bind[static_cast<uint16_t>(io) + static_cast<uint16_t>(rc_digital_io_t::max)] = bind_obj_t(bind, min, max);
    }

    void rm_bind(rc_digital_io_t io) {
        if (io == rc_digital_io_t::max)
        {
            return;
        }
        _bind[static_cast<uint16_t>(io)] = bind_obj_t(nullptr, 0, 0);
    }

    void rm_bind(rc_analog_io_t io) {
        if (io == rc_analog_io_t::max)
        {
            return;
        }
        _bind[static_cast<uint16_t>(io) + static_cast<uint16_t>(rc_digital_io_t::max)] = bind_obj_t(nullptr, 0, 0);
    }

    void task(ufo::token_t token){

        if (!_drv)
        {
            return;
        }
        pcf8575_t pcf(_drv, 0x22);
        // other io-sens
        pcf.InitSensor();
        
        pcf8575_t::state_t prev;

        while (token)
        {
            // interupt - check
            pcf.Update();
            pcf8575_t::state_t input = pcf.Get();
            if (input != prev)
            {
                app::app_data_t::get_instanse()._tumb = input;
                for (size_t i = 0; i < app::appconfig::io_perif_cnt + 1u; ++i)
                {
                    bool val = input.get(i);
                    if (val != prev.get(i))
                    {
                        _bind[i].process(val);
                    }
                }
                prev = input;
            }
            ufo::utl::sleep_for(75);

            // place code for ads1115 here
        }
    }
};