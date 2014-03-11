/**

  Nagios Monitor for the Spark Core
  Copyright (C) 2014 Nicolas Cortot
  https://github.com/ncortot/spark-nagios-monitor

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

#include "application.h"
#include "audio_player.h"
#include "flash_player.h"
#include "ht1632c.h"


#define MATRIX_DATA_PIN         D3 // blue
#define MATRIX_CLK_PIN          D4 // white
#define MATRIX_CS_PIN           D5 // yellow
#define MATRIX_WR_PIN           D6 // green

#define BRIGHTNESS              0xa
#define STATUS_X                3
#define STATUS_Y                1


AudioPlayer player;
FlashPlayer flash_player(player);
ht1632c matrix = ht1632c(MATRIX_DATA_PIN, MATRIX_WR_PIN, MATRIX_CLK_PIN,
                         MATRIX_CS_PIN, GEOM_32x16, 2);


void setup()
{
    pinMode(D7, OUTPUT);

    matrix.begin();
    matrix.setBrightness(BRIGHTNESS);
    matrix.setFont(FONT_DEFAULT);
    matrix.clear();
    matrix.rect(0, 0, 63, 15, ORANGE);
    matrix.sendframe();

    player.begin();
    player.beep(250);
    delay(250);
}


void update_idle()
{
    static uint8_t position = 1;
    static uint8_t direction = 1;

    matrix.setPixel(position, 14, BLACK);

    position += direction;

    if (position < 1) {
        position = 1;
        direction = 1;
    }

    if (position > 62) {
        position = 62;
        direction = -1;
    }

    matrix.setPixel(position, 14, GREEN);
}


void loop()
{
    if (flash_player.available()) {
        flash_player.play(RESOURCE_CHIME_START, RESOURCE_WILHELM_END);
    }

    update_idle();
    matrix.sendframe();
}
