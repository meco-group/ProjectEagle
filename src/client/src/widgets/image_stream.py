from PyQt4 import QtGui, QtCore

import cv2
import numpy as np

import struct
from PyQt4.QtCore import QThread
from src.comm.communicator import Communicator


class ImageStream(QtGui.QLabel):
    def __init__(self, parent, device):
        super(ImageStream, self).__init__(parent)
        self.device = device
        self.image_receiver = ImageReceiver()
        # self.setScaledContents(True) # to resize image to label dimensions

    def start(self):
        self.connect(self.image_receiver, self.image_receiver.signal, self.update_image)
        self.image_receiver.start()
        QtCore.QTimer.singleShot(0, self.device.start_image_stream)

    def update_image(self):
        self.setPixmap(QtGui.QPixmap.fromImage(self.image_receiver.img))
        return

    def stop(self):
        self.image_receiver.stop()
        self.device.stop_image_stream()


class ImageReceiver(QThread):
    def __init__(self):
        QThread.__init__(self)
        self.img = None
        self.signal = QtCore.SIGNAL("image_update")
        self.running = False
        self.com = None

    def __del__(self):
        self.wait()

    def run(self):
        print "======================="
        print "Starting Image Receiver"
        print "======================="
        self.running = True

        # Setup communicator
        self.com = Communicator("EAGLE")
        self.com.join("EAGLE")
        self.com.start()

        # Wait for peers to connect
        print "Waiting for peers ..."
        self.com.wait_for_peers()
        print "Found peers"

        while self.running:
            (header, img) = self.com.read()

            if img is not None:
                # Transform to QImage
                height, width, channel = img.shape
                bytesPerLine = 3 * width
                self.img = QtGui.QImage(img.data, width, height, bytesPerLine, QtGui.QImage.Format_RGB888).rgbSwapped()

                self.emit(self.signal, "image updated")
            else:
                print "Invalid image received"

        self.com.leave("EAGLE")
        self.com.stop()

    def stop(self):
        if self.running:
            print "Stopping Image Stream"
            print "======================="
            self.running = False