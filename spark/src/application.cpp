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

#define STATUS_X                1
#define STATUS_Y                1
#define STATUS_WIDTH            62
#define STATUS_CHAR_WIDTH       6
#define STATUS_MIN_SPACE        2
#define STATUS_MAX_CHARS        ((STATUS_WIDTH - 4 * STATUS_MIN_SPACE) / STATUS_CHAR_WIDTH)

#define STATE_STARTUP          0
#define STATE_COUNTS           1
#define STATE_MESSAGE          2
#define STATE_TIMEOUT          3

#define NOTIFICATION_OK         0x1
#define NOTIFICATION_WARNING    0x2
#define NOTIFICATION_CRITICAL   0x4

AudioPlayer player;
FlashPlayer flash_player(player);
ht1632c matrix = ht1632c(MATRIX_DATA_PIN, MATRIX_WR_PIN, MATRIX_CLK_PIN,
                         MATRIX_CS_PIN, GEOM_32x16, 2);

void setup()
{
    // Serial over USB used for debugging
    Serial.begin(115200);
    Serial.println("Starting...");

    // Serial line used for control
    Serial1.begin(115200);

    // Set up the LED matrix
    matrix.begin();
    matrix.setBrightness(BRIGHTNESS);
    message1("Starting...", ORANGE);
    matrix.rect(0, 0, 63, 15, ORANGE);
    matrix.sendframe();

    // Audio player initialization
    player.begin();
    // play_notification(NOTIFICATION_OK);
}


void loop()
{
    // Tune brightness to ambient luminosity
    int32_t brightness = analogRead(A7);
    matrix.setBrightness(map(brightness, 0, 0xfff, 0, 0xf));

    // Read serial commands
    static String input = "";
    while (Serial1.available()) {
        char c = (char) Serial1.read();
        if ((c == '\r' || c == '\n')) {
            if (input.length() > 0) {
                display_scores(input);
                //play_notification(NOTIFICATION_OK);
            }
            input = "";
        } else {
            input += c;
        }
    }
}


void display_scores(String &input)
{
    String p1_set = input.substring(0, 1);
    String p1_game = input.substring(2, 4);
    String p2_game = input.substring(5, 7);
    String p2_set = input.substring(8, 9);

    long service = input.substring(10, 11).toInt();

    matrix.clear();
    matrix.setFont(FONT_8x16B);

    matrix.putText(1, 0, p1_set.c_str(), ORANGE);
    matrix.putText(13, 0, p1_game.c_str(), RED);
    matrix.putText(35, 0, p2_game.c_str(), RED);
    matrix.putText(55, 0, p2_set.c_str(), ORANGE);

    if (service == 1) {
        matrix.line(1, 15, 28, 15, GREEN);
    } else if (service == 2) {
        matrix.line(35, 15, 62, 15, GREEN);
    }

    matrix.sendframe();
}


/**
 * Show a message, 15 chars per line, 2 lines max.
 */
void message(char* line1, char *line2, uint8_t color)
{
    matrix.setFont(FONT_4x6);
    if (line1) {
        matrix.clear();
        matrix.putText(2, 2, line1, color);
    }
    if (line2) {
        matrix.putText(2, 9, line2, color);
    }
}


void message1(char* line1, uint8_t color)
{
    message(line1, NULL, color);
}


void message2(char* line2, uint8_t color)
{
    message(NULL, line2, color);
}


void play_notification(uint8_t notification)
{
    return;

    // Block until the player is available
    while (!flash_player.available()) {
        delay(100);
    }

    // Play the notification
    switch (notification) {
      case NOTIFICATION_OK:
        Serial.println("Playing notification for OK");
        flash_player.play(RESOURCE_CHIME_START, RESOURCE_CHIME_END);
        break;
      case NOTIFICATION_WARNING:
        Serial.println("Playing notification for WARNING");
        flash_player.play(RESOURCE_WILHELM_START, RESOURCE_WILHELM_END);
        break;
      case NOTIFICATION_CRITICAL:
        Serial.println("Playing notification for CRITICAL");
        flash_player.play(RESOURCE_SIREN_START, RESOURCE_SIREN_END);
        break;
      default:
        Serial.print("Invalid notification code ");
        Serial.println(notification);
        break;
    }
}
