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
#include "status_client.h"


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

#define SERVER_ADDRESS          192, 168, 1, 39
#define SERVER_PORT             2222
#define SERVER_SECRET           "Mult1P8$$"


AudioPlayer player;
FlashPlayer flash_player(player);
ht1632c matrix = ht1632c(MATRIX_DATA_PIN, MATRIX_WR_PIN, MATRIX_CLK_PIN,
                         MATRIX_CS_PIN, GEOM_32x16, 2);
StatusClient client;

uint8_t status_color = ORANGE;


void setup()
{
    pinMode(D7, OUTPUT);

    Serial1.begin(9600);
    Serial1.println("Starting...");

    matrix.begin();
    matrix.setBrightness(BRIGHTNESS);
    matrix.setFont(FONT_DEFAULT);
    matrix.clear();
    matrix.rect(0, 0, 63, 15, ORANGE);
    matrix.sendframe();

    client.begin(IPAddress(SERVER_ADDRESS), SERVER_PORT, SERVER_SECRET);

    player.begin();
    flash_player.play(RESOURCE_CHIME_START, RESOURCE_CHIME_END);
    while (!flash_player.available());
}


void display_counts()
{
    String critical_text(client.criticalCount());
    String warning_text(client.warningCount());
    String ok_text(client.okCount());

    uint8_t length = critical_text.length() + warning_text.length() + ok_text.length();
    if (length > STATUS_MAX_CHARS) {
        ok_text = String("");
        length = critical_text.length() + warning_text.length();
    }
    if (length > STATUS_MAX_CHARS) {
        warning_text = String("");
        length = critical_text.length();
    }
    if (length > STATUS_MAX_CHARS) {
        critical_text = String("TOO MANY");
        length = critical_text.length();
    }

    uint8_t space_x = STATUS_WIDTH - length * STATUS_CHAR_WIDTH;

    uint8_t status_x = STATUS_X + space_x / 4;
    matrix.putText(status_x, STATUS_Y, critical_text.c_str(), RED);

    if (warning_text.length()) {
        status_x = STATUS_X + critical_text.length() * STATUS_CHAR_WIDTH + space_x / 2;
        matrix.putText(status_x, STATUS_Y, warning_text.c_str(), ORANGE);
    }

    if (ok_text.length()) {
        status_x = STATUS_X + STATUS_WIDTH - ok_text.length() * STATUS_CHAR_WIDTH - space_x / 4;
        matrix.putText(status_x, STATUS_Y, ok_text.c_str(), GREEN);
    }
}


void play_notifications()
{
    if (client.notifications() & NOTIFICATION_CRITICAL) {
        Serial1.println("Playing notification for CRITICAL");
        flash_player.play(RESOURCE_SIREN_START, RESOURCE_SIREN_END);
    } else if (client.notifications() & NOTIFICATION_WARNING) {
        Serial1.println("Playing notification for WARNING");
        flash_player.play(RESOURCE_WILHELM_START, RESOURCE_WILHELM_END);
    } else if (client.notifications() & NOTIFICATION_OK) {
        Serial1.println("Playing notification for OK");
        flash_player.play(RESOURCE_CHIME_START, RESOURCE_CHIME_END);
    }
    client.clearNotifications();
}


/**
 * Show a message, 15 chars per line, 2 lines max.
 */
void message(char* line1, char *line2, uint8_t color)
{
    Serial1.print("Message: ");
    if (line1)
        Serial1.print(line1);
    if (line1 && line2)
        Serial1.print(" - ");
    if (line2)
        Serial1.print(line2);
    Serial1.println();

    matrix.clear();
    matrix.setFont(FONT_4x6);
    if (line1)
        matrix.putText(2, 2, line1, color);
    if (line2)
        matrix.putText(2, 9, line2, color);
    matrix.setFont(FONT_DEFAULT);
    matrix.rect(0, 0, 63, 15, color);
    matrix.sendframe();
}


void message(char* line1, uint8_t color)
{
    message(line1, NULL, color);
}


void update_status()
{
    matrix.clear();

    switch (client.state()) {
      case STATE_OK:
        status_color = GREEN;
        if (client.warningCount() > 0)
            status_color = ORANGE;
        if (client.criticalCount() > 0)
            status_color = RED;
        display_counts();
        // play_notifications();
        break;
      case STATE_NO_NETWORK:
        status_color = RED;
        message("No Network", status_color);
        break;
      case STATE_CONNECTED:
        status_color = ORANGE;
        message("Connected", status_color);
        break;
      case STATE_CONNECTION_ERROR:
        status_color = RED;
        message("Connection", "Error", status_color);
        break;
      case STATE_READ_ERROR:
        status_color = RED;
        message("Read Error", status_color);
        break;
      case STATE_UPSTREAM_ERROR:
        status_color = ORANGE;
        message("Upstream Error", status_color);
        break;
      case STATE_TIMEOUT:
        status_color = ORANGE;
        message("Server Timeout", status_color);
        break;
      default:
        status_color = RED;
        message("State Error", status_color);
        break;
    }

    matrix.rect(0, 0, 63, 15, status_color);
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

    matrix.setPixel(position, 14, status_color);
}


void loop()
{
    int32_t brightness = analogRead(A7);
    matrix.setBrightness(map(brightness, 0, 0xfff, 0, 0xf));

    // Update roughly every 1s
    static uint8_t last_tick = 0;
    uint8_t current_tick = millis() >> 10;
    if (last_tick ^ current_tick) {
        client.update();
        update_status();
        last_tick = current_tick;
    }

    update_idle();
    matrix.sendframe();
    while (!flash_player.available());
}
