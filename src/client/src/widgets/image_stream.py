from PyQt4 import QtGui, QtCore

import cv2
import numpy as np
import sys

import struct
from PyQt4.QtCore import QThread
from PyQt4.QtGui import QApplication

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

    def close(self):

        super(ImageStream, self).close()


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

        while True:
            # Obtain information from camera.
            msg = self.com.recv()
            while msg[0] != 'SHOUT':
                msg = self.com.recv()

            data = msg[4]
            offset = 0

            print "received message"

            # Parse data
            while offset < len(data):
                header_size = struct.unpack('@I', data[offset:offset+4])[0]
                offset += 4
                print "header size: "+str(header_size)

                h_id, h_time = struct.unpack('IL', data[offset:(offset + header_size)])
                print "id: "+ str(h_id)
                print "time: "+str(h_time)

                offset += header_size

                data_size = struct.unpack('@I', data[offset:offset+4])[0]
                print "data size: "+str(data_size)
                offset += 4

                img_data = data[offset:offset+data_size]

                offset += data_size

                # Decode openCv
                nparr = np.fromstring(img_data, np.uint8)
                img = cv2.imdecode(nparr, 1)  # cv2.IMREAD_COLOR in OpenCV 3.1

                if img is not None:
                    # Transform to QImage
                    height, width, channel = img.shape
                    bytesPerLine = 3 * width
                    self.img = QtGui.QImage(img.data, width, height, bytesPerLine, QtGui.QImage.Format_RGB888).rgbSwapped()

                    self.emit(self.signal, "image updated")
                else:
                    print "Invalid image received. size: "+str(len(img_data))


#
# app = QApplication(sys.argv)
# iStream = ImageStream()
# iStream.show()
# QtCore.QTimer.singleShot(0, iStream.start)
# sys.exit(app.exec_())


