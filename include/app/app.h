#pragma once

#include "u_sys/config.h"
#include "u_sys/list.h"
#include "u_sys/str.h"
#include "u_sys/vct.h"
#include "u_sys/thread.h"
#include "u_sys/mutex.h"
#include "u_sys/utils.h"
#include "u_sys/fsk.h"	 // sock lora ...
#include "u_sys/cns.h"

#include "u_drivers/uart/UFO_Uart.h"

#include "u_utils/i2c_dtct.h"

#include "u_dev/lora/UFO_Lora.h"
#include "u_dev/gimb.h"

#include "crypte/encrypte.h"
#include "crypte/decrypte.h"

#include "net.h"
#include "sens.h"
#include "appdata.h"
#include "display.h"

namespace app
{
    class app_t
    {
    private:
		ufo::net::fsk_base::callback_t _net_cb = nullptr;

    public:

        app_t() {}
        ~app_t() {}

        void task(ufo::token_t token){
			using namespace ufo;
			Trace_t::log("app task start\n");

			app_data_t &appd = app_data_t::get_instanse();
			sys_data_t& msys = sys_data_t::get_instanse();
			
			app::types::event_t event = {};
			

			ufo::thread_cfg cfg_gimb;
			cfg_gimb._name = "gmb";
			cfg_gimb._core = 0;
			cfg_gimb._prio = 5;
			cfg_gimb._stackSize = 4096;
			gimball4_t gimb4;
			ufo::thread_guard task_gimb(ufo::thread(cfg_gimb, &gimball4_t::task, &gimb4));

			ufo::thread_cfg cfg_io;
			cfg_io._name = "io";
			cfg_io._core = 0;
			cfg_io._prio = 5;
			cfg_io._stackSize = 4096;
			ufo::thread_guard task_io(ufo::thread(cfg_io, 
				[](ufo::token_t token){
				
					pcf8575_t ioe(sys_data_t::get_instanse()._drv._i2c.get(), 0x22);
					app_data_t &appd = app_data_t::get_instanse();

					while (token)
					{
						ioe.Update();
						appd._tumb = ioe.Get();
						printf("state: %u\n", appd._tumb.get());
						ufo::utl::sleep_for(75);
					}
				}
			));



			nettt_t nettt;
			nettt_t::desc_t sock = 0;
			nettt_t::msg_block_t sock_msg = nettt.mk(
				sock,	
				std::make_unique<nettt_t::sock_t>(
					"192.168.0.68", 
					nettt_t::sock_t::sockt_t::client, 
					net_callback)
				);

			// nettt_t::desc_t lrr = 0;
			// nettt.mk(lrr, std::make_unique<nettt_t::lora_t>(msys._drv._uart1.get(), net_callback));
			// printf("sd %u, ld %u\n", sock, lrr);

			// lora llora(msys._drv._uart1.get());
			// UFO_LoraSettings conf = {};
			// conf._selfAddr._addh = 0;
			// conf._selfAddr._addl = 2;
			// conf._selfAddr._chan = 10;
			// conf._targAddr._addh = UFO_LORA_BROADCAST;
			// conf._targAddr._addl = UFO_LORA_BROADCAST;
			// conf._targAddr._chan = 8;
			// conf.adrt =  LORA_AIR_DATA_RATE_110_384;
			// llora.SetConfig(conf, [](lora::rcv_t* cll){
			// 	printf("rcv on RC from ROVER: %s\n", cll->_payload);
			// });
			// llora.Setup();
			// lora::msg_block_t lora_msg = llora.get_block();
			// ufo::thread_cfg cfg_lora;
			// cfg_lora._name = "lora";
			// cfg_lora._core = 1;
			// cfg_lora._prio = 5;
			// cfg_lora._stackSize = 4096;
			// ufo::thread_guard task_lora(ufo::thread(cfg_lora, [](lora* lr, token_t token){
			// 	while (token)
			// 	{
			// 		lr->Iteration();
			// 	}
			// }, &llora));
#pragma region //display
			
						// ufo::sys_data_t& _sys = ufo::sys_data_t::get_instanse();
						// display_t display (_sys._drv._spi2.get());
			// {
			// 	printf("Performing SPIFFS_check().\n");
			// 	ret = esp_spiffs_check(conf.partition_label);
			// 	if (ret != ESP_OK) {
			// 		printf("SPIFFS_check() failed\n");
			// 	}
			// 	utl::sleep_for(1);

			// 	size_t total = 0, used = 0;
			// 	ret = esp_spiffs_info(conf.partition_label, &total, &used);
			// 	if (ret != ESP_OK) {
			// 		printf("Failed to get SPIFFS partition\n");
			// 	}
			// 	printf("Mount %s success\n", conf.base_path);
			// 	printf("Partition size: total: %u, used: %u, free %u\n", total, used, total - used);
			// }

			// font_t font("/spiffs/ILGH24XB.FNT");
			// font.open();
			// font.log_struct();

			// st7789_t display(&spi_driver, CONFIG_DC_GPIO, CONFIG_RESET_GPIO, CONFIG_CS_GPIO, gpio_num_t::GPIO_NUM_NC);
			// display.init();

                // lora_msg->fMsg(3, "hello bro%d...", 1000);
				// msg_block->Msg("hello bro");
				// DISPLAY_TEST_fill_screen(&display);

				// display.fill_screen(st7789_t::basic_RED);
				// display.display();
				// printf("draw char dir 0\n\n");
				// display.draw_char(&font, 0, 0, 'A', st7789_t::basic_GREEN, st7789_t::direction_t::DIRECTION0);
				// display.draw_string(&font, 80, 50 , "string", 6, st7789_t::basic_GREEN, st7789_t::direction_t::DIRECTION0);
				// display.display();
				// ufo::utl::sleep_for(5000);
				
				// display.fill_screen(st7789_t::basic_RED);
				// display.display();
				// printf("draw char dir 90\n\n");
				// display.draw_char(&font, 0, 0, 'A', st7789_t::basic_GREEN, st7789_t::direction_t::DIRECTION90);
				// display.draw_string(&font, 80, 50 , "string", 6, st7789_t::basic_GREEN, st7789_t::direction_t::DIRECTION90);
				// display.display();
				// ufo::utl::sleep_for(5000);
            
				// display.draw_line(0, 0, 240, 320, st7789_t::basic_GREEN);
				// ufo::utl::sleep_for(2000);
            
#pragma endregion 

			crt::encrypte_t<remote_cmd_e> encripter;

			event._app.set(app_event_e::idle);
			msys._cns.unlock();

			using qcmd_t = types::app_cmd_queue_t::cmd_t; 
			qcmd_t cmd = qcmd_t::null;

            while (token)
            {
				// lora_msg->Msg(3, "cntRV:228\n",11);
				// sock_msg->fMsg("hello %lu", ufo::utl::get_time_millis());
				if (event._app == app_event_e::control)
				{
					if (xQueueReceive(appd._queue._q, &cmd, 30))
					{
						if (cmd == qcmd_t::disarm)
						{
							encripter.pack(remote_cmd_e::arm, uint8_t(0u));
							msys._cns.unlock();
						}
						else if (cmd == qcmd_t::req_ask)
						{
							encripter.pack(remote_cmd_e::ask, ufo::utl::get_time_millis());
						}
						continue;
					}

					// check connection (sometimes)
					{
						ufo::lock_guard<ufo::mutex_t> lock(appd._gimb._lock);
						if (appd._gimb._ready)
						{
							appd._gimb._ready = false;
							encripter.pack(remote_cmd_e::trpy,
										   appd._gimb._throt,
										   appd._gimb._roll,
										   appd._gimb._pitch,
										   appd._gimb._yaw);
						}
					}


				}
				else if (event._app == app_event_e::idle)
				{
					if (xQueueReceive(appd._queue._q, &cmd, 30))
					{
						if (cmd == qcmd_t::arm)					// set from btns
						{
							event._app.set(app_event_e::control);
							msys._cns.block();
						}
						else if (cmd == qcmd_t::find_on)			// set from btns
						{
							event._app.set(app_event_e::alarm);
							event._alarm.set(app_event_alarm_e::find_mode);
						}
						else {
							printf("no such cmd");
							event._app.set(app_event_e::alarm);
							event._alarm.set(app_event_alarm_e::warning);							
						}
					}
					// check connection

					// printf("hello after rcv\n");

				}
				else if (event._app == app_event_e::alarm)
				{
					if (event._alarm.get() == app_event_alarm_e::disconn)
					{
						// bip-bip-bip
						event._app.back();
					}
					else if (event._alarm.get() == app_event_alarm_e::find_mode)
					{
						while (true)
						{
							// bip-bip
							if (xQueueReceive(appd._queue._q, &cmd, 50))
							{
								if (cmd == qcmd_t::find_off)
								{
									break;
									event._app.back();
								}
							}
						}
					}
					else if (event._alarm.get() == app_event_alarm_e::warning){
						// bp
						event._app.back();
					}
					else if (event._alarm.get() == app_event_alarm_e::battery)
					{
						// bip
						event._app.back();
					}
					else if (event._alarm.get() == app_event_alarm_e::battery_crit)
					{
						//bip-bibiiiiip
						break;
					}
					else
					{
						// crit 
						for (uint16_t i = 0; i < 25; i++)
						{
							// bip-bip-bibibip
							ufo::utl::sleep_for(500);
						}
						break;
					}
				}
				else
				{
					event._app.set(app_event_e::alarm);
					event._alarm.set(app_event_alarm_e::critical);
				}

				// {
				// 	ufo::lock_guard<ufo::mutex_t> lock(appd._gimb._lock);
				// 	if (appd._gimb._ready)
				// 	{
				// 		appd._gimb._ready = false;
				// 		encripter.pack(cmd_t::remote_trpy,
				// 					   appd._gimb._throt,
				// 					   appd._gimb._roll,
				// 					   appd._gimb._pitch,
				// 					   appd._gimb._yaw);
				// 	}
				// }
				if (encripter.size())
				{
					sock_msg->Msg(encripter.get(), encripter.size());
					encripter.reset();
				}
				ufo::utl::sleep_for(1);
            }

			// reasone ??

        }

