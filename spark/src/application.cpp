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
#define STATUS_CHAR_X           6
#define STATUS_SPACE_X          2

#define STATUS_SERVER           10, 0, 0, 1
#define STATUS_PORT             2222
#define CLIENT_TOKEN            "6ayg7f1o57fvMV0vh8CSBqqlnTDIebVCQ38cKkUn2WqrrWYZXrvNXewDWXrZBsyS"


AudioPlayer player;
FlashPlayer flash_player(player);
ht1632c matrix = ht1632c(MATRIX_DATA_PIN, MATRIX_WR_PIN, MATRIX_CLK_PIN,
                         MATRIX_CS_PIN, GEOM_32x16, 2);

IPAddress server(STATUS_SERVER);
TCPClient client;
uint32_t client_buffer[8];

uint32_t critical = 0;
uint32_t warning = 0;
uint32_t good = 0;

uint8_t status_color = ORANGE;


void str_reverse2(char* buffer){
	char *i, *j;
	char c;
	i=buffer;
	j=buffer + strlen(buffer)-1;
	while(i<j){
		c = *i;
		*i = *j;
		*j = c;
		++i;
		--j;
	}
}

char* utoa2(uint32_t a, char* buffer)
{
	char* ptr=buffer;
	div_t result;
	if(a==0){
		ptr[0] = '0';
		ptr[1] = '\0';
		return buffer;
	}
	while(a){
		result.quot = a / 10;
		result.rem = a % 10;
		*ptr = result.rem;
		if(result.rem<10){
			*ptr += '0';
		}else{
			*ptr += 'a'-10;
		}
		++ptr;
		a = result.quot;
	}
	*ptr = '\0';
	str_reverse2(buffer);
	return buffer;
}


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
    flash_player.play(RESOURCE_CHIME_START, RESOURCE_CHIME_END);
}


/**
 * Show a message, 15 chars per line, 2 lines max.
 */
void message(char* line1, char *line2, uint8_t color)
{
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


void update_status()
{
    if (client.connected() && client.available()) {
        matrix.clear();

        int read = client.read((uint8_t *) client_buffer, 16);

        if (read == 16) {
            critical = client_buffer[1];
            warning = client_buffer[2];
            good = client_buffer[3];
        } else {
            critical = 0;
            warning = 0;
            good = 0;
        }

        status_color = GREEN;
        if (warning > 0)
            status_color = ORANGE;
        if (critical > 0)
            status_color = RED;

        int status_x = STATUS_X;
        char status_text[18];

        utoa2(critical, status_text);
        matrix.putText(status_x, STATUS_Y, status_text, RED);
        status_x += strlen(status_text) * STATUS_CHAR_X + STATUS_SPACE_X;

        utoa2(warning, status_text);
        matrix.putText(status_x, STATUS_Y, status_text, ORANGE);
        status_x += strlen(status_text) * STATUS_CHAR_X + STATUS_SPACE_X;

        utoa2(good, status_text);
        matrix.putText(status_x, STATUS_Y, status_text, GREEN);
        status_x += strlen(status_text) * STATUS_CHAR_X + STATUS_SPACE_X;

        matrix.rect(0, 0, 63, 15, status_color);
    } else if (!client.connected()) {
        status_color = ORANGE;
        matrix.putText(STATUS_X, STATUS_Y, "UNKNOWN", ORANGE);
        matrix.rect(0, 0, 63, 15, status_color);
    }

}


void loop()
{
    int32_t brightness = analogRead(A7);
    matrix.setBrightness(map(brightness, 0, 0xfff, 0, 0xf));

    if (WiFi.status() == WIFI_ON) {
        if (client.connected()) {
            update_status();
        } else {
            message("Connecting...", ORANGE);
            delay(500);
            client.connect(server, STATUS_PORT);
            if (client.connected()) {
                message("Connected", GREEN);
                client.println(CLIENT_TOKEN);
                delay(500);
            } else {
                message("Connection Error", RED);
                delay(2000);
            }
        }
    } else {
        message("No Network", RED);
    }

    update_idle();
    matrix.sendframe();
}
