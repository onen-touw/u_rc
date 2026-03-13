#pragma once
#include "st7789d.h"


void DISPLAY_TEST_fill_screen(st7789_t* dev) {
	TickType_t 
        startTick1 = 0;

        startTick1 = xTaskGetTickCount();
        for (size_t i = 0; i < 60; i++)
        {
            dev->fill_screen(st7789_t::basic_RED);
            dev->display();
        }
    printf("elapsed time for 30fr[ms]:%lu\n", (xTaskGetTickCount()-startTick1) * portTICK_PERIOD_MS);
    dev->fill_screen(st7789_t::basic_GREEN);
    dev->display();
    
    dev->fill_screen(st7789_t::basic_BLUE);
    dev->display();
    ufo::utl::sleep_for(3000);
    return;
}