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
#include "u_sys/sdt.h"

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

#include "rc_io.h"
#include "io_binds.h"
#include "rc_gimb.h"
#include "rc_console.h"

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
			
			{
				// base calibration data for gimballs
				appd._gimb._throt._min = 0;
				appd._gimb._throt._mid = 1633;
				appd._gimb._throt._max = 3694;

				appd._gimb._pitch._min = 0;
				appd._gimb._pitch._mid = 1525;
				appd._gimb._pitch._max = 3451;
				
				appd._gimb._roll._min = 20;
				appd._gimb._roll._mid = 1737;
				appd._gimb._roll._max = 3890;

				appd._gimb._yaw._min = 47;
				appd._gimb._yaw._mid = 1735;
				appd._gimb._yaw._max = 3893;

				
				appd._gimb._throt._offset = 0;
				appd._gimb._pitch._offset = -88;
				appd._gimb._roll._offset = -94;
				appd._gimb._yaw._offset = 105;

			}



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
			// ufo::thread_guard task_io(ufo::thread(cfg_io, 
			// 	[](ufo::token_t token){
			// 		app_data_t &appd = app_data_t::get_instanse();
			// 		using qcmd_t = types::app_cmd_queue_t::cmd_t; 		
			// 		pcf8575_t ioe(sys_data_t::get_instanse()._drv._i2c.get(), 0x22);
			// 		ioe.InitSensor();
			// 		while (token)
			// 		{
			// 			ioe.Update();
			// 			bit_flag_t<uint16_t> u = ioe.Get();
			// 			if (u != appd._tumb)
			// 			{
			// 				rc_tumblers_e t = rc_tumblers_e::max;
			// 				for (size_t i = 0; i < 6 /* max_io_chan */; i++)
			// 				{
			// 					if (u.get(i) != appd._tumb.get(i))
			// 					{
			// 						t = static_cast<rc_tumblers_e>(i);
			// 						switch (t)
			// 						{
			// 						case rc_tumblers_e::t0:
			// 							xQueueSend(appd._queue._q,appd. , 30);
			// 							printf("t00\n");
			// 							break;
			// 						case rc_tumblers_e::t1:
			// 							// xQueueSend(appd._queue._q, );
			// 							printf("t10\n");
			// 							break;
			// 						case rc_tumblers_e::t2:
			// 							// xQueueSend(appd._queue._q, );
			// 							printf("t20\n");
			// 							break;
			// 						case rc_tumblers_e::t3:
			// 							// xQueueSend(appd._queue._q, );
			// 							printf("t30\n");
			// 							break;
			// 						case rc_tumblers_e::t4:
			// 							// xQueueSend(appd._queue._q, );
			// 							printf("t40\n");
			// 							break;
			// 						case rc_tumblers_e::t5:
			// 							// xQueueSend(appd._queue._q, );
			// 							printf("t50\n");
			// 							break;
			// 						case rc_tumblers_e::t6:
			// 							// xQueueSend(appd._queue._q, );
			// 							printf("t60\n");
			// 							break;
			// 						default:
			// 							break;
			// 						}
			// 						// xQueueSend(appd._queue._q, );
			// 					}
			// 				}
			// 				appd._tumb = u;
			// 			}
			// 			ufo::utl::sleep_for(75);
			// 		}
			// 	}
			// ));

			rc_io_t io_ctl(sys_data_t::get_instanse()._drv._i2c.get());
			// io_ctl.mk_bind(rc_digital_io_t::swa, [](uint8_t val){
			// 	// sys_data_t& ss = sys_data_t::get_instanse();
			// 	printf("swa: %u\n", val);
			// }, 0);

			io_ctl.mk_bind(rc_digital_io_t::swa, rc_binds::arm_state, 0);
			io_ctl.mk_bind(rc_digital_io_t::swd, rc_binds::find_mode, 0);

			io_ctl.mk_bind(rc_digital_io_t::swb, [](uint8_t val){
				printf("swb: %u\n", val);
			}, 0);

			io_ctl.mk_bind(rc_digital_io_t::swc1, [](uint8_t val){
				printf("swc1: %u\n", val);
			}, 0);

			io_ctl.mk_bind(rc_digital_io_t::swc2, [](uint8_t val){
				printf("swc2: %u\n", val);
			}, 0);

			// ufo::thread_guard task_io(ufo::thread(cfg_io, &rc_io_t::task, &io_ctl));

			nettt_t nettt;
			nettt_t::desc_t sock = 0;
			nettt_t::desc_t lrr = 0;
			nettt_t::msg_block_t sock_msg = nettt.mk(
				sock,	
				std::make_unique<nettt_t::sock_t>(
					"192.168.0.68", 
					nettt_t::sock_t::sockt_t::client, 
					net_callback)
				);


			// nettt_t::msg_block_t lora_msg = nettt.mk(
			// 	lrr,	
			// 	std::make_unique<nettt_t::lora_t>(
			// 		msys._drv._uart1.get(), 
			// 		lora_test_callback)
			// 	);

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
				// lora_msg->fMsg(3, "rc: %lu", utl::get_time_millis());
				
				if (encripter.size())
				{
					sock_msg->Msg(encripter.get(), encripter.size());
					encripter.reset();
				}
				// lora_msg->Msg(3, "cntRV:228\n",11);
				// sock_msg->fMsg("hello %lu", ufo::utl::get_time_millis());
				if (event._app == app_event_e::control)
				{
					{
						ufo::lock_guard<ufo::mutex_t> lock(appd._gimb._lock);
						if (appd._gimb._ready)
						{
							appd._gimb._ready = false;

							encripter.pack(remote_cmd_e::trpy,
										   appd._gimb._throt.get(),
										   appd._gimb._roll.get(),
										   appd._gimb._pitch.get(),
										   appd._gimb._yaw.get());
						}
					}
					
					if (xQueueReceive(appd._queue._q, &cmd, 10))
					{
						// printf( "\n");

						if (cmd == qcmd_t::disarm)
						{
							printf( "rc::control::disarm\n");
							encripter.pack(remote_cmd_e::arm, int8_t(0));
							// encripter.log();

							// after checks

							event._app.set(app_event_e::idle);
							// msys._cns.unlock();
						}
						else if (cmd == qcmd_t::req)
						{
							// printf( "rc::control::req_ask\n");

							encripter.pack(remote_cmd_e::ask, ufo::utl::get_time_millis());
						}
						else {
							printf( "rc::control::~\n");
						}
						continue;	// ?? if we handle trpy outside this scope (like it did now)
					}
					// printf( "\rrc::control::");
					// check connection (sometimes)

					utl::sleep_for(20);
				}
				else if (event._app == app_event_e::idle)
				{
					if (xQueueReceive(appd._queue._q, &cmd, 10))
					{
						// printf("\n");
						if (cmd == qcmd_t::arm)					// set from btns
						{
							printf( "rc::idle::arm\n");
							
							if (!msys._cns.get_state().get(ufo::types::cns_t::cns_state_t::started))
							{
								// todo!!!
								// msys._cns.block();
								event._app.set(app_event_e::control);
								encripter.pack(remote_cmd_e::arm, uint8_t(1));
							}
						}
						else if (cmd == qcmd_t::find_on)			// set from btns
						{
							// printf( "rc::idle::find_mode\n");
							encripter.pack(remote_cmd_e::find_mode, uint8_t(1));

							event._app.set(app_event_e::alarm);
							event._alarm.set(app_event_alarm_e::find_mode);
						}
						else if (cmd == qcmd_t::req)
						{
							// printf("rc::idle::req_ask\n");
							encripter.pack(remote_cmd_e::ask, utl::get_time_millis());
						}
						else {
							printf("rc::no such cmd\n");

							event._app.set(app_event_e::alarm);
							event._alarm.set(app_event_alarm_e::warning);							
						}
						continue;		
					}
					// printf( "\rrc::idle::");
					utl::sleep_for(10);
					// check connection
				}
				else if (event._app == app_event_e::alarm)
				{
					if (event._alarm.get() == app_event_alarm_e::disconn)
					{
						// bip-bip-bip
						printf("rc::alarm::disconn\n");

						event._app.back();
					}
					else if (event._alarm.get() == app_event_alarm_e::find_mode)
					{
						printf("rc::alarm::find-find::\n\twait tubm-find-off\n");

						while (true)
						{
							// bip-bip
							if (xQueueReceive(appd._queue._q, &cmd, 50))
							{
								if (cmd == qcmd_t::find_off)
								{
									printf("\n");
									encripter.pack(remote_cmd_e::find_mode, uint8_t(0));
									event._app.back();
									break;
								}
								continue;
							}
							printf(".");
							utl::sleep_for(50);
						}
					}
					else if (event._alarm.get() == app_event_alarm_e::warning){
						// bp
						printf("rc::alarm::warning\n");
						event._app.back();
					}
					else if (event._alarm.get() == app_event_alarm_e::battery)
					{
						// bip
						printf("rc::alarm::bat low\n");
						event._app.back();
					}
					else if (event._alarm.get() == app_event_alarm_e::battery_crit)
					{
						//bip-bibiiiiip
						printf( "rc::alarm::bat low\n\tpower-off\n");
						break;
					}
					else
					{
						// crit 
						for (uint16_t i = 0; i < 10; i++)
						{
							// bip-bip-bibibip
							printf("rc::alarm::critical\n");
							ufo::utl::sleep_for(500);
						}
						break;
					}
					continue;

				}
				else if (event._app == app_event_e::setting)
				{
					printf("rc::app::settings\n\tno impl\n");
					event._app.back();
				}
				else
				{
					printf("rc::app::null\n");
					event._app.set(app_event_e::alarm);
					event._alarm.set(app_event_alarm_e::critical);
				}
				ufo::utl::sleep_for(1);
            }

			// reasone ??

        }

		static void net_callback(ufo::net::fsk_base::rcv_t *rcv){
			ufo::Trace_t::flog("rcv[%u] (%u): %s\n", ufo::utl::get_time_millis(), rcv->_len, rcv->_payload);
		}

		static void lora_test_callback(ufo::net::fsk_base::rcv_t *rcv){
			ufo::Trace_t::flog("rcv[%u] (%u): %s\n", ufo::utl::get_time_millis(), rcv->_len, rcv->_payload);
		}


		void cns_init(ufo::cns::console_t & cns){
			using namespace ufo;

			// cns.mk_blank(
			// 	"app",
			// 	"",
			// 	[](cns::console_t::block_t block)
			// 	{
			// 		vector_t<string_t> &arg_list = block->get_buf();

			// 		if (!arg_list.empty())
			// 		{
			// 			if (arg_list.size() > 1)
			// 			{
			// 				cns::opt_t opt(arg_list[1]);
			// 				if (opt)
			// 				{

			// 				}
			// 			}
			// 		}
			// 	});

			// cns.mk_blank(
			// 	"gmb",
			// 	"",
			// 	console_gmb);
			
				cns.mk_blank(
				"echo",
				"",
				console_echo);

			// cns.mk_blank(
			// 	"gmb",
			// 	"",
			// 	[](cns::console_t::block_t block)
			// 	{
			// 		vector_t<string_t> &arg_list = block->get_buf();

			// 		if (!arg_list.empty())
			// 		{
			// 			if (arg_list.size() > 1)
			// 			{
			// 				cns::opt_t opt(arg_list[1]);
			// 				if (opt == 'e' || opt == "echo")
			// 				{
			// 					uint16_t d = 50;

			// 					if (opt.arg_count() == 1)
			// 					{
			// 						d = opt.get_arg<uint16_t>(0);
			// 						if (!d)
			// 						{
			// 							d = 50;
			// 						}
			// 						block->fwrite("change freq to %ums\n", d);
			// 					}
								
			// 					app::app_data_t &_app = app::app_data_t::get_instanse();
			// 					while (!block->is_read_out_signal())
			// 					{
			// 						{
			// 							block->fwrite(">t:%d\n>r:%d\n>p:%d\n>y:%d\n\n",
			// 								_app._gimb._throt, 
			// 								_app._gimb._roll, 
			// 								_app._gimb._pitch, 
			// 								_app._gimb._yaw
			// 								);
			// 						}
			// 						utl::sleep_for(d);
			// 					}
			// 					block->write("stop echo\n");
			// 					return;
			// 				}
			// 				else if (opt == 'c' || opt == "calibrate")
			// 				{
			// 					app::app_data_t &_app = app::app_data_t::get_instanse();
			// 					uint16_t d = 50;

			// 					if (opt.arg_count() == 1)
			// 					{
			// 						d = opt.get_arg<uint16_t>(0);
			// 						if (!d || d > 20)
			// 						{
			// 							d = 10;
			// 						}
			// 						block->fwrite("sempl cnt: %ums\n", d);
			// 					}

			// 					block->fwrite("pitch calibrating\n");


			// 					uint16_t i = 0;
			// 					uint64_t val = 0;
			// 					for (; i < d; ++i)
			// 					{
			// 						val += _app._gimb._pitch;
			// 						ufo::utl::sleep_for(5);
			// 					}
			// 					val /= i;
			// 				}
			// 			}
			// 		}
			// 		block->log_incorrect_arg();
			// 	});

			// 	cns.mk_blank(
			// 	"tmb",
			// 	"",
			// 	[](cns::console_t::block_t block)
			// 	{
			// 		vector_t<string_t> &arg_list = block->get_buf();

			// 		if (!arg_list.empty())
			// 		{
			// 			if (arg_list.size() > 1)
			// 			{
			// 				cns::opt_t opt(arg_list[1]);
			// 				if (opt == 'e' || opt == "echo")
			// 				{
			// 					uint16_t d = 50;
			// 					if (opt.arg_count() == 1)
			// 					{
			// 						d = opt.get_arg<uint16_t>(0);
			// 						if (!d)
			// 						{
			// 							d = 50;
			// 						}
			// 						block->fwrite("change freq to %ums\n", d);
			// 					}
								
			// 					app::app_data_t &_app = app::app_data_t::get_instanse();
			// 					while (!block->is_read_out_signal())
			// 					{
			// 						block->fwrite(">tmb%u\n\n", _app._tumb.get());
			// 						utl::sleep_for(d);
			// 					}
			// 					block->write("stop echo\n");
			// 					return;
			// 				}
			// 				// else if (opt == 's' || opt == "set")
			// 				// {
			// 				// 	app::app_data_t &_app = app::app_data_t::get_instanse();
			// 				// 	// _app.
			// 				// }
			// 			}
			// 		}
			// 		block->log_incorrect_arg();
			// 	});
		}
    };

} // namespace ufo


