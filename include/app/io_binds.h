#pragma once

#include "appdata.h"
#include "apptypes.h"

namespace rc_binds
{

    using cmd_t = app::types::app_cmd_queue_t::cmd_t;
    using app::app_data_t;

    void arm_state(uint8_t val) {
        app_data_t& apd = app_data_t::get_instanse();
        cmd_t cmd = cmd_t::disarm;
        if (val)
        {
            cmd = cmd_t::arm;
        }
        xQueueSend(apd._queue._q, &cmd, 30);
    }

    void find_mode(uint8_t val) {
        app_data_t& apd = app_data_t::get_instanse();
        cmd_t cmd = cmd_t::find_off;
        if (val)
        {
            cmd = cmd_t::find_on;
        }
        xQueueSend(apd._queue._q, &cmd, 30);
    }
} // namespace binds
