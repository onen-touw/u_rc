#pragma once

#include "font.h"
#include "u_drivers/spi/uspi.h"
#include "u_sys/utils.h"
#include <math.h>
#include "u_sys/vct.h"

#include "font.h"
//  #define UFO_st7789_FRAME_BUFFER
// #define UFO_st7789_BACK_LIGHT
#define UFO_st7789_FONT_DIRCT_SUPPORTED

constexpr uint16_t rbg_to_rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3); 
}

constexpr uint16_t rbg24_to_rgb16_565(unsigned long rgb){ //convert 24 bit RGB to 16bit 5:6:5 RGB
 
    return(((rgb&0xf80000)>>8)|((rgb&0xfc00)>>5)|((rgb&0xf8)>>3));
  }


class st7789_t
{
public:
    using sz_t = uint16_t;
    using color_t = uint16_t;
    
public:
    static constexpr sz_t d_width = 240;
    static constexpr sz_t d_height = 320;

    static constexpr uint16_t basic_RED =   rbg_to_rgb565(255, 0, 0);
    static constexpr uint16_t basic_GREEN = rbg_to_rgb565(0, 255, 0);
    static constexpr uint16_t basic_BLUE =  rbg_to_rgb565(0, 0, 255);

    enum class direction_t : uint8_t
    {
        DIRECTION0,
        DIRECTION90,
        DIRECTION180,
        DIRECTION270
    };

private:
    struct driver_api_t
    {
        static constexpr uint32_t mode_data = 1;
        static constexpr uint32_t mode_cmd = 0;
        static constexpr int32_t spi_speedHZ = 80000000;

        ufo::drv::uspi_t* _driver = nullptr;    // it is neccesary for destructor
        spi_device_handle_t _handle = nullptr;

        gpio_num_t _dc = gpio_num_t::GPIO_NUM_NC;
        gpio_num_t _rst = gpio_num_t::GPIO_NUM_NC;
        gpio_num_t _cs = gpio_num_t::GPIO_NUM_NC;

        #ifdef UFO_st7789_BACK_LIGHT
                gpio_num_t _bl = gpio_num_t::GPIO_NUM_NC; // backlight
        #endif
        
        // driver_api_t(){}
        driver_api_t(ufo::drv::uspi_t *spi_drv, gpio_num_t dc, gpio_num_t rst, gpio_num_t cs, gpio_num_t bl = gpio_num_t::GPIO_NUM_NC)
            : _driver(spi_drv), _handle(nullptr), _dc(dc), _rst(rst), _cs(cs) 
#ifdef UFO_st7789_BACK_LIGHT
            , _bl(bl)
#endif
        {
            if (!_driver)
            {
                // crit error
                printf("!driver\n");
                return;
            }
            if (_driver->get_descriptor() == spi_host_device_t::SPI_HOST_MAX)
            {
                printf("!bad-spi-driver\n");
                // crit error
                return;
            }

            pin_config();

            spi_device_interface_config_t cfg = {};
            cfg.clock_speed_hz = spi_speedHZ;
            cfg.queue_size = 7;
            cfg.mode = 3;
            cfg.flags = SPI_DEVICE_NO_DUMMY;
        
            if ( cs != gpio_num_t::GPIO_NUM_NC) {
                cfg.spics_io_num = _cs;
            } else {
                cfg.spics_io_num = -1;
                // warning
            }
            _handle = _driver->mk_dev(cfg);
        }

        ~driver_api_t (){
            if (_handle)
            {
                if (!_driver)
                {
                    // crit error
                    printf("~!driver\n");
                }
                printf("~api\n");
                _driver->rm_dev(_handle);
            }
        }

        bool valid() const { return (_driver && _handle);}

        bool w_cmd(uint8_t cmd)
        {
            gpio_set_level(_dc, mode_cmd);
            return w_byte_internal(&cmd, 1 );
        }

        bool w_byte(uint8_t data){
            gpio_set_level(_dc, mode_data);
            return w_byte_internal(&data, 1);
        }

        bool w_word(uint16_t data) {
            uint8_t byte[2] = {};
            byte[0] = (data >> 8) & 0xFF;
            byte[1] = data & 0xFF;
            gpio_set_level(_dc, mode_data);
            return w_byte_internal(byte, 2);
        }

