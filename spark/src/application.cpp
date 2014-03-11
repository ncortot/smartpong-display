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


AudioPlayer player;
FlashPlayer flash_player(player);


void setup()
{
    Spark.disconnect();
    pinMode(D7, OUTPUT);

    player.begin();
    player.beep(250);
    delay(250);
}

void loop()
{
    if (flash_player.available()) {
        flash_player.play(RESOURCE_CHIME_START, RESOURCE_WILHELM_END);
    }
}
