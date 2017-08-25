import os
import subprocess
from PyQt4 import QtGui, QtCore
from widgets.image_stream import ImageStream
from widgets.file_list_view import FileListView
import core.paths as paths


class StereoCalibrateWizard(QtGui.QDialog):

    def __init__(self, parent, dev1, dev2):
        super(StereoCalibrateWizard, self).__init__(parent)

        # Setup devices
        # The device on the left is ALWAYS already integrated
        if dev1.is_integrated():
            self.left_device = dev1
            self.right_device = dev2
        else:
            self.left_device = dev2
            self.right_device = dev1

        # Setup Snap path
        self.calibration_dir, self.cal_dir_ldev, self.cal_dir_rdev = paths.get_extrinsic_calibration_dir(self.left_device, self.right_device)
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
        self.file_list = FileListView(self.cal_dir_ldev)
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

    def get_next_snap_path(self, device):
        if self.left_device == device:
            return os.path.join(self.cal_dir_ldev, 'image_%d.png' % self.snap_count)
        else:
            return os.path.join(self.cal_dir_rdev, 'image_%d.png' % self.snap_count)

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
        subprocess.call([os.path.join(paths.LOCAL_BIN_DIR, 'StereoCalibrate'),
            self.left_device.config_path, self.right_device.config_path,
            self.cal_dir_ldev+'/', self.cal_dir_rdev+'/'])
        self.left_device.upload()
        self.right_device.upload()
        self.left_device.update()
        self.right_device.update()

    def handle_cancel(self):
        self.left_image_stream.finish()
        self.right_image_stream.finish()
        self.reject()
