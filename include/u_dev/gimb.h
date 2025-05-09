#pragma once 

#include "u_sys/config.h"
#include "u_sys/utils.h"
#include "u_sys/error.h"
#include "u_sys/thread.h"
#include "u_sys/mutex.h"

#include <driver/adc.h>

#include "app/appdata.h"


class gimball_t
{
private:
    adc1_channel_t _chan = adc1_channel_t::ADC1_CHANNEL_MAX;
    uint16_t 
        _min = 0,
        _max = 5000;
    float 
        _trg_min = 0.f,
        _trg_max = 0.f;

public:
// static constexpr uint16_t g_out_min = 0;
// static constexpr uint16_t g_out_max = 1000;
static constexpr uint16_t smpl_out = 10;

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

    void set_calibration(uint16_t min, uint16_t max){
        _min = min;
        _max = max;
    }

    void calibrate_min(uint16_t samples = 20){
        
        _min = calibrate(samples);
    }

    void calibrate_max(uint16_t samples = 20){
        _max = calibrate(samples);
    }

    uint16_t get_min() const {return _min;}
    uint16_t get_max() const {return _max;}

    uint16_t calibrate(uint16_t s){
        uint16_t i = 0;
        uint64_t val = 0;
        for (; i < s; ++i)
        {
            val += read();
        }
        return val /= i;
    }

    void set_range(float min, float max){
        _trg_min = min;
        _trg_max = max;
    }

    float read(){
        
        uint32_t adc = 0;
        
        // filtering
        for (uint16_t i = 0; i < smpl_out; i++)
        {
            adc += adc1_get_raw(_chan);
        }
        uint16_t adc_ave = adc / smpl_out;

        float r = ufo::utl::constrain(static_cast<float>(adc_ave), static_cast<float>(_min), static_cast<float>(_max));
        r = ufo::utl::map(r, static_cast<float>(_min), static_cast<float>(_max), _trg_min, _trg_max);
        return r;
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

        _throt.set_calibration(0, 4009);
        _pitch.set_calibration(0, 3866);
        _roll.set_calibration(108, 4095);
        _yaw.set_calibration(111, 4095);

        _throt.set_range(0.f, 1.f);
        _pitch.set_range(-1.f, 1.f);
        _roll.set_range(-1.f, 1.f);
        _yaw.set_range(-1.f, 1.f);
    }

    // void update(){
    //     ufo::lock_guard<ufo::mutex_t> _l(_uda->_lock);

    //     _uda->_throt = _throt.read();
    //     _uda->_roll =  _roll.read();
    //     _uda->_pitch = _pitch.read();
    //     _uda->_yaw = _yaw.read();
    // }

    void task(ufo::token_t token) {
        app::app_data_t &appd = app::app_data_t::get_instanse();

        while (token)
        {
            {
                ufo::lock_guard<ufo::mutex_t> _l(appd._gimb._lock);
                appd._gimb._throt = _throt.read();
                appd._gimb._roll = _roll.read() * -1.f;
                appd._gimb._pitch = _pitch.read()* -1.f;
                appd._gimb._yaw = _yaw.read();
                appd._gimb._ready = true;
            }
            ufo::utl::sleep_for(20);
        }
        
    }



            // printf("max_calibrate af5s\n");
			// ufo::utl::sleep_for(5000);
			// gimbb1.calibrate_max(30);
			// gimbb2.calibrate_max(30);
			// gimbb3.calibrate_max(30);
			// gimbb4.calibrate_max(30);
			// printf("ready\n");
			// ufo::utl::sleep_for(2000);


			// printf("min_calibrate af5s\n");
			// ufo::utl::sleep_for(5000);
			// gimbb1.calibrate_min(30);
			// gimbb2.calibrate_min(30);
			// gimbb3.calibrate_min(30);
			// gimbb4.calibrate_min(30);
			// printf("ready\n");


			// Trace_t::flog("result:\n");
			// Trace_t::flog("\t1: min: %u; max: %u\n", gimbb1.get_min(), gimbb1.get_max());
			// Trace_t::flog("\t2: min: %u; max: %u\n", gimbb2.get_min(), gimbb2.get_max());
			// Trace_t::flog("\t3: min: %u; max: %u\n", gimbb3.get_min(), gimbb3.get_max());
			// Trace_t::flog("\t4: min: %u; max: %u\n", gimbb4.get_min(), gimbb4.get_max());

    ~gimball4_t() {}
};