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

#ifndef HT1632C_H_INCLUDED
#define HT1632C_H_INCLUDED

#pragma GCC diagnostic ignored "-Wwrite-strings"

/*
 * commands written to the chip consist of a 3 bit "ID", followed by
 * either 9 bits of "Command code" or 7 bits of address + 4 bits of data.
 */
#define HT1632_ID_CMD       4       /* ID = 100 - Commands */
#define HT1632_ID_RD        6       /* ID = 110 - Read RAM */
#define HT1632_ID_WR        5       /* ID = 101 - Write RAM */

#define HT1632_CMD_SYSDIS   0x00    /* CMD= 0000-0000-x Turn off oscil */
#define HT1632_CMD_SYSON    0x01    /* CMD= 0000-0001-x Enable system oscil */
#define HT1632_CMD_LEDOFF   0x02    /* CMD= 0000-0010-x LED duty cycle gen off */
#define HT1632_CMD_LEDON    0x03    /* CMD= 0000-0011-x LEDs ON */
#define HT1632_CMD_BLOFF    0x08    /* CMD= 0000-1000-x Blink ON */
#define HT1632_CMD_BLON     0x09    /* CMD= 0000-1001-x Blink Off */
#define HT1632_CMD_SLVMD    0x10    /* CMD= 0001-00xx-x Slave Mode */
#define HT1632_CMD_MSTMD    0x14    /* CMD= 0001-01xx-x Master Mode */
#define HT1632_CMD_RCCLK    0x18    /* CMD= 0001-10xx-x Use on-chip clock */
#define HT1632_CMD_EXTCLK   0x1C    /* CMD= 0001-11xx-x Use external clock */
#define HT1632_CMD_COMS00   0x20    /* CMD= 0010-ABxx-x commons options */
#define HT1632_CMD_COMS01   0x24    /* CMD= 0010-ABxx-x commons options */
#define HT1632_CMD_COMS10   0x28    /* CMD= 0010-ABxx-x commons options */
#define HT1632_CMD_COMS11   0x2C    /* CMD= 0010-ABxx-x commons options */
#define HT1632_CMD_PWM      0xA0    /* CMD= 101x-PPPP-x PWM duty cycle */

#define HT1632_ID_LEN       (1 << 2)    /* IDs are 3 bits */
#define HT1632_CMD_LEN      (1 << 7)    /* CMDs are 8 bits */
#define HT1632_DATA_LEN     (1 << 7)    /* Data are 4*2 bits */
#define HT1632_ADDR_LEN     (1 << 6)    /* Address are 7 bits */

#define HT1632_CS_NONE      0       /* None of ht1632c selected */
#define HT1632_CS_ALL       32      /* All of ht1632c selected */

#define PULSE_WIDTH_USEC    0       /* Puse width in microseconds */

#define GEOM_32x16          32      /* 32x16 */
#define GEOM_24x16          24      /* 24x16 */
#define GEOM_32x8           8       /* 32x8 */

#define BLACK               0
#define GREEN               1
#define RED                 2
#define ORANGE              3

#define ALIGN_LEFT          0
#define ALIGN_RIGHT         1
#define ALIGN_CENTER        2
#define ALIGN_TOP           4
#define ALIGN_BOTTOM        8
#define ALIGN_MIDDLE        16

#define SCROLL_LEFT         1
#define SCROLL_RIGHT        2
#define SCROLL_UP           4
#define SCROLL_DOWN         8


#define FONT_DEFAULT        23
#define FONT_4x6            1
//#define FONT_5x7          2
//#define FONT_5x8          3
//#define FONT_5x7W         4
//#define FONT_6x10         5
//#define FONT_6x12         6
//#define FONT_6x13         7
//#define FONT_6x13B          8
//#define FONT_6x13O        9
//#define FONT_6x9          10
//#define FONT_7x13         11
//#define FONT_7x13B        12
//#define FONT_7x13O        13
//#define FONT_7x14         14
//#define FONT_7x14B        15
//#define FONT_8x8          16
//#define FONT_8x13         17
//#define FONT_8x13B        18
//#define FONT_8x13O        19
//#define FONT_9x15         20
//#define FONT_9x15B        21
//#define FONT_8x16         22
#define FONT_8x16B        23

// Custom font, numbers only
//#define FONT_7x12BN       24

class ht1632c {

    private:

        void _set(uint8_t pin);
        void _unset(uint8_t pin);
        void _pulse(uint8_t pin, uint8_t pulses);

        uint8_t _div(uint8_t n, uint8_t d);

        void _chipselect(uint8_t cs);
        void _writebits(uint8_t bits, uint8_t msb);
        inline void _update_fb(uint8_t *ptr, uint8_t target, uint8_t pixel);

        void _sendcmd(uint8_t cs, uint8_t command);

        void _fill_r(uint8_t x, uint8_t y, uint8_t color);
        void _fill_l(uint8_t x, uint8_t y, uint8_t color);

        boolean _inside(int x, int y, int w = 0, int h = 0);

        uint8_t *_fb_green;
        uint8_t *_fb_red;

        uint8_t _data;
        uint8_t _wr;
        uint8_t _clk;
        uint8_t _cs;

        uint8_t _number;
        uint8_t _display_width;
        uint8_t _display_height;
        uint8_t _cs_max;
        uint16_t _fb_size;

        uint8_t *_font;
        uint16_t *_wfont;
        uint8_t _font_width;
        uint8_t _font_height;

    public:

        ht1632c(uint8_t data, uint8_t wr, uint8_t clk, uint8_t cs, uint8_t geometry, uint8_t number);

        void begin();
        void sendframe();
        void setBrightness(uint8_t value);
        void clear();

        void setPixel(uint8_t x, uint8_t y, uint8_t color);
        uint8_t getPixel(uint8_t x, uint8_t y);

        uint8_t getDisplayWidth();
        uint8_t getDisplayHeight();

        void line(int x0, int y0, int x1, int y1, uint8_t color);
        void rect(int x0, int y0, int x1, int y1, uint8_t color);
        void circle(int xm, int ym, int r, uint8_t color);
        void ellipse(int x0, int y0, int x1, int y1, uint8_t color);
        void fill(uint8_t x, uint8_t y, uint8_t color);
        void bezier(int x0, int y0, int x1, int y1, int x2, int y2, uint8_t color);

        void putBitmap(int x, int y, uint16_t *bitmap, uint8_t w, uint8_t h, uint8_t color);
        void setFont(uint8_t userfont);
        uint8_t putChar(int x, int y, char c, uint8_t color, uint8_t bgcolor = BLACK);
        void putText(int x, int y, const char *text, uint8_t color, uint8_t align = ALIGN_LEFT | ALIGN_TOP);

};

#endif // HT1632C_H_INCLUDED
