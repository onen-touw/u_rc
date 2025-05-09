#pragma once

#include <stdio.h>
#include <string.h>
#include "u_sys/str.h"
#include "u_sys/error.h"
#include "u_sys/trace.h"

class font_t
{
public:
    static constexpr uint8_t font_path_size = 10u;   // also font name size
    static constexpr uint8_t fontfile_header_size = 18u;  
private:
    ufo::string_t _ffpath;

	bool  _is_ank = false;

    FILE *  _file = nullptr;
    
	uint8_t _fwidth = 0;
	uint8_t _fheight = 0;
	uint8_t _fbc = 0;
	uint16_t _fsz = 0;

    /*
    /// width x height matrix of char  
    e.g. warning sign(!) in matrix form looks like this 
        . . . . . . .
        . . . * . . . 
        . . . * . . . 
        . . . * . . . 
        . . . * . . .
        . . . . . . .
        . . . . . . .
        . . . * . . .
        . . . . . . .
     */
    uint8_t *_fbuf = nullptr;  

public:
    font_t(){}
    font_t(const char *path) {

        uint16_t sz = strlen(path);
        if (sz > font_path_size)
        {
            // error
        }
        _ffpath = ufo::string_t(path);        // in spiffs cant be created dir 
                                    //so path always starts from /spiffs (usually)
    }

    ~font_t() { close(); }

    bool open()
    {
        if (_file)
        {
            // crit error
            return true;
        }
        
        _file = fopen(_ffpath.c_str(), "r");
        if (!_file)
        {
            ufo::Trace_t::flog("Fontx:%s not found.\n", _ffpath.c_str());
            return false;
        }

        char header[fontfile_header_size] = {};
		if (fread(header, 1, fontfile_header_size, _file) != fontfile_header_size) {
			fclose(_file);
			_file = nullptr;
            return false;
        }

        {
            char temp[8] = {};
            memcpy(temp, &header[6], 8);
            ufo::Trace_t::flog(
                "opening font by path %s, name: %s\n", 
                _ffpath.c_str(),
                temp);
        }

        _fwidth = header[14];
        _fheight = header[15];
        _is_ank = !header[16];
        _fbc = header[17];
        _fsz = (_fwidth + 7) / 8 * _fheight;

        _fbuf = new uint8_t[_fsz]();
        if (!_fbuf)
        {
            // crit error
            ufo::Trace_t::flog("error alloc _fbuf\n");
			fclose(_file);
            return false;
        }
        return true;
    }

    void close()
    {
        if(_file){
            fclose(_file);
            _file = nullptr;
        }

        if (_fbuf)
        {
            delete[] _fbuf;
            _fbuf = nullptr;
        }
    }


    // Displaying the FontxFile structure
    void log_struct()
    {
        printf("path=%s\n", _ffpath.c_str());
        printf("is_ank=%d\n", _is_ank);
        printf("w=%d\n", _fwidth);
        printf("h=%d\n", _fheight);
        printf("fsz=%d\n", _fsz);
        printf("bc=%d\n", _fbc);
    }

    uint8_t width() const { return _fwidth; }
    uint8_t height() const { return _fheight; }

    const uint8_t* load_char(uint8_t ascii) {
        if (!_file)
        {
            printf("font::!file\n");
        return nullptr;
        }
        uint32_t offset = 0;

        if (!_is_ank)
        {
            printf("!is_ank\n");
            return nullptr;
        }
        offset = 17 + ascii * _fsz;

        if (fseek(_file, offset, SEEK_SET))
        {
            printf("Fontx:seek(%" PRIu32 ") failed.\n", offset);    //todo warning
            return nullptr;
        }

        if (fread(_fbuf, 1, _fsz, _file) != _fsz)
        {
            printf("Fontx:fread failed.\n");            //todo warning
            return nullptr;
        }
        return _fbuf;
    }

    // Displaying font patterns
    void ShowFont(uint8_t *fonts, uint8_t pw, uint8_t ph)
    {
        int x, y, fpos;
        printf("[ShowFont pw=%d ph=%d]\n", pw, ph);
        fpos = 0;
        for (y = 0; y < ph; y++)
        {
            printf("%02d", y);
            for (x = 0; x < pw; x++)
            {
                if (fonts[fpos + x / 8] & (0x80 >> (x % 8)))
                {
                    printf("*");
                }
                else
                {
                    printf(".");
                }
            }
            printf("\n");
            fpos = fpos + (pw + 7) / 8;
        }
        printf("\n");
    }
};

