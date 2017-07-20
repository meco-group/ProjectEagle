import os
from PyQt4 import QtGui, QtCore

from io import BytesIO
from xml.dom import minidom

import xml.etree.cElementTree as ET

from src.widgets.calibrate_wizard.file_list_view import FileListView
from src.widgets.image_stream import ImageStream


class CalibrateWizard(QtGui.QDialog):
    taking_snap = False

    def __init__(self, parent, device):
        super(CalibrateWizard, self).__init__(parent)

        # Setup devices
        self.device = device

        # Setup Snap path
        # TODO no more hardcode of configpath
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
        self.file_list = FileListView(self.device.get_snap_path())
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
        if not os.path.isdir(self.device.get_snap_path()):
            os.mkdir(self.device.get_snap_path())

    def closeEvent(self, QCloseEvent):
        self.image_stream.stop()
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

    def get_next_snap_path(self):
        return self.device.get_snap_path() + "image_"+str(self.snap_count)+".png"

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
        self.generate_image_list()
        self.generate_cal_config()
        self.calibrate()
        self.image_stream.stop()
        self.accept()

    def generate_image_list(self):
        paths =  self.file_list.get_selected()
        res = '\n'
        for path in paths:
            res = res + '\t' + path + '\n'

        root = ET.Element("opencv_storage")
        ET.SubElement(root, "images").text = res

        tree = ET.ElementTree(root)

        f = BytesIO()
        tree.write(f, encoding='utf-8', xml_declaration=True)

        result = minidom.parseString(f.getvalue())
        result = result.toprettyxml(indent="\t")
        with open(self.device.get_snap_conf_path(), "w") as text_file:
            text_file.write(result)

    def generate_cal_config(self):
        image_count = len(self.file_list.get_selected())


        root = ET.Element('opencv_storage')
        cal = ET.Element('CalibrationSettings')
        ic = ET.SubElement(cal, 'Image_Count')
        ic.text = str(image_count)
        src = ET.SubElement(cal, 'Source')
        src.text = self.device.get_snap_conf_path()
        src_type = ET.SubElement(cal, 'Source_Type')
        src_type.text = "STORED"
        brds = ET.SubElement(cal, 'Board_Settings')
        brds.text = self.device.get_conf_path()
        cfar = ET.SubElement(cal, 'Calibrate_FixAspectRatio')
        cfar.text = str(1.0)
        caztd = ET.SubElement(cal, 'Calibrate_AssumeZeroTangentialDistortion')
        caztd.text = '0'
        cfppatc = ET.SubElement(cal, 'Calibrate_FixPrincipalPointAtTheCenter')
        cfppatc.text = '0'
        of = ET.SubElement(cal, 'Output_File')
        of.text = self.device.get_cal_path()

        root.append(cal)
        tree = ET.ElementTree(root)

        f = BytesIO()
        tree.write(f, encoding='utf-8', xml_declaration=True)

        result = minidom.parseString(f.getvalue())
        result = result.toprettyxml(indent="\t")
        with open(self.device.get_cal_conf_path(), "w") as text_file:
            text_file.write(result)

    def calibrate(self):
        from subprocess import call
        print "Executing: "+"../../../build/bin/Calibrate "+self.device.get_cal_conf_path()
        call(["../../../build/bin/Calibrate", self.device.get_cal_conf_path()])

    def handle_cancel(self):
        self.image_stream.stop()
        self.reject()