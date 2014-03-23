/*

  Spark Core library for HT1632C based LED dot matrix displays (eg. Sure Electronics).
  Copyright (C) 2014 by Xose Pérez <xose dot perez at gmail dot com>
  http://tinkerman.eldiariblau.net

  Based on the same library for Arduino (https://code.google.com/p/ht1632c/)

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <string.h>
#include "spark_wiring.h"
#include "spark_wiring_interrupts.h"
#include "ht1632c.h"
#include "font.h"

void ht1632c::_set(uint8_t pin) {
    digitalWrite(pin, HIGH);
}

void ht1632c::_unset(uint8_t pin) {
    digitalWrite(pin, LOW);
}

void ht1632c::_pulse(uint8_t pin, uint8_t pulses) {
    while (pulses--) {
        _set(pin);
        delayMicroseconds(PULSE_WIDTH_USEC);
        _unset(pin);
    }
}

uint8_t ht1632c::_div(uint8_t n, uint8_t d) {
    uint8_t q = 0;
    while(n >= d) {
        n -= d;
        q++;
    }
    return q;
}

void ht1632c::_chipselect(uint8_t cs) {
    _unset(_cs);
    if (cs == HT1632_CS_ALL) {
        _pulse(_clk, HT1632_CS_ALL);
    } else if (cs == HT1632_CS_NONE) {
        _set(_cs);
        _pulse(_clk, HT1632_CS_ALL);
    } else {
        _pulse(_clk, 1);
        _set(_cs);
        _pulse(_clk, cs - 1);
    }
}

void ht1632c::_writebits(uint8_t bits, uint8_t msb) {
    do {
        (bits & msb) ? _set(_data) : _unset(_data);
        _unset(_wr);
        _set(_wr);
    } while (msb >>= 1);
}

void ht1632c::_sendcmd(uint8_t cs, uint8_t command) {

    _chipselect(cs);
    _writebits(HT1632_ID_CMD, HT1632_ID_LEN);
    _writebits(command, HT1632_CMD_LEN);
    _writebits(0, 1);
    _chipselect(HT1632_CS_NONE);

}

boolean ht1632c::_inside(int x, int y, int w, int h) {
    return (
        -w <= x && x <= _display_width + w &&
        -h <= y && y <= _display_height + h);
}

void ht1632c::sendframe() {

    uint8_t offs;
    uint8_t addr, csm;

    csm = _cs_max;

    noInterrupts();

    for (uint8_t cs = 0; cs < csm; cs++)   {
        addr = cs << 4;
        _chipselect(cs + 1);
        _writebits(HT1632_ID_WR, HT1632_ID_LEN);
        _writebits(0, HT1632_ADDR_LEN);
        for (offs = 0; offs < 16; offs++) {
            _writebits(_fb_green[addr+offs], HT1632_DATA_LEN);
        }
        for (offs = 0; offs < 16; offs++) {
            _writebits(_fb_red[addr+offs], HT1632_DATA_LEN);
        }
        _chipselect(HT1632_CS_NONE);
    }

    interrupts();

}

void ht1632c::setBrightness(uint8_t value) {
    noInterrupts();
    value &= 0x0F;
    _sendcmd(HT1632_CS_ALL, HT1632_CMD_PWM | value);
    interrupts();
}

void ht1632c::begin() {

    _display_width = 32 * _number;
    _display_height = 16;
    _cs_max = 4 * _number;
    _fb_size = 16 * _cs_max;
    _fb_green = (uint8_t*) malloc(_fb_size);
    _fb_red = (uint8_t*) malloc(_fb_size);

    setFont(FONT_DEFAULT);

    pinMode(_data, OUTPUT);
    pinMode(_wr, OUTPUT);
    pinMode(_clk, OUTPUT);
    pinMode(_cs, OUTPUT);

    noInterrupts();
    _sendcmd(HT1632_CS_ALL, HT1632_CMD_SYSDIS);
    _sendcmd(HT1632_CS_ALL, HT1632_CMD_COMS00);
    _sendcmd(HT1632_CS_ALL, HT1632_CMD_MSTMD);
    _sendcmd(HT1632_CS_ALL, HT1632_CMD_RCCLK);
    _sendcmd(HT1632_CS_ALL, HT1632_CMD_SYSON);
    _sendcmd(HT1632_CS_ALL, HT1632_CMD_LEDON);
    _sendcmd(HT1632_CS_ALL, HT1632_CMD_PWM);
    interrupts();

    clear();

}

void ht1632c::clear() {
    memset(_fb_green, 0, _fb_size);
    memset(_fb_red, 0, _fb_size);
    // sendframe();
}

ht1632c::ht1632c(uint8_t d, uint8_t w, \
    uint8_t k, uint8_t c, \
    uint8_t geometry, uint8_t number) : \
    _data(d), _wr(w), _clk(k), _cs(c), _number(number)
    {}

uint8_t ht1632c::getDisplayWidth() {
    return _display_width;
}

uint8_t ht1632c::getDisplayHeight() {
    return _display_height;
}

inline void ht1632c::_update_fb(uint8_t *ptr, uint8_t target, uint8_t pixel) {
    uint8_t &val = *ptr;
    (target) ? val |= pixel : val &= ~pixel;
}

void ht1632c::setPixel(uint8_t x, uint8_t y, uint8_t color) {

    uint8_t val;
    uint8_t addr;

    if (!_inside(x, y)) return;

    addr = (x & 31) + ((x & ~31) << 1) + ((y & ~7) << 2);
    val = 128 >> (y & 7);

    _update_fb(_fb_green + addr, (color & GREEN), val);
    _update_fb(_fb_red + addr, (color & RED), val);

}

uint8_t ht1632c::getPixel(uint8_t x, uint8_t y) {

    uint8_t g, r, val;
    uint16_t addr;

    if (!_inside(x, y)) return BLACK;

    addr = (x & 31) + ((x & ~31) << 1) + ((y & ~7) << 2);
    val = 1 << (7 - (y & 7));
    g = _fb_green[addr];
    r = _fb_red[addr];
    return ((g & val) ? GREEN : BLACK) | ((r & val) ? RED : BLACK);

}

void ht1632c::line(int x0, int y0, int x1, int y1, uint8_t color) {

    int dx =  abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2; /* error value e_xy */

    for (;;) {
        setPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
        if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
    }

}

