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

unsigned long last_update = 0;
uint8_t status_color = ORANGE;
uint8_t message_color = ORANGE;

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
    play_notification(NOTIFICATION_OK);

    last_update = millis();
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
                command(input);
            }
            input = "";
        } else {
            input += c;
        }
    }

    // Check for command timeout after 1 min
    if (millis() - last_update > 60000) {
        Serial.println("Command timeout");
        matrix.clear();
        status_color = ORANGE;
        message1("Timeout...", ORANGE);
    }

    update_idle();
    matrix.rect(0, 0, 63, 15, status_color);
    matrix.sendframe();
}


void command(String &input)
{
    static char msg[16];

    if (input.startsWith("UPDATE ")) {
        // Display new status counts
        String args = command_args(input);
        Serial.print("UPDATE: ");
        Serial.println(args);
        unsigned int start1 = args.indexOf(' ');
        unsigned int start2 = args.indexOf(' ', start1 + 1);
        long critical = args.substring(0, start1).toInt();
        long warning = args.substring(start1, start2).toInt();
        long ok = args.substring(start2).toInt();
        matrix.clear();
        display_counts(critical, warning, ok);
        last_update = millis();
    } else if (input.startsWith("NOTIFY ")) {
        // Play a notification
        String args = command_args(input);
        Serial.print("NOTIFY: ");
        Serial.println(args);
        if (args == "OK") {
          play_notification(NOTIFICATION_OK);
        } else if (args == "WARNING") {
          play_notification(NOTIFICATION_WARNING);
        } else if (args == "CRITICAL") {
          play_notification(NOTIFICATION_CRITICAL);
        }
    } else if (input.startsWith("COLOR ")) {
        // Change the message color
        String args = command_args(input);
        Serial.print("COLOR: ");
        Serial.println(args);
        if (args == "GREEN") {
          message_color = GREEN;
        } else if (args == "ORANGE") {
          message_color = ORANGE;
        } else if (args == "RED") {
          message_color = RED;
        }
    } else {
        // Display a text message
        Serial.print("MESSAGE: ");
        Serial.println(input);
        matrix.clear();
        message1(msg, message_color);
        input.toCharArray(msg, 16);
        message2(msg, message_color);
        last_update = millis();
    }
}


String command_args(String &input)
{
    unsigned int start = input.indexOf(' ');
    return input.substring(start + 1);
}


void display_counts(long critical, long warning, long ok)
{
    Serial.print("Counts: ");
    Serial.print(critical);
    Serial.print(", ");
    Serial.print(warning);
    Serial.print(", ");
    Serial.println(ok);

    status_color = GREEN;
    if (warning > 0)
        status_color = ORANGE;
    if (critical > 0)
        status_color = RED;

    String critical_text(critical);
    String warning_text(warning);
    String ok_text(ok);

    if (critical == 0)
        critical_text = "-";
    if (warning == 0)
        warning_text = "-";

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

    matrix.setFont(FONT_6x13B);

    uint8_t space_x = STATUS_WIDTH - length * STATUS_CHAR_WIDTH;

    uint8_t status_x = STATUS_X + space_x / 4;
    matrix.putText(status_x, STATUS_Y, critical_text.c_str(), critical > 0 ? RED : GREEN);

    if (warning_text.length()) {
        status_x = STATUS_X + critical_text.length() * STATUS_CHAR_WIDTH + space_x / 2;
        matrix.putText(status_x, STATUS_Y, warning_text.c_str(), warning > 0 ? ORANGE : GREEN);
    }

    if (ok_text.length()) {
        status_x = STATUS_X + STATUS_WIDTH - ok_text.length() * STATUS_CHAR_WIDTH - space_x / 4;
        matrix.putText(status_x, STATUS_Y, ok_text.c_str(), GREEN);
    }
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
