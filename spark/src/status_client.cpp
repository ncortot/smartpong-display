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

#include "status_client.h"
#include "spark_wiring_usartserial.h"


StatusClient::StatusClient()
{
}


StatusClient::~StatusClient()
{
}


void StatusClient::begin(const IPAddress &ip, uint16_t port, const char *secret)
{
    _ip = ip;
    _port = port;

    size_t len = strlen(secret);
    _secret = (char *) malloc(len + 2);
    memcpy(_secret, secret, len);
    _secret[len] = '\n';
    _secret[len + 1] = 0;
}


uint8_t StatusClient::update()
{
    if (WiFi.status() == WIFI_ON) {
        if (_client.connected()) {
            if (_client.available()) {
                _state = read();
                if (_state == STATE_OK) {
                    _last_update = millis();
                }
            }
            if (_state == STATE_OK && millis() - _last_update > 1000 * CLIENT_TIMEOUT) {
                Serial1.println("Update Timeout");
                _state = STATE_TIMEOUT;
            }
        } else {
            Serial1.println("Connecting...");
            _client.connect(_ip, _port);
            if (_client.connected()) {
                Serial1.println("Connected");
                _client.print(_secret);
                _state = STATE_CONNECTED;
            } else {
                Serial1.println("Connection Error");
                _state = STATE_CONNECTION_ERROR;
            }
        }
    } else {
        Serial1.println("No Network");
        _state = STATE_NO_NETWORK;
    }
    return _state;
}


uint8_t StatusClient::read()
{
    // Read the message type
    int32_t message_type;
    size_t bytes = _client.read((uint8_t *) &message_type, sizeof(message_type));
    if (bytes < 1) {
        Serial1.println("Error reading message type");
        return STATE_READ_ERROR;
    }

    // Decode message type as big-endian
    message_type = __REV(message_type);
    switch (message_type) {
      case MESSAGE_UPDATE:
        break;
      case MESSAGE_ERROR:
        Serial1.println("Upstream error");
        return STATE_UPSTREAM_ERROR;
      default:
        Serial1.print("Unknown message type ");
        Serial1.print(message_type, HEX);
        Serial1.println();
        return STATE_READ_ERROR;
    }

    // Read the message contents
    bytes = _client.read((uint8_t *) &_buffer, sizeof(NagiosStatus));
    if (bytes < sizeof(_buffer)) {
        Serial1.println("Error reading message contents");
        return STATE_READ_ERROR;
    }

    // Switch to little-endian values
    uint32_t *ptr = (uint32_t *) &_buffer;
    uint32_t *max_ptr = ptr + sizeof(NagiosStatus);
    for (; ptr < max_ptr; ++ptr)
        *ptr = __REV(*ptr);

    // Update the notification flags
    if (_buffer.critical_serial > _status.critical_serial)
        _notifications |= NOTIFICATION_CRITICAL;
    if (_buffer.warning_serial > _status.warning_serial)
        _notifications |= NOTIFICATION_WARNING;
    if (_buffer.ok_serial > _status.ok_serial)
        _notifications |= NOTIFICATION_OK;

    // Store the new status
    memcpy(&_state, &_buffer, sizeof(NagiosStatus));

    // Debug logging
    Serial1.print("New status: ");
    Serial1.print(criticalCount());
    Serial1.print(" ");
    Serial1.print(warningCount());
    Serial1.print(" ");
    Serial1.print(okCount());
    Serial1.print(" , notifications: 0x");
    Serial1.print(notifications(), HEX);
    Serial1.println();

    return STATE_OK;
}