void ht1632c::rect(int x0, int y0, int x1, int y1, uint8_t color) {
    line(x0, y0, x0, y1, color); /* left line   */
    line(x1, y0, x1, y1, color); /* right line  */
    line(x0, y0, x1, y0, color); /* top line    */
    line(x0, y1, x1, y1, color); /* bottom line */
}

void ht1632c::circle(int xm, int ym, int r, uint8_t color) {
    int x = -r, y = 0, err = 2 - 2 * r; /* II. Quadrant */
    do {
        setPixel(xm - x, ym + y, color); /*   I. Quadrant */
        setPixel(xm - y, ym - x, color); /*  II. Quadrant */
        setPixel(xm + x, ym - y, color); /* III. Quadrant */
        setPixel(xm + y, ym + x, color); /*  IV. Quadrant */
        r = err;
        if (r >  x) err += ++x * 2 + 1; /* e_xy+e_x > 0 */
        if (r <= y) err += ++y * 2 + 1; /* e_xy+e_y < 0 */
    } while (x < 0);
}

void ht1632c::ellipse(int x0, int y0, int x1, int y1, uint8_t color) {

    int a = abs(x1 - x0), b = abs(y1 - y0), b1 = b & 1; /* values of diameter */
    long dx = 4 * (1 - a) * b * b, dy = 4 * (b1 + 1) * a * a; /* error increment */
    long err = dx + dy + b1 * a * a, e2; /* error of 1.step */

    if (x0 > x1) { x0 = x1; x1 += a; } /* if called with swapped points */
    if (y0 > y1) y0 = y1; /* .. exchange them */
    y0 += (b + 1) / 2; /* starting pixel */
    y1 = y0 - b1;
    a *= 8 * a;
    b1 = 8 * b * b;

    do {
        setPixel(x1, y0, color); /*   I. Quadrant */
        setPixel(x0, y0, color); /*  II. Quadrant */
        setPixel(x0, y1, color); /* III. Quadrant */
        setPixel(x1, y1, color); /*  IV. Quadrant */
        e2 = 2 * err;
        if (e2 >= dx) { x0++; x1--; err += dx += b1; } /* x step */
        if (e2 <= dy) { y0++; y1--; err += dy += a; }  /* y step */
    } while (x0 <= x1);

    while (y0 - y1 < b) {  /* too early stop of flat ellipses a=1 */
        setPixel(x0 - 1, ++y0, color); /* -> complete tip of ellipse */
        setPixel(x0 - 1, --y1, color);
    }
}

