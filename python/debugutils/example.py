from debugutils import *

import cv2 as cv

def handle_img(img):
    img = cv.imdecode(img, cv.IMREAD_COLOR)
    cv.imshow("img", img)
    cv.waitKey(1)

def handle_ping(text):
    print(f"Got ping {text}")

print("Connecting...")
debug = DebugClient("192.168.123.62")
print("Connected")
debug.addListener("", handle_img)
debug.addListener("ping", handle_ping)
