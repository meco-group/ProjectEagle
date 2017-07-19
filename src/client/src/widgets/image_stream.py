from PyQt4 import QtGui, QtCore

import cv2
import numpy as np
import sys
from PyQt4.QtCore import QThread
from PyQt4.QtGui import QApplication

from src.comm.communicator import Communicator


class ImageStream(QtGui.QLabel):
    def __init__(self, parent=None):
        super(ImageStream, self).__init__(parent)
        self.image_receiver = ImageReceiver()
        # self.setScaledContents(True) # to resize image to label dimensions

    def start(self):
        self.connect(self.image_receiver, self.image_receiver.signal, self.update_image)
        self.image_receiver.start()

    def update_image(self):
        self.setPixmap(QtGui.QPixmap.fromImage(self.image_receiver.img))
        return


class ImageReceiver(QThread):
    def __init__(self):
        QThread.__init__(self)
        self.img = None
        self.signal = QtCore.SIGNAL("image_update")

    def __del__(self):
        self.wait()

    def run(self):
        # Setup communicator
        self.com = Communicator("EAGLE")
        self.com.join("EAGLE")
        self.com.start()

        # Wait for peers to connect
        print "Waiting for peers ..."
        self.com.wait_for_peers()
        print "Found peers"

        # Drop connection frames
        self.com.read()
        self.com.read()

        while True:
            # Get message from Zyre
            msg = self.com.read()

            # Decode openCv
            nparr = np.fromstring(msg[-1][24:], np.uint8)
            img = cv2.imdecode(nparr, 1)  # cv2.IMREAD_COLOR in OpenCV 3.1

            # Transform to QImage
            height, width, channel = img.shape
            bytesPerLine = 3 * width
            self.img = QtGui.QImage(img.data, width, height, bytesPerLine, QtGui.QImage.Format_RGB888).rgbSwapped()

            self.emit(self.signal, "image updated")


#
# app = QApplication(sys.argv)
# iStream = ImageStream()
# iStream.show()
# QtCore.QTimer.singleShot(0, iStream.start)
# sys.exit(app.exec_())


