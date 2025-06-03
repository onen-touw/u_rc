#pragma once 

#include "u_sys/config.h"
#include "u_sys/utils.h"
#include "u_sys/error.h"
#include "u_sys/thread.h"
#include "u_sys/mutex.h"

#include <driver/adc.h>

#include "app/appdata.h"
#include "app/appconfig.h"

class gimball_t
{
private:
    adc1_channel_t _chan = adc1_channel_t::ADC1_CHANNEL_MAX;

    int16_t _prev = 0;
    float alpha = 0.6f;
public:
    static constexpr uint16_t smpl_out = 15;

public:

    gimball_t(){}

    gimball_t(gpio_num_t gpio, adc_atten_t at = ADC_ATTEN_DB_12) {

        switch (gpio)
        {
        case gpio_num_t::GPIO_NUM_36:
            _chan = adc1_channel_t::ADC1_CHANNEL_0;
            break;
        case gpio_num_t::GPIO_NUM_37:
            _chan = adc1_channel_t::ADC1_CHANNEL_1;
            break;
        case gpio_num_t::GPIO_NUM_38:
            _chan = adc1_channel_t::ADC1_CHANNEL_2;
            break;
        case gpio_num_t::GPIO_NUM_39:
            _chan = adc1_channel_t::ADC1_CHANNEL_3;
            break;
        case gpio_num_t::GPIO_NUM_32:
            _chan = adc1_channel_t::ADC1_CHANNEL_4;
            break;
        case gpio_num_t::GPIO_NUM_33:
            _chan = adc1_channel_t::ADC1_CHANNEL_5;
            break;
        case gpio_num_t::GPIO_NUM_34:
            _chan = adc1_channel_t::ADC1_CHANNEL_6;
            break;
        case gpio_num_t::GPIO_NUM_35:
            _chan = adc1_channel_t::ADC1_CHANNEL_7;
            break;
        
        default:
            _chan = adc1_channel_t::ADC1_CHANNEL_MAX;
            break;
        }

        esp_err_t e = adc1_config_channel_atten(_chan, at);

        if (e!=ESP_OK)
        {
            ufo::Error_t& _error = ufo::Error_t::GetInstance();
            _error.Push(ufo::CriticalError_t(GenerateInfo_Code(ufo::error::codes_t::uninit_nullptr, "uda was null")));
            return;
        }
    }
    ~gimball_t() {}

    int16_t read(){
        
        
        // filtering
        // for (uint16_t i = 0; i < smpl_out; i++)
        // {
            // adc += adc1_get_raw(_chan);
        // }
        // uint16_t adc_ave = adc / smpl_out;

        _prev = _prev * alpha + adc1_get_raw(_chan) * (1.f - alpha);

        return _prev;
    }
};


class gimball4_t
{
private:

    gimball_t _throt;
    gimball_t _pitch;
    gimball_t _roll;
    gimball_t _yaw;

public:
    gimball4_t() {

        _throt = gimball_t(gpio_num_t::GPIO_NUM_32);
        _pitch = gimball_t(gpio_num_t::GPIO_NUM_35);
        _roll = gimball_t(gpio_num_t::GPIO_NUM_34);
        _yaw = gimball_t(gpio_num_t::GPIO_NUM_33);
    }

    void task(ufo::token_t token) {
        using app::appconfig;
        app::app_data_t &appd = app::app_data_t::get_instanse();
        
        while (token)
        {
            {
                ufo::lock_guard<ufo::mutex_t> _l(appd._gimb._lock);
                appd._gimb._throt.upd(_throt.read(), appconfig::gimb_throt_min, 1024, appconfig::gimb_throt_max, appconfig::gimb_throt_invert);
                appd._gimb._roll.upd(_roll.read(), appconfig::gimb_angle_min, 0, appconfig::gimb_angle_max, appconfig::gimb_roll_invert);
                appd._gimb._pitch.upd(_pitch.read(), appconfig::gimb_angle_min, 0, appconfig::gimb_angle_max, appconfig::gimb_pitch_invert);
                appd._gimb._yaw.upd(_yaw.read(), appconfig::gimb_angle_min, 0, appconfig::gimb_angle_max, appconfig::gimb_yaw_invert);
                appd._gimb._ready = true;
            }
            ufo::utl::sleep_for(20);
        }
    }
    ~gimball4_t() {}
};