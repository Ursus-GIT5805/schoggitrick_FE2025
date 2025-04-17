#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
A simple DebugClient, used for the DebugServer

It'll receive data from the server and run handlers for the given topics.

```
def tick(x):
    print(f"Server on {x} ticks so far!")

def tack(x):
    print(f"Server on {x} tacks so far!")

client = DebugClient("localhost")
client.addListener('tick', tick)
client.addListener('tack', tack)
# Will loop indefinitely
```
"""

import argparse as argp
import threading

import zmq

from common import string_to_topic, unpack

class DebugClient:
    def __init__(self, host, port=5556):
        """
        The client for the DebugServer.
        The constructor waits until the client is connected.

        host -- Specify the address of the server (string)
        port -- Specify port to use
        """

        uri = f"tcp://{host}:{port}"

        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.SUB)
        self.socket.setsockopt_string(zmq.SUBSCRIBE, "")

        self.socket.connect(uri)
        self.listeners = {}

        poller = zmq.Poller()
        poller.register(self.socket, zmq.POLLIN)

        # Wait until the socket is connected
        while True:
            socks = dict(poller.poll())
            if self.socket in socks and socks[self.socket] == zmq.POLLIN:
                break

            time.sleep(1)

        # Start listener thread
        self.thread = threading.Thread(target=self.listen)
        self.thread.start()

    def __del__(self):
        self.socket.close()
        self.context.destroy()

    def addListener(self, topic, callback):
        """
        Adds a listener for the given topic.
        Overwrites the current listener if there is one.

        callback -- Callback with one argument, which will be called upon a received packet with 'topic'
        """

        head = string_to_topic(topic)
        self.listeners[head] = callback

    def listen(self):
        """
        Listens to the incoming packets and run listeners.
        This function runs indefinitely until an error occurs.
        """

        while True:
            try:
                data = self.socket.recv()
                topic, data = unpack(data)
            except:
                break

            try:
                self.listeners[topic](data)
            except KeyError:
                pass

if __name__ == "__main__":
    parser = argp.ArgumentParser()
    parser.add_argument('--host', type=str, default="localhost")
    parser.add_argument('-p', '--port', type=int, default=5556)
    args = parser.parse_args()

    print("Connecting...")
    debug = DebugClient(args.host, args.port)
    print("Connected!")

    def handler(msg):
        print(f"Received message: {msg}")

    debug.addListener("", handler)
