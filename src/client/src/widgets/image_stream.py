import os
from PyQt4 import QtGui, QtCore

from PyQt4.QtCore import QThread
from src.comm.communicator import Communicator

from xml.etree import ElementTree as ET


class ImageStream(QtGui.QLabel):
    def __init__(self, cal_wizard, device):
        super(ImageStream, self).__init__()
        self.cal_wizard = cal_wizard
        self.signal_snap = QtCore.SIGNAL("snapshot_taken")
        self.signal_reload = QtCore.SIGNAL("reload")
        self.device = device

        group = "EAGLE"
        if os.path.isfile(device.get_conf_path()):
            tree = ET.ElementTree(file=device.get_conf_path())
            for elem in tree.iterfind('CommunicatorSettings/Group'):
                group = elem.text


        self.image_receiver = ImageReceiver(group)
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

    def snap(self):
        def execute():
            # Stop ImageTransmitter/Receiver
            self.image_receiver.stop()
            self.device.stop_image_stream()

            # Take the snapshot
            self.device.take_snapshot(self.cal_wizard.get_next_snap_path())

            # Start ImageTransmitter again
            QtCore.QTimer.singleShot(0, self.device.start_image_stream)
            self.image_receiver.start()

            self.emit(self.signal_snap, "snapshot taken")

        QtCore.QTimer.singleShot(0, execute)

    def reload(self):
        def execute():
            # Stop ImageTransmitter/Receiver
            self.image_receiver.stop()
            self.device.stop_image_stream()

            # Start ImageTransmitter again
            QtCore.QTimer.singleShot(0, self.device.start_image_stream)
            self.image_receiver.start()

            self.emit(self.signal_reload, "reload")

        QtCore.QTimer.singleShot(0, execute)


class ImageReceiver(QThread):
    def __init__(self, group="EAGLE"):
        QThread.__init__(self)
        self.img = None
        self.group = group
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

        print "Group: "+self.group

        # Setup communicator
        self.com = Communicator(self.group)
        self.com.join(self.group)
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

        self.com.leave(self.group)
        self.com.stop()

    def stop(self):
        if self.running:
            print "Stopping Image Stream"
            print "======================="
            self.running = False