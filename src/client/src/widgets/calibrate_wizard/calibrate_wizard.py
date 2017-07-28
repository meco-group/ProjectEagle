import os
from PyQt4 import QtGui, QtCore

from io import BytesIO
from xml.dom import minidom

import xml.etree.cElementTree as ET

from src.config.cal_config import CalibrationConfig
from src.widgets.calibrate_wizard.file_list_view import FileListView
from src.widgets.image_stream import ImageStream


class CalibrateWizard(QtGui.QDialog):
    IMG_FOLDER = "intrinsic_snaps"
    IMG_CONFIG = "intrinsic_snaps.xml"
    CONFIG = "intrinsic_conf.xml"

    def __init__(self, parent, device):
        super(CalibrateWizard, self).__init__(parent)

        # Setup devices
        self.device = device

        # Setup Snap path
        self.setup_folder()
        self.snap_count = 0

        self.setWindowTitle('Calibrate Device')

        self.content = QtGui.QHBoxLayout()

        # Build the snapper block
        self.snapper = QtGui.QWidget(self)
        snapper_layout = QtGui.QVBoxLayout()

        self.image_stream = ImageStream(self, device)
        snapper_layout.addWidget(self.image_stream)

        self.snapper.setLayout(snapper_layout)
        self.content.addWidget(self.snapper)

        # Build the snapper block
        self.snap_list = QtGui.QWidget(self)
        snap_list_layout = QtGui.QVBoxLayout()

        # Build the snapshot list
        self.file_list = FileListView(device.local_path_finder.get_path("[INTRINSIC_SNAP_FOLDER]"))
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
        QtCore.QTimer.singleShot(0, self.image_stream.start)

        # Connect snap finished
        self.connect(self.image_stream, self.image_stream.signal_snap, self.handle_snapshot_finished)
        self.connect(self.image_stream, self.image_stream.signal_reload, self.handle_reload_finished)

    def setup_folder(self):
        path = self.device.local_path_finder.get_path("[INTRINSIC_SNAP_FOLDER]")
        if not os.path.isdir(path):
            os.mkdir(path)

    def get_next_snap_path(self, device):
        return device.local_path_finder.get_path("[INTRINSIC_SNAP_FOLDER]/image_" + str(self.snap_count) + ".png")

    def closeEvent(self, QCloseEvent):
        self.image_stream.finish()
        super(CalibrateWizard, self).closeEvent(QCloseEvent)

    def handle_snapshot(self):
        self.snapshot_button.setEnabled(False)
        self.reload_stream_button.setEnabled(False)
        self.cal_button.setEnabled(False)

        # remove all pending mouseclicks
        QtCore.QCoreApplication.instance().processEvents()

        QtCore.QTimer.singleShot(0, self.image_stream.snap)

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

        QtCore.QTimer.singleShot(0, self.image_stream.reload)

    def handle_reload_finished(self):
        # remove all pending mouseclicks
        QtCore.QCoreApplication.instance().processEvents()

        # enable button
        self.snapshot_button.setEnabled(True)
        self.reload_stream_button.setEnabled(True)
        self.cal_button.setEnabled(True)

    def handle_calibrate(self):
        conf = CalibrationConfig(
            device=self.device,
            images=self.file_list.get_selected(),
        )
        conf.write_config(self.device.local_path_finder.get_path("[INTRINSIC_CONFIG]"))
        conf.write_image_list(self.device.local_path_finder.get_path("[INTRINSIC_SNAP_LIST]"))

        self.calibrate()
        self.device.update()
        # TODO send calibration
        self.image_stream.finish()
        self.accept()

    def calibrate(self):
        target = self.device.local_path_finder.get_path("[INTRINSIC_CONFIG]")
        from subprocess import call
        print "Executing: "+"../../../build/bin/Calibrate "+target
        call(["../../../build/bin/Calibrate", target])
        print "Sending over calibration file"

    def handle_cancel(self):
        self.image_stream.finish()
        self.reject()