void ht1632c::bezier(int x0, int y0, int x1, int y1, int x2, int y2, uint8_t color) {

    int sx = x0 < x2 ? 1 : -1, sy = y0 < y2 ? 1 : -1; /* step direction */
    int cur = sx * sy * ((x0 - x1) * (y2 - y1) - (x2 - x1) * (y0 - y1)); /* curvature */
    int x = x0 - 2 * x1 + x2, y = y0 - 2 * y1 + y2, xy = 2 * x * y * sx * sy;

    /* compute error increments of P0 */
    long dx = (1 - 2 * abs(x0 - x1)) * y * y + abs(y0 - y1) * xy - 2 * cur * abs(y0 - y2);
    long dy = (1 - 2 * abs(y0 - y1)) * x * x + abs(x0 - x1) * xy + 2 * cur * abs(x0 - x2);

    /* compute error increments of P2 */
    long ex = (1 - 2 * abs(x2 - x1)) * y * y + abs(y2 - y1) * xy + 2 * cur * abs(y0 - y2);
    long ey = (1 - 2 * abs(y2 - y1)) * x * x + abs(x2 - x1) * xy - 2 * cur * abs(x0 - x2);

    if (cur == 0) { line(x0, y0, x2, y2, color); return; } /* straight line */

    x *= 2 * x; y *= 2 * y;
    if (cur < 0) {                             /* negated curvature */
        x = -x; dx = -dx; ex = -ex; xy = -xy;
        y = -y; dy = -dy; ey = -ey;
    }
    /* algorithm fails for almost straight line, check error values */
    if (dx >= -y || dy <= -x || ex <= -y || ey >= -x) {
        line(x0, y0, x1, y1, color);                /* simple approximation */
        line(x1, y1, x2, y2, color);
        return;
    }
    dx -= xy; ex = dx+dy; dy -= xy;              /* error of 1.step */

    for (;;) {                                         /* plot curve */
        setPixel(x0, y0, color);
        ey = 2 * ex - dy;                /* save value for test of y step */
        if (2 * ex >= dx) {                                   /* x step */
            if (x0 == x2) break;
            x0 += sx; dy -= xy; ex += dx += y;
        }
        if (ey <= 0) {                                      /* y step */
            if (y0 == y2) break;
            y0 += sy; dx -= xy; ex += dy += x;
        }
    }

}

void ht1632c::_fill_r(uint8_t x, uint8_t y, uint8_t color) {
    if (!_inside(x, y)) return;
    if (!getPixel(x, y)) {
        setPixel(x, y, color);
        _fill_r(++x, y ,color);
        x--;
        _fill_r(x, y - 1, color);
        _fill_r(x, y + 1, color);
    }
}

