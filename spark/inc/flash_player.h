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

#ifndef FLASH_PLAYER_H_
#define FLASH_PLAYER_H_

#include "spark_wiring.h"
#include "audio_player.h"
#include "resources.h"

class FlashPlayer
{
  public:

    FlashPlayer(AudioPlayer &player);
    virtual ~FlashPlayer() {};
    virtual bool available();
    virtual void play(uint32_t start, uint32_t end);

  private:

    AudioPlayer _player;
    uint8_t _buffer[RESOURCE_BUFFER_SIZE];

};

#endif  // FLASH_PLAYER_H_
