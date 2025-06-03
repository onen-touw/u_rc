#pragma once

#include "appdata.h"
#include "apptypes.h"
#include "appconfig.h"

#include "u_sys/utils.h"
#include "u_sys/mutex.h"
#include "u_sys/cns.h"

void console_echo(ufo::cns::console_t::block_t block)
{
    using namespace ufo;
    vector_t<string_t> &arg_list = block->get_buf();
    app::app_data_t &_app = app::app_data_t::get_instanse();

    if (!arg_list.empty())
    {
        if (arg_list.size() > 1)
        {
            cns::opt_t opt(arg_list[1]);

            uint16_t f = 50;
            if (opt.arg_count() == 1)
            {
                f = opt.get_arg<uint16_t>(0);
                if (!f || f > 1000)
                {
                    f = 50;
                }
                block->fwrite("change freq to %ums\n", f);
            }

            if (opt == 'g' || opt == "gimb")
            {
                while (!block->is_read_out_signal())
                {
                    {
                        if (_app._gimb._ready)
                        {
                            block->fwrite(">t:%d\n>r:%d\n>p:%d\n>y:%d\n\n",
                                          _app._gimb._throt.get(),
                                          _app._gimb._roll.get(),
                                          _app._gimb._pitch.get(),
                                          _app._gimb._yaw.get());
                        }
                    }
                    utl::sleep_for(f);
                }
            }
            else if (opt == 't' || opt == "tumb")
            {

                ufo::bit_flag_t<uint16_t> prev;

                while (!block->is_read_out_signal())
                {
                    {
                        if (prev != _app._tumb)
                        {
                            for (size_t i = 0; i < app::appconfig::io_perif_cnt + 1u; i++)
                            {
                                bool val = _app._tumb.get(i);
                                if (val != prev.get(i))
                                {
                                    block->fwrite(">t%u:%u\n", i, val);
                                }
                            }
                            prev = _app._tumb;
                            block->write("\n");
                        }
                    }
                    utl::sleep_for(f);
                }
            }
        }
    }
}