void ht1632c::_fill_l(uint8_t x, uint8_t y, uint8_t color) {
    if (!_inside(x, y)) return;
    if (!getPixel(x, y)) {
        setPixel(x, y, color);
        _fill_l(--x, y, color);
        x++;
        _fill_l(x, y - 1, color);
        _fill_l(x, y + 1, color);
    }
}

void ht1632c::fill(uint8_t x, uint8_t y, uint8_t color) {
    _fill_r(x, y, color);
    _fill_l(x - 1, y, color);
}

void ht1632c::putBitmap(int x, int y, uint16_t *bitmap, uint8_t w, uint8_t h, uint8_t color) {

    uint16_t dots, msb;

    if (!_inside(x, y, w, h)) return;

    msb = 1 << (w - 1);
    for (uint8_t row = 0; row < h; row++) {
        dots = bitmap[row];
        if (dots && color) {
            for (uint8_t col = 0; col < w; col++) {
                if (dots & (msb >> col)) {
                    setPixel(x + col, y + row, color);
                } else {
                    setPixel(x + col, y + row, BLACK);
                }
            }
        }
    }

}

void ht1632c::setFont(uint8_t user_font) {

    switch (user_font) {

        #ifdef FONT_4x6
        case FONT_4x6:
            _font = (uint8_t *) &font_4x6[0];
            _font_width = 4;
            _font_height = 6;
            break;
        #endif

        #ifdef FONT_5x7
        case FONT_5x7:
            _font = (uint8_t *) &font_5x7[0];
            _font_width = 5;
            _font_height = 7;
            break;
        #endif

        #ifdef FONT_5x8
        case FONT_5x8:
            _font = (uint8_t *) &font_5x8[0];
            _font_width = 5;
            _font_height = 8;
            break;
        #endif

        #ifdef FONT_5x7W
        case FONT_5x7W:
            _font = (uint8_t *) &font_5x7w[0];
            _font_width = 5;
            _font_height = 8;
            break;
        #endif

        #ifdef FONT_6x10
        case FONT_6x10:
            _wfont = (uint16_t *) &font_6x10[0];
            _font_width = 6;
            _font_height = 10;
            break;
        #endif

        #ifdef FONT_6x12
        case FONT_6x12:
            _wfont = (uint16_t *) &font_6x12[0];
            _font_width = 6;
            _font_height = 12;
            break;
        #endif

        #ifdef FONT_6x13
        case FONT_6x13:
            _wfont = (uint16_t *) &font_6x13[0];
            _font_width = 6;
            _font_height = 13;
            break;
        #endif

        #ifdef FONT_6x13B
        case FONT_6x13B:
            _wfont = (uint16_t *) &font_6x13B[0];
            _font_width = 6;
            _font_height = 13;
            break;
        #endif

        #ifdef FONT_6x13O
        case FONT_6x13O:
            _wfont = (uint16_t *) &font_6x13O[0];
            _font_width = 6;
            _font_height = 13;
            break;
        #endif

        #ifdef FONT_6x9
        case FONT_6x9:
            _wfont = (uint16_t *) &font_6x9[0];
            _font_width = 6;
            _font_height = 9;
            break;
        #endif

        #ifdef FONT_7x13
        case FONT_7x13:
            _wfont = (uint16_t *) &font_7x13[0];
            _font_width = 7;
            _font_height = 13;
            break;
        #endif

        #ifdef FONT_7x13B
        case FONT_7x13B:
            _wfont = (uint16_t *) &font_7x13B[0];
            _font_width = 7;
            _font_height = 13;
            break;
        #endif

        #ifdef FONT_7x13O
        case FONT_7x13O:
            _wfont = (uint16_t *) &font_7x13O[0];
            _font_width = 7;
            _font_height = 13;
            break;
        #endif

        #ifdef FONT_7x14
        case FONT_7x14:
            _wfont = (uint16_t *) &font_7x14[0];
            _font_width = 7;
            _font_height = 14;
            break;
        #endif

        #ifdef FONT_7x14B
        case FONT_7x14B:
            _wfont = (uint16_t *) &font_7x14B[0];
            _font_width = 7;
            _font_height = 14;
            break;
        #endif

        #ifdef FONT_8x8
        case FONT_8x8:
            _font = (uint8_t *) &font_8x8[0];
            _font_width = 8;
            _font_height = 8;
            break;
        #endif

        #ifdef FONT_8x13
        case FONT_8x13:
            _wfont = (uint16_t *) &font_8x13[0];
            _font_width = 8;
            _font_height = 13;
            break;
        #endif

        #ifdef FONT_8x13B
        case FONT_8x13B:
            _wfont = (uint16_t *) &font_8x13B[0];
            _font_width = 8;
            _font_height = 13;
            break;
        #endif

        #ifdef FONT_8x13O
        case FONT_8x13O:
            _wfont = (uint16_t *) &font_8x13O[0];
            _font_width = 8;
            _font_height = 13;
            break;
        #endif

        #ifdef FONT_9x15
        case FONT_9x15:
            _wfont = (uint16_t *) &font_9x15[0];
            _font_width = 9;
            _font_height = 15;
            break;
        #endif

        #ifdef FONT_9x15B
        case FONT_9x15B:
            _wfont = (uint16_t *) &font_9x15b[0];
            _font_width = 9;
            _font_height = 15;
            break;
        #endif

        #ifdef FONT_8x16
        case FONT_8x16:
            _wfont = (uint16_t *) &font_8x16[0];
            _font_width = 8;
            _font_height = 16;
            break;
        #endif

        #ifdef FONT_8x16B
        case FONT_8x16B:
            _wfont = (uint16_t *) &font_8x16b[0];
            _font_width = 8;
            _font_height = 16;
            break;
        #endif

        #ifdef FONT_8x13BK
        case FONT_8x13BK:
            _wfont = (uint16_t *) &font_8x13bk[0];
            _font_width = 8;
            _font_height = 13;
            break;
        #endif

        #ifdef FONT_7x12BN
        case FONT_7x12BN:
            _wfont = (uint16_t *) (font_7x12bn + 4);
            _font_width = 6;
            _font_height = 12;
            break;
        #endif
    }
}


