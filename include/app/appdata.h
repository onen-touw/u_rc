#pragma once 



#include "apptypes.h"

namespace app
{
        class app_data_t
        {
        public:
            // types::remote_data_t _remote = {};
            types::gimb_data_t _gimb = {};

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

