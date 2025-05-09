#pragma once

#include "u_sys/config.h" // only this include
#include "u_sys/mutex.h"
#include "u_sys/btflg.h"

// this file describes only data structures for
// sensors/devices and other in simple types

namespace app
{
    namespace types
    {
        enum class mot_cmd_t
        {
            mot_no,
            mot_set_arm,
            mot_set_disarm,
            mot_vals,
            mot_valscns, // t== 1, r == 2, p == 3, y == 4
        };

        struct gimb_data_t
        {
            float _throt = 0.f;
            float _roll = 0.f;
            float _pitch = 0.f;
            float _yaw = 0.f;
            bool _ready = false;
            ufo::mutex_t _lock;
        };

    }
} // namespace app
