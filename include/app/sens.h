#pragma once

#include "u_sys/thread.h"
#include "u_sys/list.h"

#include "u_sys/fsk.h"

// sensor control class
class sens_t
{
    using slist_t = ufo::list_t<ufo::thread_guard>;
private:
    slist_t _list;
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

};