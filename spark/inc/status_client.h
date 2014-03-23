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

#ifndef STATUS_CLIENT_H_
#define STATUS_CLIENT_H_

// Not reciving an update for this many seconds triggers an error
#define CLIENT_TIMEOUT          30

// Connected and receiving status updates
#define STATE_OK                0
// No connection to the network available
#define STATE_NO_NETWORK        1
// Connected to the status server and waiting for the first update
#define STATE_CONNECTED         2
// Error during connection to the server
#define STATE_CONNECTION_ERROR  3
// Could not decode the server's message
#define STATE_READ_ERROR        4
// The server reported an error connecting to Nagios
#define STATE_UPSTREAM_ERROR    5
// Connected but no update received for CLIENT_TIMEOUT seconds
#define STATE_TIMEOUT           6

// Message types received from the server
#define MESSAGE_UPDATE          1
#define MESSAGE_ERROR           2

// Notification flags
#define NOTIFICATION_OK         0x01
#define NOTIFICATION_WARNING    0x02
#define NOTIFICATION_CRITICAL   0x04

#include "spark_wiring.h"
#include "spark_wiring_tcpclient.h"

struct NagiosStatus {
    uint32_t critical_count;
    uint32_t warning_count;
    uint32_t ok_count;
    uint32_t critical_serial;
    uint32_t warning_serial;
    uint32_t ok_serial;
};

class StatusClient
{
  public:
    StatusClient();
    virtual ~StatusClient();
    void begin(const IPAddress &ip, uint16_t port, const char *secret);
    uint8_t update();

    uint8_t state() { return _state; };
    uint8_t notifications() { return _notifications; };
    void clearNotifications() { _notifications = 0; };

    uint32_t criticalCount() { return _status.critical_count; };
    uint32_t warningCount() { return _status.warning_count; };
    uint32_t okCount() { return _status.ok_count; };

  private:
    IPAddress _ip;
    uint16_t _port;
    char * _secret;

    TCPClient _client;
    system_tick_t _last_update;
    uint8_t _state;
    uint8_t _notifications;
    NagiosStatus _status;
    NagiosStatus _buffer;

    uint8_t read();
};

#endif  // STATUS_CLIENT_H_
