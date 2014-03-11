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

#include "flash_player.h"

#define FLASH_DATA_START        0x00080000
#define FLASH_DATA_END          0x00200000

#define HALF_BUFFER_SIZE        RESOURCE_BUFFER_SIZE / 2


uint8_t * volatile buffer;
uint32_t volatile offset = 0;
uint32_t volatile offset_max = 0;


bool callback(bool transfer_complete)
{
    // Read into the first or second half of the buffer
    uint8_t *next = buffer;
    if (transfer_complete) {
        next += HALF_BUFFER_SIZE;
    }

    // Read data from the flash
    sFLASH_ReadBuffer(next, offset, HALF_BUFFER_SIZE);
    offset += HALF_BUFFER_SIZE;

    if (offset > offset_max) {
        // Stop playing
        offset = 0;
        return false;
    } else {
        // Continue
        return true;
    }
}


FlashPlayer::FlashPlayer(AudioPlayer &player) : _player(player)
{
}


/**
 * Return true if the flash player can play a sound.
 *
 * Return false if another sound is currently playing.
 */
bool FlashPlayer::available()
{
    return _player.available();
}


/**
 * Play a sound from the external sFLASH chip.
 *
 * The data is read from addresses `start` to `end`. Addresses must be aligned
 * to multiples of RESOURCES_BUFFER_SIZE bytes.
 *
 * Keep in mind that addresses below 0x00080000 are reserved for the system.
 */
void FlashPlayer::play(uint32_t start, uint32_t end)
{
    if (available() && start >= FLASH_DATA_START && end < FLASH_DATA_END) {
        buffer = _buffer;
        offset = start;
        offset_max = end;

        // Fill the buffer
        callback(false);
        callback(true);

        _player.play(buffer, RESOURCE_BUFFER_SIZE, callback);
    }
}
