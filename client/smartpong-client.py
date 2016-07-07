#!/usr/bin/env python2.7
import thread
import time
import websocket


SOCKET_URL = 'ws://smartpong.rznc.net/display'
SOCKET_URL = 'ws://localhost:9000/ws/display'


def on_message(ws, message):
    print 'Message:', message


def on_error(ws, error):
    print error


def on_close(ws):
    print "### closed ###"


def on_open(ws):
    def run(*args):
        for i in range(3):
            time.sleep(1)
            ws.send("Hello %d" % i)
        time.sleep(1)
        ws.close()
        print "thread terminating..."
    thread.start_new_thread(run, ())


if __name__ == "__main__":
    websocket.enableTrace(True)
    ws = websocket.WebSocketApp(SOCKET_URL,
                                on_message=on_message,
                                on_error=on_error,
                                on_close=on_close)
    ws.on_open = on_open
    ws.run_forever()
