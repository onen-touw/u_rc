#pragma once

#include "u_sys/thread.h"
#include "u_sys/list.h"

#include "u_sys/fsk.h"
#include "u_dev/pcf8575.h"
#include "appdata.h"

// sensor control class
class sens_t
{
    // using slist_t = ufo::list_t<ufo::thread_guard>;
// private:
    // slist_t _list;

    ufo::drv::UFO_I2C_Driver* _driver = nullptr;
    net_t::msg_block_t _msg;
public:

    sens_t(ufo::drv::UFO_I2C_Driver* drv, net_t::msg_block_t msg) : _msg(msg){
        if (drv->Initialized())
        {
            _driver = drv;
        }
    }

    ~sens_t() {
    
    }

    // private:
public:

    void task_pcf8575(ufo::token_t token) {
        pcf8575_t ioe(_driver, 0x22);
        
        app::app_data_t& _app = app::app_data_t::get_instanse();

        while (token)
        {
            ioe.Update();
            // _msg->fMsg("vzik_bar@%d=%.3f;%.3f\n", ufo::utl::get_time_millis(), data.Presure, data.Tempreture);
            // ufo::Trace_t::flog("baro: %.3f,%.3f,\n", data.Presure, data.Tempreture);
            // ufo::Trace_t::flog(">b:%.3f, T:%.3f\n", data.Presure, data.Tempreture);
            {
                _app._tumb = ioe.Get();
            }
            ufo::utl::sleep_for(50);
        }
    }


};