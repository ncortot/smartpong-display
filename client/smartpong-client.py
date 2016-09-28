#!/usr/bin/env python2.7
import json
import websocket


SOCKET_URL = 'ws://smartpong.rznc.net/ws'


def display(out):
    f = open('/dev/ttyAMA0', 'w')
    f.write(out + '\n')
    f.close()


def on_message(ws, message):
    data = json.loads(message)
    p1 = str(data['p1']).rjust(2)[-2:]
    p2 = str(data['p2']).ljust(2)[-2:]
    s1 = str(data['s1'])[0]
    s2 = str(data['s2'])[0]
    service = str(data['service'])[0]
    display(' '.join((s1, p1, p2, s2, service)))


def on_error(ws, error):
    display('E rr or ! 0')
    print 'Error:', error


def on_close(ws):
    display('- -- -- - 0')
    print 'Closed'


if __name__ == "__main__":
    websocket.enableTrace(True)
    ws = websocket.WebSocketApp(SOCKET_URL,
                                on_message=on_message,
                                on_error=on_error,
                                on_close=on_close)
    ws.run_forever()
