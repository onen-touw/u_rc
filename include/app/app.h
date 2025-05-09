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

// #define use_wf_driver
// #define use_net
// #define use_sens
// #define use_uart
// #define use_lora


namespace app
{
    class app_t
    {
    private:
    public:

        app_t() {
          
        }
        ~app_t() {}

        void task(ufo::token_t token){
			using namespace ufo;
			Trace_t::log("app task start\n");

			app_data_t &appd = app_data_t::get_instanse();

			net_t nett;
			nett.mk_sock(
				net_descriptors_t::sock_main,
				"192.168.0.68",
				net::uSocketType_t::UFO_SOCK_SERVER,
				[](net::fast_sock::rcv_t *rcv)
				{
					// Trace_t::flog("rcv[%u] (%u): %s\n", ufo::utl::get_time_millis(),rcv->_len, rcv->_payload);

					/* crt::decrypte_t::unpack(
						reinterpret_cast<uint8_t *>(rcv->_payload),
						rcv->_len,
						[](cmd_t cmd, uint8_t *buf)
						{
							//  app_data_t &appd = app_data_t::get_instanse();

							//  if (cmd == cmd_t::remote_trpy)
							//  {
							// 	 ufo::lock_guard<mutex_t> _l(appd._remote._lock);

							// 	 appd._remote._throt = crt::get_arg<float>(0, buf);
							// 	 appd._remote._roll = crt::get_arg<float>(1, buf);
							// 	 appd._remote._pitch = crt::get_arg<float>(2, buf);
							// 	 appd._remote._yaw = crt::get_arg<float>(3, buf);
							// 	 appd._remote._mcmd = udt::types::mot_cmd_t::mot_vals;
							//  }
							//  else if (cmd == cmd_t::remote_arm)
							//  {
							// 	 ufo::lock_guard<mutex_t> _l(appd._remote._lock);
							// 	 if (crt::get_arg<int32_t>(0, buf) > 0)
							// 	 {
							// 		 appd._remote._mcmd = udt::types::mot_cmd_t::mot_set_arm;
							// 	 }
							// 	 else
							// 	 {
							// 		 appd._remote._mcmd = udt::types::mot_cmd_t::mot_set_disarm;
							// 	 }
							//  }
						}); */
				},
				"192.168.0.68");
			net_t::msg_block_t msg_block = nett.get_block(net_descriptors_t::sock_main);

			ufo::thread_cfg cfg_net;
			cfg_net._name = "net";
			cfg_net._core = 0;
			cfg_net._prio = 5;
			cfg_net._stackSize = 4096;
			ufo::thread_guard task_net(ufo::thread(cfg_net, &net_t::task, &nett));

			ufo::thread_cfg cfg_gimb;
			cfg_gimb._name = "gimb";
			cfg_gimb._core = 0;
			cfg_gimb._prio = 5;
			cfg_gimb._stackSize = 4096;
			gimball4_t gimb4;
			ufo::thread_guard task_gimb(ufo::thread(cfg_gimb, &gimball4_t::task, &gimb4));

			// ufo::sys_data_t& _sys = ufo::sys_data_t::get_instanse();
			// display_t display (_sys._drv._spi2.get());
#pragma region //display
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

			crt::encrypte_t encripter;

            while (token)
            {
				{
					ufo::lock_guard<ufo::mutex_t> lock(appd._gimb._lock);
					if (appd._gimb._ready)
					{
						appd._gimb._ready = false;
						encripter.pack(cmd_t::remote_trpy,
									   appd._gimb._throt,
									   appd._gimb._roll,
									   appd._gimb._pitch,
									   appd._gimb._yaw);
					}
				}
				if (encripter.size())
				{
					msg_block->Msg(encripter.get(), encripter.size());
					encripter.reset();
				}
				ufo::utl::sleep_for(50);
            }
            
        }

		void cns_init(ufo::cns::console_t & cns){
			using namespace ufo;

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
						}
					}
					block->log_incorrect_arg();
				});
			
			
		}
    };

} // namespace ufo


