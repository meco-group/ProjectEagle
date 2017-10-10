import os
import subprocess
from PyQt4 import QtGui, QtCore
from file_list_view import FileListView
from widgets.image_stream import ImageStream
import core.paths as paths


class CalibrateWizard(QtGui.QDialog):

    def __init__(self, parent, device):
        super(CalibrateWizard, self).__init__(parent)

        # Setup devices
        self.device = device

        # Setup Snap path
        self.calibration_dir = paths.get_intrinsic_calibration_dir(self.device)
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
        self.file_list = FileListView(self.calibration_dir)
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

        self.setModal(False)
        self.show()

        # Start image stream
        QtCore.QTimer.singleShot(0, self.image_stream.start)

        # Connect snap finished
        self.connect(self.image_stream, self.image_stream.signal_snap, self.handle_snapshot_finished)
        self.connect(self.image_stream, self.image_stream.signal_reload, self.handle_reload_finished)

    def get_next_snap_path(self, device):
        return os.path.join(self.calibration_dir, 'image_%d.png' % self.snap_count)

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
        subprocess.call([os.path.join(paths.LOCAL_BIN_DIR, 'Calibrate'), self.device.local_config_path, self.calibration_dir])
        subprocess.call([os.path.join(paths.LOCAL_BIN_DIR, 'GroundCalibrate'), self.device.local_config_path, self.calibration_dir])
        self.device.upload()
        self.device.update()
        # self.image_stream.finish()
        # self.accept()

    def handle_cancel(self):
        self.image_stream.finish()
        self.reject()
