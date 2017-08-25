import os
from PyQt4 import QtGui, QtCore
from PyQt4.QtCore import QThread, QObject
import time
from xml.etree import ElementTree as ET
import core.paths as paths
from core.communicator import Communicator


class ImageStream(QtGui.QLabel):
    def __init__(self, cal_wizard, device):
        super(ImageStream, self).__init__()
        self.cal_wizard = cal_wizard
        self.signal_snap = QtCore.SIGNAL("snapshot_taken")
        self.signal_reload = QtCore.SIGNAL("reload")
        self.device = device

        tree = ET.parse(device.local_config_path)
        self.group = tree.find('communicator').find('group').text

        self.receiver_thread = QThread()
        self.receiver_thread.start()

        self.com = Communicator('receiver')
        self.com.join(self.group)

        self.image_receiver = ImageReceiver(self.device, self.group, self.com)
        self.image_receiver.moveToThread(self.receiver_thread)
        self.connect(self.image_receiver, self.image_receiver.signal, self.update_image)

        self.transmitter_thread = QThread()
        self.transmitter_thread.start()

        self.image_transmitter = ImageTransmitter(self.device)
        self.image_transmitter.moveToThread(self.transmitter_thread)
        self.connect(self.image_transmitter, self.image_transmitter.signal, self.reload)

    def finish(self):
        self.stop()
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
        # note: The image receiver has to be stopped first
        #       otherwise zyre will keep waiting for a message from the
        #       transmitter indefinitely
        self.image_receiver.stop()
        self.image_transmitter.stop()

    def snap(self):
        # give command to take snapshot
        self.image_receiver.snap()
        # download snapshot
        ssh = self.device.ssh_manager.get_ssh()
        sftp = ssh.open_sftp()
        sftp.get(os.path.join(REMOTE_CONFIG_DIR, 'snapshot.png'), self.cal_wizard.get_next_snap_path(self.device))
        sftp.remove(os.path.join(REMOTE_CONFIG_DIR, 'snapshot.png'))
        ssh.close()
        self.emit(self.signal_snap, 'snapshot taken')

    def reload(self):
        # Stop ImageTransmitter/Receiver
        self.stop()
        # Start ImageTransmitter again
        self.start()
        self.emit(self.signal_reload, 'reload')


class ImageTransmitter(QObject):
    def __init__(self, device):
        super(ImageTransmitter, self).__init__()
        self.device = device

        self._isRunning = False
        self._isDead = False
        self.signal = QtCore.SIGNAL('hanging')

    def task(self):
        if not self._isRunning:
            # start image transmitter
            cmd = os.path.join(paths.get_remote_bin_dir(self.device), 'ImageTransmitter')
            if (not self.device.ssh_manager.start_process('image_transmitter', cmd, self.device.name, '0', '1', '1', os.path.join(paths.get_remote_config_dir(self.device), 'snapshot.png'))):
                print 'Image transmission could not be started on ' + self.device.name + '.'
                self.device.set_online(False)
                return
            self.device.set_online(True)
            self._isRunning = True

    def stop(self):
        self._isRunning = False
        self.device.ssh_manager.end_process('image_transmitter')
        print 'Stopped image transmission.'


class ImageReceiver(QObject):
    def __init__(self, device, group, com):
        super(ImageReceiver, self).__init__()
        self.device = device
        self.group = group
        self.img = None
        self.com = com

        self._isRunning = False
        self._isDead = False
        self._snapshot = False

        self.signal = QtCore.SIGNAL('image_update')

    def task(self):
        if not self._isRunning:
            self._isRunning = True

            # start communicator
            self.com.start()

            # Wait for peers to connect
            if (not self.com.wait_for_peer(self.device.name, 2)):
                print 'Peer ' + self.device.name + ' not available.',
                self.com.leave(self.group)
                self.com.stop()
                self._isDead = True
                self.stop()
                return

        print 'Started image receiving from %s.' % self.device.name
        while self._isRunning:
            img, pr = self.com.rcv_img()
            if pr == self.device.name:
                if img is not None:
                    # Transform to QImage
                    height, width, channel = img.shape
                    bytesPerLine = 3 * width
                    self.img = QtGui.QImage(img.data, width, height, bytesPerLine, QtGui.QImage.Format_RGB888).rgbSwapped()
                    self.emit(self.signal, 'image updated')
                else:
                    print 'Invalid image received from %s!' % self.device.name
            if self._snapshot:
                self.com.send_cmd(0, self.device.name)
                self._snapshot = False

        self.com.leave(self.group)
        self.com.stop()
        self._isDead = True

    def stop(self):
        self._isRunning = False
        while not self._isDead:
            time.sleep(.01)
        print 'Stopped image receiving from %s.' % self.device.name

    def snap(self):
        self._snapshot = True
