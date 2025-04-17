#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
A simple DebugServer which sends custom debug information to all clients connected.

Each data can be accompanied by a topic, so you can listen from different types of
events as a client.
The topics are limited to 16 characters.

```
import time

debug = DebugServer()
x = 0
while True:
    debug.send(x, 'tick') # Send x in the topic 'tick'
    time.sleep(1)
    debug.send(x, 'tack') # Send x in the topic 'tack'
    time.sleep(1)
	x += 1
```
"""

import zmq

from common import pack

# ==== Lazy imports =====
# import cv2 as cv

class DebugServer:
    def __init__(self, port=5556):
        """
        The whole point.

        port -- Specify port to use for the connection (default 5556)
        """

        host = f"tcp://*:{port}"

        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.PUB)
        self.socket.bind(host)

        print(f"DebugServer listening at {host}")

    def __del__(self):
        print("Close DebugServer...")
        self.socket.close()
        self.context.destroy()

    def send(self, data, topic=""):
        """
        Sends the given data to all clients.
        The data is serialized automatically.

        data -- Data to send
        topic -- Optional topic to specify
        """
        packet = pack(data, topic)
        self.socket.send(packet)

    def send_img(self, img, topic=""):
        """
        Sends the given image.
        This will autoload opencv!

        img -- The image to send
        topic -- Optional topic to specify
        """
        import cv2 as cv

        ret, buffer = cv.imencode(".jpg", img, [int(cv.IMWRITE_JPEG_QUALITY), 20])
        self.send(buffer, topic)
