#pragma once

#include "u_sys/config.h" // only this include
#include "u_sys/mutex.h"
#include "u_sys/btflg.h"

// this file describes only data structures for
// sensors/devices and other in simple types

namespace app
{

    enum remote_cmd_e : uint8_t {
        null,
        
        ask,
        req,

        arm,
        servo,
        trpy,
        find_mode,
    };

// =========================== events ===========================
    enum class app_event_e
    {
        null,

        idle,
        control,
        setting,
        alarm,
    };

    enum class app_event_control_e
    {
        idle,      // idle
        trpy,      // throt/roll/pitch/yaw
        arm_state, // arm/disarm cmd
        tunm,      // tumblers
        pot,       // potenciometrs
    };

    enum class app_event_alarm_e
    {
        null,

        battery,
        battery_crit,
        disconn,
        find_mode,

        warning,
        critical,
    };

// ==============================================================

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

        struct app_cmd_queue_t
        {
            enum class cmd_t {
                null,
                req,

                arm,
                disarm,
                find_on,
                find_off,

            };
            
            QueueHandle_t _q = nullptr;

            app_cmd_queue_t(){
                _q = xQueueCreate(2, sizeof(cmd_t));
                if (!_q)
                {
                    //
                }
            }
            ~app_cmd_queue_t(){
                if (_q)
                {
                    vQueueDelete(_q);
                }
            }
        };
        
        template <typename Ty> 
        struct event_base_t
        {
            Ty _event;
            Ty _prev;

            void set(Ty e) {
                _prev = _event;
                _event = e;
            }

            Ty get() const {
                return _event;
            }

            Ty get_prev() const {
                return _prev;
            }

            void back() {
                std::swap(_event, _prev);
            }

            bool operator == (const Ty& other) {
                return _event == other;
            }
        };

        struct event_t
        {
            // ufo::mutex_t _lock;
            event_base_t<app_event_e> _app = {};
            event_base_t<app_event_control_e> _control = {};
            event_base_t<app_event_alarm_e> _alarm = {};

            // event_subj_t<app_event_calibrate_e> _subj_calibrate = {};
        };
        
        struct gimb_stick_t{
            uint16_t _raw = 0;
            int16_t _min = 0;
            int16_t _mid = 0;
            int16_t _max = 0;
            int16_t _result = 0;
            int16_t _offset = 0;

            void upd(uint16_t val, const int16_t nmin, const int16_t nmid, const int16_t nmax, const int16_t inv)
            {
                _raw = val;
                if (_raw > _mid)
                {
                    _result = ufo::utl::constrain(static_cast<int16_t>(_raw), _mid, _max);
                    _result = ufo::utl::map(_result, _mid, _max, nmid, nmax) * inv;
                    return;
                }

                _result = ufo::utl::constrain(static_cast<int16_t>(_raw), _min, _mid);
                _result = ufo::utl::map(_result, _min, _mid, nmin, nmid) * inv;
            }

            int16_t get() const {
                return _result;
            }
            
        };

        struct gimb_data_t
        {
            
            gimb_stick_t _throt;
            gimb_stick_t _roll;
            gimb_stick_t _pitch;
            gimb_stick_t _yaw;
            bool _ready = false;
            ufo::mutex_t _lock;
        };

    }
} // namespace app
