#!/usr/bin/env python

import asynchat
import asyncore
import base64
import urllib.request
import socket
import struct
import sys
import time

from bs4 import BeautifulSoup


CLIENT_TOKEN = '6ayg7f1o57fvMV0vh8CSBqqlnTDIebVCQ38cKkUn2WqrrWYZXrvNXewDWXrZBsyS'

POLL_INTERVAL = 10


class NagiosServer(object):
    """"Connect to a Nagios server and report its status.

    Cache data and poll the Nagios server every POLL_INTERVAL seconds only.

    Probably not thread-safe, this should be investigated.

    """

    def __init__(self, url, username, password):
        problems_url = '{0}/cgi-bin/status.cgi?host=all&servicestatustypes=28'.format(url)
        token = '{0}:{1}'.format(username, password)
        auth = base64.encodestring(token.encode('latin1')).decode('ascii')
        print(problems_url);
        self.request = urllib.request.Request(problems_url)
        self.request.add_header('Authorization', 'Basic %s' % auth)
        self.__status = None
        self.__last_update = 0

    def __update(self):
        print('Polling server status')
        response = urllib.request.urlopen(self.request)

        soup = BeautifulSoup(response.read())
        totals_table = soup.find('table', {'class': 'serviceTotals'})
        titles = [th.string for th in totals_table.find_all('th')]
        totals = [int(td.string) for td in totals_table.find_all('td')]

        return dict(zip(titles, totals))

    @property
    def status(self):
        now = time.time()
        if self.__last_update < now - POLL_INTERVAL:
            self.__last_update = now
            self.__status = self.__update()
        return self.__status


class StatusHandler(asynchat.async_chat):
    def __init__(self, sock, addr, nagios):
        asynchat.async_chat.__init__(self, sock=sock)
        self.addr = addr
        self.nagios = nagios
        self.struct = struct.Struct('<III')
        self.input_buffer = []
        self.set_terminator(b'\n')

    def collect_incoming_data(self, data):
        self.input_buffer.append(data)

    def found_terminator(self):
        token = b''.join(self.input_buffer).decode('ascii').strip()
        self.input_buffer = []

        if token == CLIENT_TOKEN:
            while True:
                self.send_status();
                time.sleep(10);
        else:
            print('Client token mismatch from {0}'.format(repr(self.addr)))
            print('Got: {0}'.format(repr(token)))
            self.send(b'Unauthorized\n')

    def send_status(self):
        try:
            status = self.nagios.status
            critical = status['Critical']
            warning = status['Warning'] + status['Unknown']
            good = status['Ok'] + status['Pending']
        except Exception as e:
            critical = 1
            warning = 0
            good = 0
        print('Status update: {0} {1} {2}'.format(critical, warning, good))
        self.send(self.struct.pack(critical, warning, good))



class StatusServer(asyncore.dispatcher):
    def __init__(self, host, port, nagios):
        asyncore.dispatcher.__init__(self)
        self.nagios = nagios
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.settimeout(10);
        self.set_reuse_addr()
        self.bind((host, port))
        self.listen(5)

    def handle_accepted(self, sock, addr):
        print('Incoming connection from {0}'.format(repr(addr)))
        handler = StatusHandler(sock, addr, self.nagios)


if __name__ == '__main__':
    nagios = NagiosServer(sys.argv[1], sys.argv[2], sys.argv[3])
    server = StatusServer('0.0.0.0', 8080, nagios)
    asyncore.loop()
