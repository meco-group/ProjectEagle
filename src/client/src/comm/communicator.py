import time

from PyQt4.QtGui import QImage, QApplication, QPixmap, QLabel
from pyre import Pyre
from pyre import zhelper
import zmq
import uuid
import logging
import sys
import json
import numpy as np
import cv2

class Communicator(Pyre):
    def __init__(self, group):
        super(Communicator, self).__init__(group)
        self.poller = zmq.Poller()
        self.poller.register(self.socket(), zmq.POLLIN)

    def wait_for_peers(self):
        while len(self.peers()) <= 0:
            time.sleep(1)

    def read(self):
        return self.recv()

# com = Communicator("EAGLE")
# com.join("EAGLE")
# com.start()
# print "Waiting for peers ..."
# com.wait_for_peers()
# print "Found peers"
# com.read()
# com.read()
# while True:
#     buffer = com.read()
#     #image res 640, 480
#
#     nparr = np.fromstring(buffer[-1][24:], np.uint8)
#     img = cv2.imdecode(nparr, 1)  # cv2.IMREAD_COLOR in OpenCV 3.1
#     # check types
#     height, width, channel = img.shape
#     bytesPerLine = 3 * width
#     qImg = QImage(img.data, width, height, bytesPerLine, QImage.Format_RGB888)
#
#     print type(qImg)
#     break
#
# app = QApplication(sys.argv)
# label = QLabel()
# label.setPixmap(QPixmap.fromImage(qImg))
# label.show()
# sys.exit(app.exec_())
