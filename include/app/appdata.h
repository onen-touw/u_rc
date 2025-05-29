#pragma once 

#include "apptypes.h"
#include "u_sys/btflg.h"

namespace app
{
        class app_data_t
        {
        public:
            // types::remote_data_t _remote = {};
            types::gimb_data_t _gimb = {};
            
            ufo::bit_flag_t<uint16_t> _tumb = {};

            types::event_t _event = {};
            types::app_cmd_queue_t _queue = {};
        private:
            app_data_t(){}

        public:
            static app_data_t &get_instanse()
            {
                static app_data_t i;
                return i;
            }

            app_data_t(app_data_t &) = delete;
            app_data_t &operator=(app_data_t &) = delete;

            app_data_t &operator=(app_data_t &&) = default;
            app_data_t(app_data_t &&) = default;
        };
} // namespace app

