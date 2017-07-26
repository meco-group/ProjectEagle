import os
from PyQt4 import QtGui, QtCore

import time
from PyQt4.QtCore import QThread, QObject
from pathlib import Path

from src.comm.communicator import Communicator

from xml.etree import ElementTree as ET


class ImageStream(QtGui.QLabel):
    def __init__(self, cal_wizard, device):
        super(ImageStream, self).__init__()
        self.cal_wizard = cal_wizard
        self.signal_snap = QtCore.SIGNAL("snapshot_taken")
        self.signal_reload = QtCore.SIGNAL("reload")
        self.device = device

        # The group stored in the config of the device has to be unique to allow for
        # stereo calibration!
        # TODO verify this in code
        self.group = "EAGLE"
        if os.path.isfile(device.get_config_path()):
            tree = ET.ElementTree(file=device.get_config_path())
            for elem in tree.iterfind('CommunicatorSettings/Group'):
                self.group = elem.text

        # self.setScaledContents(True) # to resize image to label dimensions
        self.receiver_thread = QThread()
        self.receiver_thread.start()

        self.image_receiver = ImageReceiver(self.group)
        self.image_receiver.moveToThread(self.receiver_thread)
        self.connect(self.image_receiver, self.image_receiver.signal, self.update_image)

        self.transmitter_thread = QThread()
        self.transmitter_thread.start()

        self.image_transmitter = ImageTransmitter(self.device)
        self.image_transmitter.moveToThread(self.transmitter_thread)
        self.connect(self.image_transmitter, self.image_transmitter.signal, self.reload)

    def finish(self):
        self.image_receiver.stop()
        self.image_transmitter.stop()
        self.receiver_thread.quit()
        self.receiver_thread.wait()
        self.transmitter_thread.quit()
        self.transmitter_thread.wait()

    def start(self):
        QtCore.QTimer.singleShot(0, self.image_transmitter.task)
        QtCore.QTimer.singleShot(0, self.image_receiver.task)

    def update_image(self):
        self.setPixmap(QtGui.QPixmap.fromImage(self.image_receiver.img))
        return

    def stop(self):
        self.image_transmitter.stop()
        self.image_receiver.stop()

    def snap(self):
        # Stop ImageTransmitter/Receiver
        self.stop()

        # Take the snapshot
        target_path = self.cal_wizard.get_next_snap_path(self.device)
        print "Taking snapshot "+target_path

        command = "/home/pi/ProjectEagle/build/bin/Snapshot"

        arg1 = str(Path(self.device.get_config_path()).relative_to(os.path.abspath("../../..")))
        arg1 = os.path.join("/home/pi/ProjectEagle", arg1)

        arg2 = "/home/pi/ProjectEagle/" + self.device.name + "_snapshot.png"

        self.device.ssh_manager.start_process("snapshot", command, arg1, arg2)
        self.device.ssh_manager.wait_for_process("snapshot")

        print "Downloading snapshot"

        ssh = self.device.ssh_manager.get_ssh()
        sftp = ssh.open_sftp()
        sftp.get(arg2, target_path)
        sftp.remove(arg2)
        ssh.close()

        print "Done"

        # Start ImageTransmitter again
        self.start()

        self.emit(self.signal_snap, "snapshot taken")

    def reload(self):
        # Stop ImageTransmitter/Receiver
        self.stop()

        # Start ImageTransmitter again
        self.start()

        self.emit(self.signal_reload, "reload")


class ImageTransmitter(QObject):
    def __init__(self, device):
        super(ImageTransmitter, self).__init__()
        self.device = device

        self._isRunning = False
        self._isDead = False
        self.signal = QtCore.SIGNAL("hanging")

    def task(self):
        if not self._isRunning:
            print "=========================="
            print "Starting Image Transmitter"
            print "=========================="

            # Start image transmitter
            command = "/home/pi/ProjectEagle/build/bin/ImageTransmitter"

            target = str(Path(self.device.get_config_path()).relative_to(os.path.abspath("../../..")))
            target = os.path.join("/home/pi/ProjectEagle", target)

            self.device.ssh_manager.start_process("image_transmitter", command, target)

            self._isRunning = True
            self._isDead = False

        while self._isRunning and not self._isDead:
            self.device.ssh_manager.wait_for_process("image_transmitter")
            if self._isRunning:
                print "Lost transmitter process"
                # self.emit(self.signal, "transmitter lost")
                self._isDead = True

        while self._isRunning:
            time.sleep(.01)

        self.device.ssh_manager.end_process("image_transmitter")

    def stop(self):
        print "Stopping Image Transmitter"
        print "======================="
        self._isRunning = False
        self.device.ssh_manager.end_process("image_transmitter")


class ImageReceiver(QObject):
    def __init__(self, group="EAGLE"):
        super(ImageReceiver, self).__init__()
        self.group = group
        self.img = None
        self.com = None

        self._isRunning = False
        self._isDead = False

        self.signal = QtCore.SIGNAL("image_update")

    def task(self):
        if not self._isRunning:
            print "======================="
            print "Starting Image Receiver"
            print "======================="

            print "Group: "+self.group

            # Setup communicator
            self.com = Communicator("receiver")
            self.com.join(self.group)
            self.com.start()

            # Wait for peers to connect
            print "Waiting for peers ..."
            self.com.wait_for_peers()
            print "Found peers"

            self._isRunning = True

        while self._isRunning:
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
        print "Stopping Image Receiver"
        print "======================="
        self._isRunning = False