uint8_t ht1632c::putChar(int x, int y, char c, uint8_t color, uint8_t bgcolor) {

    if (!_inside(x, y, _font_width, _font_height)) return x;

    if ((unsigned char) c >= 0xc0) c -= 0x41;
    c -= 0x20;

    uint16_t msb = 1 << (_font_height - 1);

    if (_font_height > 8) {

        uint16_t *addr = _wfont + c * _font_width;
        for (uint16_t col = 0; col < _font_width; col++) {
            uint16_t dots = addr[col];
            for (uint16_t row = 0; row < _font_height; row++) {
                setPixel(x + col, y + row, (dots & (msb >> row)) ? color : bgcolor);
            }
        }

    } else {

        uint8_t *addr = _font + c * _font_width;
        for (uint8_t col = 0; col < _font_width; col++) {
            uint8_t dots = addr[col];
            for (uint8_t row = 0; row < _font_height; row++) {
                setPixel(x + col, y + row, (dots & (msb >> row)) ? color : bgcolor);
            }
        }

    }

    return x + _font_width;

}

void ht1632c::putText(int x, int y, const char *text, uint8_t color, uint8_t align) {

    int len = strlen(text);

    if (align & ALIGN_CENTER) {
        x = (_display_width - _font_width * len) / 2;
    } else if (align & ALIGN_RIGHT) {
        x = (_display_width - x - _font_width * len);
    }

    if (align & ALIGN_MIDDLE) {
        y = (_display_height - _font_height) / 2;
    } else if (align & ALIGN_BOTTOM) {
        y = (_display_height - y - _font_height);
    }

    int position = 0;
    for (int i = 0; i < len; i++) {
        putChar(x + _font_width * position++,  y, text[i], color);
    }

    //sendframe();

}