        bool w_addr(uint16_t addr1, uint16_t addr2){
            uint8_t byte[4] = {};
            byte[0] = (addr1 >> 8) & 0xFF;
            byte[1] = addr1 & 0xFF;
            byte[2] = (addr2 >> 8) & 0xFF;
            byte[3] = addr2 & 0xFF;
            gpio_set_level(_dc, mode_data);
            return w_byte_internal(byte, 4);
        }

        bool w_color(uint16_t color, uint16_t size)
        {
            static uint8_t byte[1024] = {}; // well 1024 oks \\but maxx len is 320 color sz is 2 bytes so 320*2=640
            int index = 0;
            for (int i = 0; i < size; i++)
            {
                byte[index++] = (color >> 8) & 0xFF;
                byte[index++] = color & 0xFF;
            }
            gpio_set_level(_dc, mode_data);
            return w_byte_internal(byte, size * 2); // *2 is necessary
        }

        bool w_colors(ufo::vector_t<color_t>& hr_line)
        {
            static uint8_t byte[1024] = {};
            int index = 0;
            for (int i = 0; i < hr_line.size(); i++)
            {
                byte[index++] = (hr_line[i] >> 8) & 0xFF;
                byte[index++] = hr_line[i] & 0xFF;
            }
            gpio_set_level(_dc, mode_data);
            return w_byte_internal(byte, hr_line.size() * 2); // *2 is necessary
        }

        // // not tested
        // bool w_colors8(ufo::vector_t<uint8_t>& hr_line)
        // {
        //     gpio_set_level(_dc, mode_data);
        //     return w_byte_internal(hr_line.raw(), hr_line.size() * 2); // *2 is necessary
        // }
        void _r_rst()
        {
            gpio_set_level(_rst, 1);
            ufo::utl::sleep_for(100);
            gpio_set_level(_rst, 0);
            ufo::utl::sleep_for(100);
            gpio_set_level(_rst, 1);
            ufo::utl::sleep_for(100);
        }

    private:
        void pin_config(){
            if (_cs != gpio_num_t::GPIO_NUM_NC)
            {
                ufo::utl::gpio_config(_cs, GPIO_MODE_OUTPUT);
                gpio_set_level(_cs, 0);
            }

            if (_dc == gpio_num_t::GPIO_NUM_NC)
            {
                // error
                printf("disp::!dc\n");
                return;
            }
            ufo::utl::gpio_config(_dc, GPIO_MODE_OUTPUT);
            gpio_set_level(_dc, 0);

            // ESP_LOGI(TAG, "GPIO_RESET=%d", GPIO_RESET);
            if (_rst != gpio_num_t::GPIO_NUM_NC)
            {
                ufo::utl::gpio_config(_rst, GPIO_MODE_OUTPUT);

                _r_rst();
            }
#ifdef UFO_st7789_BACK_LIGHT
            if (_bl >= 0)
            {
                ufo::utl::gpio_config(_bl, GPIO_MODE_OUTPUT);
                gpio_set_level(_bl, 0);
            }
#endif
        }

        bool w_byte_internal(const uint8_t* data, size_t size)
        {
            spi_transaction_t transaction= {};
        
            if ( size > 0 ) {
                transaction.length = size *8;	// for what 8??? but * 8 is necessary
                transaction.tx_buffer = data;
                spi_device_transmit(_handle, &transaction);
            }
            return true;
        }
    };

private:
    driver_api_t _api;

    sz_t _width = 0;
    sz_t _height = 0;
    sz_t _offsetx = 0;
    sz_t _offsety = 0;

    uint8_t _font_direction = 0;
    uint8_t _font_fill = 0;

    color_t _font_fill_color = 0;  // ???

#ifdef UFO_st7789_FRAME_BUFFER
    ufo::vector_t<ufo::vector_t<color_t>> _frame_buffer;

    // ufo::vector_t<ufo::vector_t<uint8_t>> _fbuf;
#endif

public:
    st7789_t(
        ufo::drv::uspi_t *spi_drv,
        gpio_num_t dc,
        gpio_num_t rst,
        gpio_num_t cs,
        gpio_num_t bl = gpio_num_t::GPIO_NUM_NC,
        sz_t width = d_width,
        sz_t height = d_height)
        : _api(spi_drv, dc, rst, cs, bl),
          _width(width),
          _height(height),
          _offsetx(0),
          _offsety(0),
          _font_direction(static_cast<uint8_t>(direction_t::DIRECTION0))
    {}

