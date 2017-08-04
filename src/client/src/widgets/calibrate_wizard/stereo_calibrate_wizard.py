import os
from PyQt4 import QtGui, QtCore

from io import BytesIO
from xml.dom import minidom

import xml.etree.cElementTree as ET

from config.cal_config import CalibrationConfig
from file_list_view import FileListView
from widgets.image_stream import ImageStream


class StereoCalibrateWizard(QtGui.QDialog):
    IMG_FOLDER = "extrinsic_snaps"
    IMG_CONFIG = "extrinsic_snaps.xml"
    CONFIG = "extrinsic_conf.xml"

    def __init__(self, parent, dev1, dev2):
        super(StereoCalibrateWizard, self).__init__(parent)

        # setup devices
        # the device on the left is ALWAYS already integrated
        if dev1.is_integrated():
            self.left_device = dev1
            self.right_device = dev2
        else:
            self.left_device = dev2
            self.right_device = dev1

        # Setup Snap path
        # TODO no more hardcode of configpath
        self.setup_folder()
        self.snap_count = 0

        self.setWindowTitle('Calibrate Devices')

        self.content = QtGui.QHBoxLayout()

        # Build the left snapper block
        self.left_snapper = QtGui.QWidget(self)
        left_snapper_layout = QtGui.QVBoxLayout()

        self.left_image_stream = ImageStream(self, self.left_device)
        left_snapper_layout.addWidget(self.left_image_stream)

        self.left_snapper.setLayout(left_snapper_layout)
        self.content.addWidget(self.left_snapper)

        # Build the snapper block
        self.right_snapper = QtGui.QWidget(self)
        right_snapper_layout = QtGui.QVBoxLayout()

        self.right_image_stream = ImageStream(self, self.right_device)
        right_snapper_layout.addWidget(self.right_image_stream)

        self.right_snapper.setLayout(right_snapper_layout)
        self.content.addWidget(self.right_snapper)

        # Build the option pane
        self.snap_list = QtGui.QWidget(self)
        snap_list_layout = QtGui.QVBoxLayout()

        # Build the snapshot list
        self.file_list = FileListView(self.left_device.local_path_finder.get_path("[EXTRINSIC_SNAP_FOLDER]"))
        self.snap_count = self.file_list.get_file_count()
        snap_list_layout.addWidget(self.file_list)

        self.snapshot_button = QtGui.QPushButton('Snapshot', self)
        snap_list_layout.addWidget(self.snapshot_button)
        self.snapshot_button.clicked.connect(self.handle_snapshot)

        self.reload_stream_button = QtGui.QPushButton('Refresh Stream', self)
        snap_list_layout.addWidget(self.reload_stream_button)
        self.reload_stream_button.clicked.connect(self.handle_reload)

        self.cal_button = QtGui.QPushButton('Calibrate', self)
        snap_list_layout.addWidget(self.cal_button)
        self.cal_button.clicked.connect(self.handle_calibrate)

        self.cancel_button = QtGui.QPushButton('Cancel', self)
        snap_list_layout.addWidget(self.cancel_button)
        self.cancel_button.clicked.connect(self.handle_cancel)

        self.snap_list.setLayout(snap_list_layout)
        self.content.addWidget(self.snap_list)

        # Build the window
        self.setLayout(self.content)

        self.setModal(True)
        self.show()

        # Start image stream
        QtCore.QTimer.singleShot(0, self.left_image_stream.start)
        QtCore.QTimer.singleShot(0, self.right_image_stream.start)

        # Connect snap finished
        self.connect(self.left_image_stream, self.left_image_stream.signal_snap, self.handle_snapshot_finished)
        self.connect(self.left_image_stream, self.left_image_stream.signal_reload, self.handle_reload_finished)
        self.connect(self.right_image_stream, self.right_image_stream.signal_snap, self.handle_snapshot_finished)
        self.connect(self.right_image_stream, self.right_image_stream.signal_reload, self.handle_reload_finished)

    def setup_folder(self):
        path = self.left_device.local_path_finder.get_path("[EXTRINSIC_SNAP_FOLDER]")
        if not os.path.isdir(path):
            os.mkdir(path)

        path = self.right_device.local_path_finder.get_path("[EXTRINSIC_SNAP_FOLDER]")
        if not os.path.isdir(path):
            os.mkdir(path)

    def get_next_snap_path(self, device):
        return device.local_path_finder.get_path("[EXTRINSIC_SNAP_FOLDER]/image_" + str(self.snap_count) + ".png")

    def closeEvent(self, QCloseEvent):
        self.left_image_stream.finish()
        self.right_image_stream.finish()
        super(StereoCalibrateWizard, self).closeEvent(QCloseEvent)

    def handle_snapshot(self):
        self.snapshot_button.setEnabled(False)
        self.reload_stream_button.setEnabled(False)
        self.cal_button.setEnabled(False)

        # remove all pending mouseclicks
        QtCore.QCoreApplication.instance().processEvents()

        def execute():
            # We have to snap right first
            # because left decides the name of the next image (file list is attached to folder of left)
            self.right_image_stream.snap()
            QtCore.QTimer.singleShot(0, self.left_image_stream.snap)

        QtCore.QTimer.singleShot(0, execute)

    def handle_snapshot_finished(self):
        # remove all pending mouseclicks
        QtCore.QCoreApplication.instance().processEvents()

        # enable button
        self.snapshot_button.setEnabled(True)
        self.reload_stream_button.setEnabled(True)
        self.cal_button.setEnabled(True)

        # update snapshot list
        self.file_list.read_files()
        self.snap_count = self.file_list.get_file_count()

    def handle_reload(self):
        self.snapshot_button.setEnabled(False)
        self.reload_stream_button.setEnabled(False)
        self.cal_button.setEnabled(False)

        # remove all pending mouseclicks
        QtCore.QCoreApplication.instance().processEvents()

        def execute():
            self.left_image_stream.reload()
            QtCore.QTimer.singleShot(0, self.right_image_stream.reload)

        QtCore.QTimer.singleShot(0, execute)

    def handle_reload_finished(self):
        # remove all pending mouseclicks
        QtCore.QCoreApplication.instance().processEvents()

        # enable button
        self.snapshot_button.setEnabled(True)
        self.reload_stream_button.setEnabled(True)
        self.cal_button.setEnabled(True)

    def handle_calibrate(self):
        path = self.left_device.local_path_finder.get_path("[EXTRINSIC_SNAP_FOLDER]")
        conf = CalibrationConfig(
            device=self.left_device,
            images=self.file_list.get_selected(path),
            image_list_path="[EXTRINSIC_SNAP_LIST]",
            output_path="[EXTRINSIC_CALIBRATION]"
        )
        conf.write_config(self.left_device.local_path_finder.get_path("[EXTRINSIC_CONFIG]"))
        conf.write_image_list(self.left_device.local_path_finder.get_path("[EXTRINSIC_SNAP_LIST]"))

        path = self.right_device.local_path_finder.get_path("[EXTRINSIC_SNAP_FOLDER]")
        conf = CalibrationConfig(
            device=self.right_device,
            images=self.file_list.get_selected(path),
            image_list_path="[EXTRINSIC_SNAP_LIST]",
            output_path="[EXTRINSIC_CALIBRATION]"
        )
        conf.write_config(self.right_device.local_path_finder.get_path("[EXTRINSIC_CONFIG]"))
        conf.write_image_list(self.right_device.local_path_finder.get_path("[EXTRINSIC_SNAP_LIST]"))

        self.calibrate()

        self.left_device.update()
        self.right_device.update()

        self.left_image_stream.finish()
        self.right_image_stream.finish()
        # TODO send calibration
        self.accept()

    def calibrate(self):
        from subprocess import call
        command = "../../../build/bin/StereoCalibrate"
        config1 = self.left_device.local_path_finder.get_path("[EXTRINSIC_CONFIG]")
        config2 = self.right_device.local_path_finder.get_path("[EXTRINSIC_CONFIG]")

        # TODO direction of transformation? => swap configs?

        print "Executing: " + command + \
              config1 + " " + \
              config2
        call([command, config1, config2])

    def handle_cancel(self):
        self.left_image_stream.finish()
        self.right_image_stream.finish()
        self.reject()