		static void net_callback(ufo::net::fsk_base::rcv_t *rcv){
			ufo::Trace_t::flog("rcv[%u] (%u): %s\n", ufo::utl::get_time_millis(), rcv->_len, rcv->_payload);
		}

		void cns_init(ufo::cns::console_t & cns){
			using namespace ufo;

			cns.mk_blank(
				"app",
				"",
				[](cns::console_t::block_t block)
				{
					vector_t<string_t> &arg_list = block->get_buf();

					if (!arg_list.empty())
					{
						if (arg_list.size() > 1)
						{
							cns::opt_t opt(arg_list[1]);
							if (opt)
							{

							}
						}
					}
				});

			cns.mk_blank(
				"gmb",
				"",
				[](cns::console_t::block_t block)
				{
					block->write("gmb was called\n");

					vector_t<string_t> &arg_list = block->get_buf();

					if (!arg_list.empty())
					{
						if (arg_list.size() > 1)
						{
							cns::opt_t opt(arg_list[1]);
							if (opt == 'e' || opt == "echo")
							{
								uint16_t d = 50;

								if (opt.arg_count() == 1)
								{
									d = opt.get_arg<uint16_t>(0);
									if (!d)
									{
										d = 50;
									}
									block->fwrite("change freq to %ums\n", d);
								}
								
								app::app_data_t &_app = app::app_data_t::get_instanse();
								while (!block->is_read_out_signal())
								{
									{
										block->fwrite(">t:%.3f\n>r:%.3f\n>p:%.3f\n>y:%.3f\n\n",
											_app._gimb._throt, 
											_app._gimb._roll, 
											_app._gimb._pitch, 
											_app._gimb._yaw
											);
									}
									utl::sleep_for(d);
								}
								block->write("stop echo\n");
								return;
							}
							else if (opt == 'c' || opt == "calibrate")
							{
								app::app_data_t &_app = app::app_data_t::get_instanse();
								// _app.
							}
						}
					}
					block->log_incorrect_arg();
				});
		}
    };

} // namespace ufo