    ~st7789_t() {}

        void swap_w_h(){
            sz_t tmp = _width;
            _width = _height;
            _height = tmp;
#ifdef UFO_st7789_FRAME_BUFFER
            _frame_buffer.reset();
            _frame_buffer.reserve(_height);
            for (size_t i = 0; i < _height; i++)
            {
                _frame_buffer.push_back(ufo::vector_t<color_t>(_width));
            }    
#endif
        }

    // void set_api(ufo::drv::uspi_t* spi_drv, gpio_num_t dc, gpio_num_t rst, gpio_num_t cs,  gpio_num_t bl = gpio_num_t::GPIO_NUM_NC)
    // {
    //     printf ("set_api\n");
    //     if (_api.valid())
    //     {
    //         printf("disp::valid\n");
    //         return;
    //     }
    //     _api = driver_api_t(spi_drv, dc, rst, cs, bl);
    //     if (!_api.valid())
    //     {
    //         printf("disp::!valid\n");
    //         return;
    //     }
    // }

    void set_offsets(sz_t x, sz_t y)
    {
        _offsetx = x;
        _offsety = y;
    }

    void init(){
        
        if (!_api.valid())
        {
            printf("init::!_api.valid");
            // crit error
            return;
        }

        _api.w_cmd(0x01);                   // Software Reset
        ufo::utl::sleep_for(150);

        _api.w_cmd(0x11); // Sleep Out
        ufo::utl::sleep_for(255);

        _api.w_cmd(0x3A);                   // Interface Pixel Format
        _api.w_byte(0x55);
        ufo::utl::sleep_for(10);

        _api.w_cmd(0x36);                   // Memory Data Access Control
        _api.w_byte(0x00);

        _api.w_cmd(0x2A);                   // Column Address Set
        _api.w_byte(0x00);
        _api.w_byte(0x00);
        _api.w_byte(0x00);
        _api.w_byte(0xF0);

        _api.w_cmd(0x2B);                   // Row Address Set
        _api.w_byte(0x00);
        _api.w_byte(0x00);
        _api.w_byte(0x00);
        _api.w_byte(0xF0);

        _api.w_cmd(0x21);                   // Display Inversion On
        ufo::utl::sleep_for(10);

        _api.w_cmd(0x13);                   // Normal Display Mode On
        ufo::utl::sleep_for(10);

        _api.w_cmd(0x29);                   // Display ON
        ufo::utl::sleep_for(255);

#ifdef UFO_st7789_BACK_LIGHT
        if (_api._bl != gpio_num_t::GPIO_NUM_NC)
        {
            gpio_set_level(_api._bl, 1);
        }
#endif

#ifdef UFO_st7789_FRAME_BUFFER
        printf("MALLOC_CAP_DEFAULT: %d bytes\n", heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
        printf("MALLOC_CAP_INTERNAL: %d bytes\n", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
        printf("MALLOC_CAP_SPIRAM: %d bytes\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        printf("Free heap size: %lu\n", esp_get_free_heap_size());

        printf("sz:%u\n", sizeof(color_t) * _width * _height);

        _frame_buffer.reserve(_height);
        for (size_t i = 0; i < _height; i++)
        {
            _frame_buffer.push_back(ufo::vector_t<color_t>(_width));
        }     
        
        
        // _fbuf.reserve(_height);
        // for (size_t i = 0; i < _height; i++)
        // {
        //     _fbuf.push_back(ufo::vector_t<uint8_t>(_width * sizeof(color_t)));
        // }  
#endif
    }


    void draw_pixel(sz_t x, sz_t y, color_t color){
        if (x >= _width) return;
        if (y >= _height) return;

#ifdef UFO_st7789_FRAME_BUFFER
                _frame_buffer[y][x] = color;

                // _fbuf[y][x * 2] = (color >> 8) & 255;
                // _fbuf[y][x * 2 + 1] = color & 255;
#else
            sz_t _x = x + _offsetx;
            sz_t _y = y + _offsety;
    
            _api.w_cmd(0x2A);               // set column(x) address
            _api.w_addr(_x, _x);
            
            _api.w_cmd(0x2B);               // set column(y) address
            _api.w_addr(_y, _y);

            _api.w_cmd(0x2C);               // Memory Write

            _api.w_color(color, 1);
#endif
    }

    void draw_rect_filled(sz_t x1, sz_t y1, sz_t x2, sz_t y2, color_t color) {
        if (x1 >= _width) {return;}
        if (x2 >= _width) {
            x2 = _width - 1;
        }
        if (y1 >= _height) {return;}
        if (y2 >= _height) {
            y2 = _height - 1;
        }

        // ESP_LOGD("st7789", "offset(x)=%d offset(y)=%d", _offsetx, _offsety);

#ifdef UFO_st7789_FRAME_BUFFER
        for (sz_t j = y1; j <= y2; ++j)
        {
            for (sz_t i = x1; i <= x2; ++i)
            {
                // _frame_buffer[j * _width + i] = color;
                _frame_buffer[j][i] = color;

                // _fbuf[j][i * 2] = (color >> 8) & 255;
                // _fbuf[j][i * 2 + 1] = color & 255;
            }
        }
#else
        sz_t _x1 = x1 + _offsetx;
        sz_t _x2 = x2 + _offsetx;
        sz_t _y1 = y1 + _offsety;
        sz_t _y2 = y2 + _offsety;

        _api.w_cmd(0x2A);               // set column(x) address
        _api.w_addr(_x1, _x2);
        
        _api.w_cmd(0x2B);               // set column(y) address
        _api.w_addr(_y1, _y2);

        _api.w_cmd(0x2C);               // Memory Write

        for (int i = _x1; i <= _x2; i++)
        {
            uint16_t size = _y2 - _y1 + 1;
            _api.w_color(color, size);
        }
#endif
    }

    void fill_screen(uint16_t color) {
        draw_rect_filled(0, 0, _width - 1, _height - 1, color);
    }

    // Draw Frame Buffer
    void display()
    {
#ifndef UFO_st7789_FRAME_BUFFER
        return;
#else
        if (!_frame_buffer.size())
        {
            // crit error;
            printf("display-crit error\n");
            return;
        }

        _api.w_cmd(0x2A); // set column(x) address
        _api.w_addr(_offsetx, _offsetx + _width - 1);

        _api.w_cmd(0x2B); // set column(y) address
        _api.w_addr(_offsety, _offsety + _height - 1);

        _api.w_cmd(0x2C); // Memory Write

        for (size_t i = 0; i < _frame_buffer.size(); ++i)
        {
            _api.w_colors(_frame_buffer[i]);
        }

        // for (size_t i = 0; i < _fbuf.size(); ++i)
        // {
        //     _api.w_colors(_fbuf[i]);
        // }
        return;
#endif

    }

    // Display OFF
    void off()
    {
        _api.w_cmd(0x28);               // Display off
    }

    // Display ON
    void on()
    {
        _api.w_cmd(0x29);               // Display on
    }

    void rst() {
        _api.w_cmd(0x01);                   // Software Reset
        ufo::utl::sleep_for(150);

        _api.w_cmd(0x11); // Sleep Out
        ufo::utl::sleep_for(255);

        _api.w_cmd(0x3A);                   // Interface Pixel Format
        _api.w_byte(0x55);
        ufo::utl::sleep_for(10);

        _api.w_cmd(0x36);                   // Memory Data Access Control
        _api.w_byte(0x00);

        _api.w_cmd(0x2A);                   // Column Address Set
        _api.w_byte(0x00);
        _api.w_byte(0x00);
        _api.w_byte(0x00);
        _api.w_byte(0xF0);

        _api.w_cmd(0x2B);                   // Row Address Set
        _api.w_byte(0x00);
        _api.w_byte(0x00);
        _api.w_byte(0x00);
        _api.w_byte(0xF0);

        _api.w_cmd(0x21);                   // Display Inversion On
        ufo::utl::sleep_for(10);

        _api.w_cmd(0x13);                   // Normal Display Mode On
        ufo::utl::sleep_for(10);

        _api.w_cmd(0x29);                   // Display ON
        ufo::utl::sleep_for(255);
    }
    void rst_hard() {
        _api._r_rst();
    }

    // todo draw_line_horizontal
    // todo draw_line_vertical
    void draw_line(sz_t x1, sz_t y1, sz_t x2, sz_t y2, color_t color) {
        int i;
        int dx,dy;
        int sx,sy;
        int E;
    
        /* distance between two points */
        dx = ( x2 > x1 ) ? x2 - x1 : x1 - x2;
        dy = ( y2 > y1 ) ? y2 - y1 : y1 - y2;
    
        /* direction of two point */
        sx = ( x2 > x1 ) ? 1 : -1;
        sy = ( y2 > y1 ) ? 1 : -1;
    
        /* inclination < 1 */
        if ( dx > dy ) {
            E = -dx;
            for (i = 0; i <= dx; ++i)
            {
                draw_pixel(x1, y1, color);
                x1 += sx;
                E += 2 * dy;
                if ( E >= 0 ) {
                y1 += sy;
                E -= 2 * dx;
            }
            }

        /* inclination >= 1 */
        } else {
            E = -dy;
            for (i = 0; i <= dy; ++i)
            {
                draw_pixel(x1, y1, color);
                y1 += sy;
                E += 2 * dx;
                if ( E >= 0 ) {
                    x1 += sx;
                    E -= 2 * dy;
                }
            }
        }
    }

    void draw_rect(sz_t x1, sz_t y1, sz_t x2, sz_t y2, color_t color) {
        draw_line(x1, y1, x2, y1, color);
        draw_line(x2, y1, x2, y2, color);
        draw_line(x2, y2, x1, y2, color);
        draw_line(x1, y2, x1, y1, color);
    }

    // Draw rectangle with angle
    // xcenter:Center X coordinate
    // ycenter:Center Y coordinate
    // w:Width of rectangle
    // h:Height of rectangle
    // angle:Angle of rectangle
    // color:color
    void draw_rect_angle(sz_t xcenter, sz_t ycenter, sz_t w, sz_t h, float angle, color_t color)
    {
        float
            xd = 0.f,
            yd = 0.f,
            rd = 0.f;
        int x1 = 0, y1 = 0;
        int x2 = 0, y2 = 0;
        int x3 = 0, y3 = 0;
        int x4 = 0, y4 = 0;

        rd = -angle * M_PI / 180.0f;
        xd = 0.0f - w / 2.f;
        yd = h / 2.f;
        x1 = static_cast<int>(xd * std::cos(rd) - yd * std::sin(rd) + xcenter);
        y1 = static_cast<int>(xd * std::sin(rd) + yd * std::cos(rd) + ycenter);

        yd = 0.0f - yd;
        x2 = static_cast<int>(xd * std::cos(rd) - yd * std::sin(rd) + xcenter);
        y2 = static_cast<int>(xd * std::sin(rd) + yd * std::cos(rd) + ycenter);

        xd = w / 2.f;
        yd = h / 2.f;
        x3 = static_cast<int>(xd * std::cos(rd) - yd * std::sin(rd) + xcenter);
        y3 = static_cast<int>(xd * std::sin(rd) + yd * std::cos(rd) + ycenter);

        yd = 0.0f - yd;
        x4 = static_cast<int>(xd * std::cos(rd) - yd * std::sin(rd) + xcenter);
        y4 = static_cast<int>(xd * std::sin(rd) + yd * std::cos(rd) + ycenter);

        draw_line(x1, y1, x2, y2, color);
        draw_line(x1, y1, x3, y3, color);
        draw_line(x2, y2, x4, y4, color);
        draw_line(x3, y3, x4, y4, color);
    }

    void draw_circle(sz_t x0, sz_t y0, sz_t r, color_t color)
    {
        int x = 0;
        int y = 0;
        int err = 0;
        int old_err = 0;

        x = 0;
        y = -r;
        err = 2 - 2 * r;
        do
        {
            draw_pixel(x0 - x, y0 + y, color);
            draw_pixel(x0 - y, y0 - x, color);
            draw_pixel(x0 + x, y0 - y, color);
            draw_pixel(x0 + y, y0 + x, color);
            if ((old_err = err) <= x)
                {err += ++x * 2 + 1;}
            if (old_err > y || err > x)
                {err += ++y * 2 + 1;}
        } while (y < 0);
    }


    void draw_circle_filled(sz_t x0, sz_t y0, sz_t r, color_t color) {
        int x = 0;
        int y = 0;
        int err = 0;
        int old_err = 0;
        int ChangeX = 0;
    
        x=0;
        y=-r;
        err=2-2*r;
        ChangeX=1;
        do{
            if(ChangeX) {
                draw_line(x0-x, y0-y, x0-x, y0+y, color);
                draw_line(x0+x, y0-y, x0+x, y0+y, color);
            } // endif
            ChangeX=(old_err=err)<=x;
            if (ChangeX)			{err+=++x*2+1;}
            if (old_err>y || err>x) {err+=++y*2+1;}
        } while(y<=0);
    }

    // Draw rectangle with round corner
    // x1:Start X coordinate
    // y1:Start Y coordinate
    // x2:End	X coordinate
    // y2:End	Y coordinate
    // r:radius
    // color:color
    void draw_rect_radius(sz_t x1, sz_t y1, sz_t x2, sz_t y2, sz_t r, color_t color)
    {
        int x = 0;
        int y = 0;
        int err = 0;
        int old_err = 0;
        unsigned char temp = 0;

        if (x1 > x2)
        {
            temp = x1;
            x1 = x2;
            x2 = temp;
        } // endif

        if (y1 > y2)
        {
            temp = y1;
            y1 = y2;
            y2 = temp;
        } // endif

        // ESP_LOGD("st7789", "x1=%d x2=%d delta=%d r=%d", x1, x2, x2 - x1, r);
        // ESP_LOGD("st7789", "y1=%d y2=%d delta=%d r=%d", y1, y2, y2 - y1, r);
        if (x2 - x1 < r)
            return; // Add 20190517
        if (y2 - y1 < r)
            return; // Add 20190517

        x = 0;
        y = -r;
        err = 2 - 2 * r;

        do
        {
            if (x)
            {
                draw_pixel(x1 + r - x, y1 + r + y, color);
                draw_pixel(x2 - r + x, y1 + r + y, color);
                draw_pixel(x1 + r - x, y2 - r - y, color);
                draw_pixel(x2 - r + x, y2 - r - y, color);
            } // endif
            if ((old_err = err) <= x)
                err += ++x * 2 + 1;
            if (old_err > y || err > x)
                err += ++y * 2 + 1;
        } while (y < 0);

        // ESP_LOGD("st7789", "x1+r=%d x2-r=%d", x1 + r, x2 - r);
        draw_line(x1 + r, y1, x2 - r, y1, color);
        draw_line(x1 + r, y2, x2 - r, y2, color);
        // ESP_LOGD("st7789", "y1+r=%d y2-r=%d", y1 + r, y2 - r);
        draw_line(x1, y1 + r, x1, y2 - r, color);
        draw_line(x2, y1 + r, x2, y2 - r, color);
    }

    // Draw arrow of filling
    // x1:Start X coordinate
    // y1:Start Y coordinate
    // x2:End	X coordinate
    // y2:End	Y coordinate
    // w:Width of the botom
    // color:color
    void draw_arrow_filled(sz_t x0, sz_t y0, sz_t x1, sz_t y1, sz_t w, color_t color)
    {
        float Vx = x1 - x0;
        float Vy = y1 - y0;
        float v = std::sqrt(Vx * Vx + Vy * Vy);
        // printf("v=%f\n",v);
        float Ux = Vx / v;
        float Uy = Vy / v;

        uint16_t L[2] = {}, R[2] = {};
        L[0] = x1 - Uy * w - Ux * v;
        L[1] = y1 + Ux * w - Uy * v;
        R[0] = x1 + Uy * w - Ux * v;
        R[1] = y1 - Ux * w - Uy * v;
        // printf("L=%d-%d R=%d-%d\n",L[0],L[1],R[0],R[1]);

        draw_line(x0, y0, x1, y1, color);
        draw_line(x1, y1, L[0], L[1], color);
        draw_line(x1, y1, R[0], R[1], color);
        draw_line(L[0], L[1], R[0], R[1], color);

        int ww;
        for (ww = w - 1; ww > 0; --ww)
        {
            L[0] = x1 - Uy * ww - Ux * v;
            L[1] = y1 + Ux * ww - Uy * v;
            R[0] = x1 + Uy * ww - Ux * v;
            R[1] = y1 - Ux * ww - Uy * v;
            // printf("Fill>L=%d-%d R=%d-%d\n",L[0],L[1],R[0],R[1]);
            draw_line(x1, y1, L[0], L[1], color);
            draw_line(x1, y1, R[0], R[1], color);
        }
    }

    // Display Inversion Off
    void inversion_off()
    {
        _api.w_cmd(0x20);
    }

    // Display Inversion On
    void inversion_on()
    {
        _api.w_cmd(0x21);
    }

    // Draw ASCII character
    // x:X coordinate
    // y:Y coordinate
    // ascii: ascii code
    // color:color
    void draw_char(font_t* font, s16_t x, sz_t y, uint8_t ascii, color_t color, direction_t direction)
    {
        if (!font)
        {
            printf("no font\n");
            return;
        }
        const uint8_t* carr = font->load_char(ascii);
        if (!carr)
        {
            printf("no carr\n");
            return;
        }
        uint8_t pw = font->width(), ph= font->height();

#ifdef UFO_st7789_FONT_DIRCT_SUPPORTED
        int16_t xd1 = 0;
        int16_t yd1 = 0;
        int16_t xd2 = 0;
        int16_t yd2 = 0;
        int16_t xss = 0;
        int16_t yss = 0;
        int16_t xsd = 0;
        int16_t ysd = 0;

        if (direction == direction_t::DIRECTION0)
        {
            xd1 = +1;
            yd1 = +1; //-1;
            xd2 = 0;
            yd2 = 0;
            xss = x;
            yss = y;
            xsd = 1;
            ysd = 0;
        }
        else if (direction == direction_t::DIRECTION180)
        {
            xd1 = -1;
            yd1 = -1; //+1;
            xd2 = 0;
            yd2 = 0;
            xss = x;
            yss = y - ph;
            xsd = 1;
            ysd = 0;
        }
        else if (direction == direction_t::DIRECTION90)
        {
            xd1 = 0;
            yd1 = 0;
            xd2 = -1;
            yd2 = +1; //-1;
            xss = x;
            yss = y;
            xsd = 0;
            ysd = 1;
        }
        else if (direction == direction_t::DIRECTION270)
        {
            xd1 = 0;
            yd1 = 0;
            xd2 = +1;
            yd2 = -1; //+1;
            xss = x + (ph - 2);
            yss = y;
            xsd = 0;
            ysd = 1;
        }
#else
    int16_t xd1 = +1;
    int16_t yd1 = +1; //-1;
    int16_t xd2 = 0;
    int16_t yd2 = 0;
    int16_t xss = x;
    int16_t yss = y - (ph - 1);
    int16_t xsd = 1;
    int16_t ysd = 0;
    int16_t next = x + pw;
    int16_t x0 = x;
    int16_t y0 = y - (ph - 1);
    int16_t x1 = x + (pw - 1);
    int16_t y1 = y;
#endif
        if (true)
        printf("xss=%d yss=%d\n", xss, yss);
        
        uint16_t xx = xss, yy = yss, bit = 0, ofs = 0;
        uint16_t mask = 0;
        int bits = 0;
        
        for (sz_t h = 0; h < ph; h++)
        {
            if (xsd)
                xx = xss;
            if (ysd)
                yy = yss;
            // for(w=0;w<(pw/8);w++) {
            bits = pw;
            for (sz_t w = 0; w < ((pw + 4) / 8); w++)
            {
                mask = 0x80;
                for (bit = 0; bit < 8; bit++)
                {
                    bits--;
                    if (bits < 0)
                        continue;
                    // if(_DEBUG_)printf("xx=%d yy=%d mask=%02x fonts[%d]=%02x\n",xx,yy,mask,ofs,fxs->fonts[ofs]);
                    if (carr[ofs] & mask)
                    {
                        draw_pixel(xx, yy, color);
                    }
                    xx = xx + xd1;
                    yy = yy + yd2;
                    mask = mask >> 1;
                }
                ofs++;
            }
            yy = yy + yd1;
            xx = xx + xd2;
        }
    }

    // todo
    // sz_t get_string_width(){}
    // sz_t get_string_height(){}

    void draw_string(font_t* font, sz_t x, sz_t y, const char *ascii, uint8_t sz, uint16_t color, direction_t direction)
    {
        for (int i = 0; i < sz; i++)
        {
            if (direction == direction_t::DIRECTION0)
            {
                draw_char(font, x + font->width() * i, y, ascii[i], color, direction);
            }
            else if (direction == direction_t::DIRECTION180)
            {
                draw_char(font, x - font->width() * i, y, ascii[i], color, direction);
            }
            else if (direction == direction_t::DIRECTION90)
            {
                draw_char(font, x, y + font->width() * i, ascii[i], color, direction);
            }
            else if (direction == direction_t::DIRECTION270)
            {
                draw_char(font, x, y - font->width() * i, ascii[i], color, direction);
            }
        }
        return;
    }
};