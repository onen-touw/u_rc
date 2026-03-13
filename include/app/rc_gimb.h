#pragma once

#include "appdata.h"
#include "apptypes.h"
#include "appconfig.h"

#include "u_sys/utils.h"
#include "u_sys/mutex.h"
#include "u_sys/cns.h"

void console_gmb(ufo::cns::console_t::block_t& block)
{
    using namespace ufo;
    vector_t<string_t> &arg_list = block->get_buf();

    if (!arg_list.empty())
    {
        if (arg_list.size() > 1)
        {
            cns::opt_t opt(arg_list[1]);
            if (opt == 'c' || opt == "calibrate")
            {
                app::app_data_t &_app = app::app_data_t::get_instanse();
                uint16_t d = 50;

                if (opt.arg_count() == 1)
                {
                    d = opt.get_arg<uint16_t>(0);
                    if (!d || d > 50)
                    {
                        d = 20;
                    }
                    block->fwrite("sempl cnt: %ums\n", d);
                }

                {
                    block->fwrite("calibrating:\n\t");
                    uint64_t sumT = 0;
                    uint64_t sumP = 0;
                    uint64_t sumR = 0;
                    uint64_t sumY = 0;

                    block->fwrite("turn into min and wait end msg\n");
                    ufo::utl::sleep_for(5000);
                    block->fwrite("start\n");
                    {
                        for (int32_t i = 0; i < d; i++)
                        {
                            ufo::lock_guard<ufo::mutex_t> _l(_app._gimb._lock);

                            if (!_app._gimb._ready)
                            {
                                --i;
                                continue;
                            }

                            sumT += _app._gimb._throt._raw;
                            sumP += _app._gimb._pitch._raw;
                            sumR += _app._gimb._roll._raw;
                            sumY += _app._gimb._yaw._raw;
                            _app._gimb._ready = false;

                            block->fwrite(">t:%llu\n>p:%llu\n>r:%llu\n>y:%llu\n", sumT, sumP, sumR, sumY);

                            ufo::utl::sleep_for(25);
                        }

                        ufo::lock_guard<ufo::mutex_t> _l(_app._gimb._lock);
                        _app._gimb._throt._min = sumT / d;
                        _app._gimb._pitch._min = sumP / d;
                        _app._gimb._roll._min = sumR / d;
                        _app._gimb._yaw._min = sumY / d;
                        block->fwrite("result(min):\tt:%u; p:%u; r:%u; y:%u\n\n",
                                      _app._gimb._throt._min,
                                      _app._gimb._pitch._min,
                                      _app._gimb._roll._min,
                                      _app._gimb._yaw._min);
                    }
                    block->fwrite("end\n");

                    sumT = 0;
                    sumP = 0;
                    sumR = 0;
                    sumY = 0;

                    block->fwrite("turn into max and wait end msg\n");
                    ufo::utl::sleep_for(5000);
                    {
                        for (int32_t i = 0; i < d; i++)
                        {
                            ufo::lock_guard<ufo::mutex_t> _l(_app._gimb._lock);

                            if (!_app._gimb._ready)
                            {
                                --i;
                                continue;
                            }

                            sumT += _app._gimb._throt._raw;
                            sumP += _app._gimb._pitch._raw;
                            sumR += _app._gimb._roll._raw;
                            sumY += _app._gimb._yaw._raw;
                            
                            block->fwrite(">t:%llu\n>p:%llu\n>r:%llu\n>y:%llu\n\n", sumT, sumP, sumR, sumY);
                            _app._gimb._ready = false;
                            ufo::utl::sleep_for(25);
                        }

                        ufo::lock_guard<ufo::mutex_t> _l(_app._gimb._lock);
                        _app._gimb._throt._max = sumT / d;
                        _app._gimb._pitch._max = sumP / d;
                        _app._gimb._roll._max = sumR / d;
                        _app._gimb._yaw._max = sumY / d;

                        block->fwrite("result(max):\tt:%u; p:%u; r:%u; y:%u\n",
                                    _app._gimb._throt._max,
                                    _app._gimb._pitch._max,
                                    _app._gimb._roll._max,
                                    _app._gimb._yaw._max);
                    }
                    block->fwrite("end\n");


                    sumT = 0;
                    sumP = 0;
                    sumR = 0;
                    sumY = 0;

                    block->fwrite("turn into mid and wait end msg\n");
                    ufo::utl::sleep_for(5000);
                    {
                        for (int32_t i = 0; i < d; i++)
                        {
                            ufo::lock_guard<ufo::mutex_t> _l(_app._gimb._lock);

                            if (!_app._gimb._ready)
                            {
                                --i;
                                continue;
                            }

                            sumT += _app._gimb._throt._raw;
                            sumP += _app._gimb._pitch._raw;
                            sumR += _app._gimb._roll._raw;
                            sumY += _app._gimb._yaw._raw;
                            _app._gimb._ready = false;
                            
                            block->fwrite(">t:%llu\n>p:%llu\n>r:%llu\n>y:%llu\n\n", sumT, sumP, sumR, sumY);

                            ufo::utl::sleep_for(25);
                        }

                        ufo::lock_guard<ufo::mutex_t> _l(_app._gimb._lock);
                        _app._gimb._throt._mid = sumT / d;
                        _app._gimb._pitch._mid = sumP / d;
                        _app._gimb._roll._mid = sumR / d;
                        _app._gimb._yaw._mid = sumY / d;

                        block->fwrite("result(mid):\tt:%u; p:%u; r:%u; y:%u\n",
                                    _app._gimb._throt._mid,
                                    _app._gimb._pitch._mid,
                                    _app._gimb._roll._mid,
                                    _app._gimb._yaw._mid);
                    }
                    block->fwrite("end\n");

                             
                                      
                    {
                        ufo::lock_guard<ufo::mutex_t> _l(_app._gimb._lock);

                        if (app::appconfig::gimb_throt_invert < 0)
                        {
                            std::swap(_app._gimb._throt._min,_app._gimb._throt._max);
                        }
                        if (app::appconfig::gimb_pitch_invert < 0)
                        {
                            std::swap(_app._gimb._pitch._min,_app._gimb._pitch._max);
                        }
                        if (app::appconfig::gimb_roll_invert < 0)
                        {
                            std::swap(_app._gimb._roll._min,_app._gimb._roll._max);
                        }
                        if (app::appconfig::gimb_yaw_invert < 0)
                        {
                            std::swap(_app._gimb._yaw._min, _app._gimb._yaw._max);
                        }

                        block->fwrite("throt\t min: %d; mid:%d; max:%u;\n",
                                      _app._gimb._throt._min,
                                      _app._gimb._throt._mid,
                                      _app._gimb._throt._max);
                        block->fwrite("pitch\t min: %d; mid:%d; max:%u;\n",
                                      _app._gimb._pitch._min,
                                      _app._gimb._pitch._mid,
                                      _app._gimb._pitch._max);
                        block->fwrite("roll\t min: %d; mid:%d; max:%u;\n",
                                      _app._gimb._roll._min,
                                      _app._gimb._roll._mid,
                                      _app._gimb._roll._max);
                        block->fwrite("yaw\t min: %d; mid:%d; max:%u;\n",
                                      _app._gimb._yaw._min,
                                      _app._gimb._yaw._mid,
                                      _app._gimb._yaw._max);
                        
                    }
                    return;
                }
            }
        }
    }
    block->log_incorrect_arg();
}
