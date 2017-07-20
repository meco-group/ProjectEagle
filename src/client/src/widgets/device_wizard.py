from PyQt4 import QtGui

import sys
from xml.etree import ElementTree as ET
from xml.dom import minidom

from io import BytesIO


def prettify(rough_string):
    """
    Return a pretty-printed XML string for the Element.
    """

class DeviceWizard(QtGui.QDialog):
    class GeneralTab(QtGui.QWidget):
        def __init__(self, parent=None):
            super(DeviceWizard.GeneralTab, self).__init__(parent)

            box_layout = QtGui.QVBoxLayout()

            self.grid = QtGui.QWidget(self)
            grid_layout = QtGui.QGridLayout()

            self.name_label = QtGui.QLabel(self)
            self.ip_label = QtGui.QLabel(self)
            self.user_label = QtGui.QLabel(self)
            self.pass_label = QtGui.QLabel(self)

            self.name_label.setText("Name:")
            self.ip_label.setText("Ip:")
            self.user_label.setText("Username:")
            self.pass_label.setText("Password:")

            self.name_field = QtGui.QLineEdit(self)
            self.ip_field = QtGui.QLineEdit(self)
            self.user_field = QtGui.QLineEdit(self)
            self.pass_field = QtGui.QLineEdit(self)
            self.pass_field.setEchoMode(QtGui.QLineEdit.Password)

            grid_layout.addWidget(self.name_label, 0, 0)
            grid_layout.addWidget(self.ip_label, 1, 0)
            grid_layout.addWidget(self.user_label, 2, 0)
            grid_layout.addWidget(self.pass_label, 3, 0)
            grid_layout.addWidget(self.name_field, 0, 1)
            grid_layout.addWidget(self.ip_field, 1, 1)
            grid_layout.addWidget(self.user_field, 2, 1)
            grid_layout.addWidget(self.pass_field, 3, 1)

            grid_layout.setColumnStretch(1, 10)
            grid_layout.setMargin(0)

            self.grid.setLayout(grid_layout)
            box_layout.addWidget(self.grid)

            verticalSpacer = QtGui.QSpacerItem(
                20, 40, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
            box_layout.addItem(verticalSpacer)

            self.setLayout(box_layout)

        def load_defaults(self, device):
            self.name_field.setText(device.name)
            self.ip_field.setText(device.ip)
            self.user_field.setText(device.username)
            self.pass_field.setText(device.password)

    class CameraTab(QtGui.QWidget):
        def __init__(self, parent=None):
            super(DeviceWizard.CameraTab, self).__init__(parent)

            box_layout = QtGui.QVBoxLayout()

            self.grid = QtGui.QWidget(self)
            grid_layout = QtGui.QGridLayout()

            self.index_label = QtGui.QLabel(self)
            self.type_label = QtGui.QLabel(self)

            self.index_label.setText("Index:")
            self.type_label.setText("Type:")

            self.index_field = QtGui.QLineEdit(self)
            self.index_field.setValidator(QtGui.QIntValidator(0, 1e10, self))

            self.type_field = QtGui.QComboBox(self)
            self.type_field.addItem("PICAM")
            self.type_field.addItem("OPICAM")
            self.type_field.addItem("SEE3CAM")
            self.type_field.addItem("LATCAM")
            self.type_field.addItem("OCAM")

            grid_layout.addWidget(self.index_label, 0, 0)
            grid_layout.addWidget(self.type_label, 1, 0)
            grid_layout.addWidget(self.index_field, 0, 1)
            grid_layout.addWidget(self.type_field, 1, 1)

            grid_layout.setColumnStretch(1, 10)
            grid_layout.setMargin(0)

            self.grid.setLayout(grid_layout)
            box_layout.addWidget(self.grid)

            verticalSpacer = QtGui.QSpacerItem(
                20, 40, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
            box_layout.addItem(verticalSpacer)

            self.setLayout(box_layout)

        def load_defaults(self, tree):
            for elem in tree.iterfind('CameraSettings/Camera_Index'):
                self.index_field.setText(elem.text)
            for elem in tree.iterfind('CameraSettings/Camera_Type'):
                index = self.type_field.findText(elem.text)
                self.type_field.setCurrentIndex(index)

        def get_xml(self, device):
            root = ET.Element('CameraSettings')
            cam_index = ET.SubElement(root, 'Camera_Index')
            cam_index.text = str(self.index_field.text())
            cam_type = ET.SubElement(root, 'Camera_Type')
            cam_type.text = str(self.type_field.currentText())
            cam_cal = ET.SubElement(root, 'Camera_Calibration')
            cam_cal.text = device.get_cal_path()

            return root

    class ComTab(QtGui.QWidget):
        def __init__(self, parent=None):
            super(DeviceWizard.ComTab, self).__init__(parent)

            box_layout = QtGui.QVBoxLayout()

            self.grid = QtGui.QWidget(self)
            grid_layout = QtGui.QGridLayout()

            self.interface_label = QtGui.QLabel(self)
            self.wait_time_label = QtGui.QLabel(self)

            self.interface_label.setText("Interface:")
            self.wait_time_label.setText("Zyre Wait Time:")

            self.interface_field = QtGui.QLineEdit(self)
            self.wait_time_field = QtGui.QLineEdit(self)
            self.wait_time_field.setValidator(QtGui.QIntValidator(0, 1e10, self))

            grid_layout.addWidget(self.interface_label, 0, 0)
            grid_layout.addWidget(self.wait_time_label, 1, 0)
            grid_layout.addWidget(self.interface_field, 0, 1)
            grid_layout.addWidget(self.wait_time_field, 1, 1)

            grid_layout.setColumnStretch(1, 10)
            grid_layout.setMargin(0)

            self.grid.setLayout(grid_layout)
            box_layout.addWidget(self.grid)

            verticalSpacer = QtGui.QSpacerItem(
                20, 40, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
            box_layout.addItem(verticalSpacer)

            self.setLayout(box_layout)

        def load_defaults(self, tree):
            for elem in tree.iterfind('CommunicatorSettings/Interface'):
                self.interface_field.setText(elem.text)
            for elem in tree.iterfind('CommunicatorSettings/Init_Wait_Time'):
                self.wait_time_field.setText(elem.text)

        def get_xml(self, device):
            root = ET.Element('CommunicatorSettings')
            interface = ET.SubElement(root, 'Interface')
            interface.text = str(self.interface_field.text())
            type = ET.SubElement(root, 'Init_Wait_Time')
            type.text = str(self.wait_time_field.text())

            return root

    class BoardTab(QtGui.QWidget):
        def __init__(self, parent=None):
            super(DeviceWizard.BoardTab, self).__init__(parent)

            box_layout = QtGui.QVBoxLayout()

            self.grid = QtGui.QWidget(self)
            grid_layout = QtGui.QGridLayout()

            self.width_label = QtGui.QLabel(self)
            self.height_label = QtGui.QLabel(self)
            self.square_size_label = QtGui.QLabel(self)

            self.width_label.setText("Board Width: ")
            self.height_label.setText("Board Height")
            self.square_size_label.setText("Square Size: ")

            self.width_field = QtGui.QLineEdit(self)
            self.width_field.setValidator(QtGui.QIntValidator(0, 1e10, self))
            self.height_field = QtGui.QLineEdit(self)
            self.height_field.setValidator(QtGui.QIntValidator(0, 1e10, self))
            self.square_size_field = QtGui.QLineEdit(self)
            self.square_size_field.setValidator(QtGui.QDoubleValidator(0, sys.float_info.max, 3, self))

            grid_layout.addWidget(self.width_label, 0, 0)
            grid_layout.addWidget(self.height_label, 1, 0)
            grid_layout.addWidget(self.square_size_label, 2, 0)
            grid_layout.addWidget(self.width_field, 0, 1)
            grid_layout.addWidget(self.height_field, 1, 1)
            grid_layout.addWidget(self.square_size_field, 2, 1)

            grid_layout.setColumnStretch(1, 10)
            grid_layout.setMargin(0)

            self.grid.setLayout(grid_layout)
            box_layout.addWidget(self.grid)

            verticalSpacer = QtGui.QSpacerItem(
                20, 40, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
            box_layout.addItem(verticalSpacer)

            self.setLayout(box_layout)

        def load_defaults(self, tree):
            for elem in tree.iterfind('BoardSettings/BoardSize_Width'):
                self.width_field.setText(elem.text)
            for elem in tree.iterfind('BoardSettings/BoardSize_Height'):
                self.height_field.setText(elem.text)
            for elem in tree.iterfind('BoardSettings/Square_Size'):
                self.square_size_field.setText(elem.text)

        def get_xml(self, device):
            root = ET.Element('BoardSettings')
            width = ET.SubElement(root, 'BoardSize_Width')
            width.text = str(self.width_field.text())
            height = ET.SubElement(root, 'BoardSize_Height')
            height.text = str(self.height_field.text())
            size = ET.SubElement(root, 'Square_Size')
            size.text = str(self.square_size_field.text())
            pattern = ET.SubElement(root, 'Calibrate_Pattern')
            pattern.text = "CHESSBOARD"

            return root


    def __init__(self, parent, device_manager, device=None):
        super(DeviceWizard, self).__init__(parent)
        self.resize(360, 120)
        self.setWindowTitle('Add Device')

        self.device = device

        self.deviceManager = device_manager

        layout = QtGui.QVBoxLayout()
        self.tabs = QtGui.QTabWidget(self)

        # Define General Tab
        self.general_tab = self.GeneralTab()
        if device is not None:
            self.general_tab.load_defaults(device)
        self.tabs.addTab(self.general_tab, "General")

        tree = None
        if device is not None:
            tree = ET.ElementTree(file=device.get_conf_path())

        # Define Camera Tab
        self.camera_tab = self.CameraTab()
        if device is not None:
            self.camera_tab.load_defaults(tree)
        self.tabs.addTab(self.camera_tab, "Camera")

        # Define Com Tab
        self.com_tab = self.ComTab()
        if device is not None:
            self.com_tab.load_defaults(tree)
        self.tabs.addTab(self.com_tab, "Communicator")

        # Define Board Tab
        self.board_tab = self.BoardTab()
        if device is not None:
            self.board_tab.load_defaults(tree)
        self.tabs.addTab(self.board_tab, "Board")

        # Define dialog on the bottom
        self.dialog = QtGui.QWidget(self)
        dialog_layout = QtGui.QHBoxLayout()

        self.confirm_button = QtGui.QPushButton(self)
        self.confirm_button.clicked.connect(self.confirm)
        self.confirm_button.setText("Confirm")

        self.cancel_button = QtGui.QPushButton(self)
        self.cancel_button.clicked.connect(self.cancel)
        self.cancel_button.setText("Cancel")

        dialog_layout.addWidget(self.confirm_button)
        dialog_layout.addWidget(self.cancel_button)

        self.dialog.setLayout(dialog_layout)

        # Build main layout
        layout.addWidget(self.tabs)
        layout.addWidget(self.dialog)
        self.setLayout(layout)

        self.setModal(True)
        self.show()

    def confirm(self):
        if self.device is None:
            # add the device
            self.deviceManager.add_device(
                str(self.general_tab.name_field.text()),
                str(self.general_tab.ip_field.text()),
                str(self.general_tab.user_field.text()),
                str(self.general_tab.pass_field.text())
            )
            self.device = self.deviceManager.devices.get(str(self.general_tab.name_field.text()))
        else:
            self.device.set_name(str(self.general_tab.name_field.text()))
            self.device.set_ip(str(self.general_tab.ip_field.text()))
            self.device.username = str(self.general_tab.user_field.text())
            self.device.password = str(self.general_tab.pass_field.text())

        result = self.get_xml()
        path = self.device.get_conf_path()
        with open(path, "w") as text_file:
            text_file.write(result)

        self.accept()

    def get_xml(self):
        root = ET.Element('opencv_storage')
        cam = self.camera_tab.get_xml(self.device)
        root.append(cam)
        com = self.com_tab.get_xml(self.device)
        root.append(com)
        board = self.board_tab.get_xml(self.device)
        root.append(board)

        tree = ET.ElementTree(root)

        f = BytesIO()
        tree.write(f, encoding='utf-8', xml_declaration=True)

        result = minidom.parseString(f.getvalue())
        return result.toprettyxml(indent="\t")

    def cancel(self):
        self.reject()