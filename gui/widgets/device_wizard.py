from PyQt4 import QtGui
import sys
from xml.etree import ElementTree as et
from core.paths import MAIN_CONFIG_PATH, DEFAULT_DEVICE_CONFIG_PATH, DEFAULT_REMOTE_ROOT_DIR


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
            self.root_label = QtGui.QLabel(self)

            self.name_label.setText('Name:')
            self.ip_label.setText('Ip:')
            self.user_label.setText('Username:')
            self.pass_label.setText('Password:')
            self.root_label.setText('Root directory:')

            self.name_field = QtGui.QLineEdit(self)
            self.ip_field = QtGui.QLineEdit(self)
            self.user_field = QtGui.QLineEdit(self)
            self.pass_field = QtGui.QLineEdit(self)
            self.pass_field.setEchoMode(QtGui.QLineEdit.Password)
            self.root_field = QtGui.QLineEdit(self)

            grid_layout.addWidget(self.name_label, 0, 0)
            grid_layout.addWidget(self.ip_label, 1, 0)
            grid_layout.addWidget(self.user_label, 2, 0)
            grid_layout.addWidget(self.pass_label, 3, 0)
            grid_layout.addWidget(self.root_label, 4, 0)
            grid_layout.addWidget(self.name_field, 0, 1)
            grid_layout.addWidget(self.ip_field, 1, 1)
            grid_layout.addWidget(self.user_field, 2, 1)
            grid_layout.addWidget(self.pass_field, 3, 1)
            grid_layout.addWidget(self.root_field, 4, 1)

            grid_layout.setColumnStretch(1, 10)
            grid_layout.setMargin(0)

            self.grid.setLayout(grid_layout)
            box_layout.addWidget(self.grid)

            verticalSpacer = QtGui.QSpacerItem(
                20, 40, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
            box_layout.addItem(verticalSpacer)

            self.setLayout(box_layout)

        def load_defaults(self, device):
            if device is not None:
                self.name_field.setText(device.name)
                self.ip_field.setText(device.ip)
                self.user_field.setText(device.username)
                self.pass_field.setText(device.password)
                self.root_field.setText(device.remote_root_dir)
            else:
                self.root_field.setText(DEFAULT_REMOTE_ROOT_DIR)

    class CameraTab(QtGui.QWidget):
        RESOLUTIONS = ['640 x 480', '1280 x 720', '1920 x 1080']
        CAM_TYPES = ['PICAM', 'OPICAM', 'SEE3CAM', 'LATCAM', 'OCAM']

        def __init__(self, parent=None):
            super(DeviceWizard.CameraTab, self).__init__(parent)

            box_layout = QtGui.QVBoxLayout()

            self.grid = QtGui.QWidget(self)
            grid_layout = QtGui.QGridLayout()

            self.index_label = QtGui.QLabel(self)
            self.type_label = QtGui.QLabel(self)
            self.snap_res_label = QtGui.QLabel(self)
            self.stream_res_label = QtGui.QLabel(self)

            self.index_label.setText('Index:')
            self.type_label.setText('Type:')
            self.snap_res_label.setText('Snapshot Resolution')
            self.stream_res_label.setText('Stream Resolution')

            self.index_field = QtGui.QLineEdit(self)
            self.index_field.setValidator(QtGui.QIntValidator(0, 1e10, self))

            self.type_field = QtGui.QComboBox(self)
            for el in self.CAM_TYPES:
                self.type_field.addItem(el)

            self.snap_res_field = QtGui.QComboBox(self)
            for el in self.RESOLUTIONS:
                self.snap_res_field.addItem(el)

            self.stream_res_field = QtGui.QComboBox(self)
            for el in self.RESOLUTIONS:
                self.stream_res_field.addItem(el)

            grid_layout.addWidget(self.index_label, 0, 0)
            grid_layout.addWidget(self.type_label, 1, 0)
            grid_layout.addWidget(self.snap_res_label, 2, 0)
            grid_layout.addWidget(self.stream_res_label, 3, 0)
            grid_layout.addWidget(self.index_field, 0, 1)
            grid_layout.addWidget(self.type_field, 1, 1)
            grid_layout.addWidget(self.snap_res_field, 2, 1)
            grid_layout.addWidget(self.stream_res_field, 3, 1)

            grid_layout.setColumnStretch(1, 10)
            grid_layout.setMargin(0)

            self.grid.setLayout(grid_layout)
            box_layout.addWidget(self.grid)

            verticalSpacer = QtGui.QSpacerItem(
                20, 40, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
            box_layout.addItem(verticalSpacer)

            self.setLayout(box_layout)

        def load_defaults(self, root):
            for elem in root.findall('index'):
                self.index_field.setText(elem.text)
            for elem in root.findall('type'):
                index = self.type_field.findText(elem.text)
                self.type_field.setCurrentIndex(index)
            for elem in root.findall('resolution'):
                width = elem.find('width').text
                height = elem.find('height').text
                index = self.snap_res_field.findText('%s x %s' % (width, height))
                self.snap_res_field.setCurrentIndex(index)
            for elem in root.findall('stream_resolution'):
                width = elem.find('width').text
                height = elem.find('height').text
                index = self.stream_res_field.findText('%s x %s' % (width, height))
                self.stream_res_field.setCurrentIndex(index)

        def get_xml(self, root):
            for elem in root.findall('index'):
                elem.text = str(self.index_field.text())
            for elem in root.findall('type'):
                elem.text = str(self.type_field.currentText())
            for elem in root.findall('resolution'):
                res = str(self.snap_res_field.currentText()).split(' x ')
                elem.find('width').text = res[0]
                elem.find('height').text = res[1]
            for elem in root.findall('stream_resolution'):
                res = str(self.stream_res_field.currentText()).split(' x ')
                elem.find('width').text = res[0]
                elem.find('height').text = res[1]

    class ComTab(QtGui.QWidget):
        def __init__(self, parent=None):
            super(DeviceWizard.ComTab, self).__init__(parent)

            box_layout = QtGui.QVBoxLayout()

            self.grid = QtGui.QWidget(self)
            grid_layout = QtGui.QGridLayout()

            self.group_label = QtGui.QLabel(self)
            self.interface_label = QtGui.QLabel(self)
            self.wait_time_label = QtGui.QLabel(self)

            self.group_label.setText('Group:')
            self.interface_label.setText('Interface:')
            self.wait_time_label.setText('Zyre Wait Time:')

            self.group_field = QtGui.QLineEdit(self)
            self.interface_field = QtGui.QLineEdit(self)
            self.wait_time_field = QtGui.QLineEdit(self)
            self.wait_time_field.setValidator(QtGui.QIntValidator(0, 1e10, self))

            grid_layout.addWidget(self.group_label, 0, 0)
            grid_layout.addWidget(self.interface_label, 1, 0)
            grid_layout.addWidget(self.wait_time_label, 2, 0)
            grid_layout.addWidget(self.group_field, 0, 1)
            grid_layout.addWidget(self.interface_field, 1, 1)
            grid_layout.addWidget(self.wait_time_field, 2, 1)

            grid_layout.setColumnStretch(1, 10)
            grid_layout.setMargin(0)

            self.grid.setLayout(grid_layout)
            box_layout.addWidget(self.grid)

            verticalSpacer = QtGui.QSpacerItem(
                20, 40, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
            box_layout.addItem(verticalSpacer)

            self.setLayout(box_layout)

        def load_defaults(self, root):
            for elem in root.findall('group'):
                self.group_field.setText(elem.text)
            for elem in root.findall('interface'):
                self.interface_field.setText(elem.text)
            for elem in root.findall('zyre_wait_time'):
                self.wait_time_field.setText(elem.text)

        def get_xml(self, root):
            for elem in root.findall('group'):
                elem.text = str(self.group_field.text())
            for elem in root.findall('interface'):
                elem.text = str(self.interface_field.text())
            for elem in root.findall('zyre_wait_time'):
                elem.text = str(self.wait_time_field.text())

    class BoardTab(QtGui.QWidget):
        def __init__(self, parent=None):
            super(DeviceWizard.BoardTab, self).__init__(parent)

            box_layout = QtGui.QVBoxLayout()

            self.grid = QtGui.QWidget(self)
            grid_layout = QtGui.QGridLayout()

            self.width_label = QtGui.QLabel(self)
            self.height_label = QtGui.QLabel(self)
            self.square_size_label = QtGui.QLabel(self)

            self.width_label.setText('Board Width: ')
            self.height_label.setText('Board Height')
            self.square_size_label.setText('Square Size: ')

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

        def load_defaults(self, root):
            for elem in root.iterfind('board_width'):
                self.width_field.setText(elem.text)
            for elem in root.iterfind('board_height'):
                self.height_field.setText(elem.text)
            for elem in root.iterfind('square_size'):
                self.square_size_field.setText(elem.text)

        def get_xml(self, root):
            for elem in root.iterfind('board_width'):
                elem.text = str(self.width_field.text())
            for elem in root.iterfind('board_height'):
                elem.text = str(self.height_field.text())
            for elem in root.iterfind('square_size'):
                elem.text = str(self.square_size_field.text())

    def __init__(self, parent, device_manager, device=None):
        super(DeviceWizard, self).__init__(parent)
        self.resize(360, 120)
        self.setWindowTitle('Add Device')
        self.device = device

        self.deviceManager = device_manager

        layout = QtGui.QVBoxLayout()
        self.tabs = QtGui.QTabWidget(self)

        # define General Tab
        self.general_tab = self.GeneralTab()
        self.general_tab.load_defaults(device)
        self.tabs.addTab(self.general_tab, 'General')

        # load xml data
        tree = None
        if device is not None:
            tree = et.parse(device.local_config_path)
        else:
            try:
                tree = et.parse(DEFAULT_DEVICE_CONFIG_PATH)
            except IOError:
                print 'Could not find default config path.'
                tree = None

        # define tabs
        self.camera_tab = self.CameraTab()
        self.tabs.addTab(self.camera_tab, 'Camera')
        self.com_tab = self.ComTab()
        self.tabs.addTab(self.com_tab, 'Communicator')
        self.board_tab = self.BoardTab()
        self.tabs.addTab(self.board_tab, 'Board')
        if tree is not None:
            self.camera_tab.load_defaults(tree.find('camera'))
            self.com_tab.load_defaults(tree.find('communicator'))
            self.board_tab.load_defaults(tree.find('calibrator'))

        # define dialog on the bottom
        self.dialog = QtGui.QWidget(self)
        dialog_layout = QtGui.QHBoxLayout()

        self.confirm_button = QtGui.QPushButton(self)
        self.confirm_button.clicked.connect(self.confirm)
        self.confirm_button.setText('Confirm')

        self.cancel_button = QtGui.QPushButton(self)
        self.cancel_button.clicked.connect(self.cancel)
        self.cancel_button.setText('Cancel')

        dialog_layout.addWidget(self.confirm_button)
        dialog_layout.addWidget(self.cancel_button)

        self.dialog.setLayout(dialog_layout)

        # build main layout
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
                str(self.general_tab.pass_field.text()),
                False,
                str(self.general_tab.root_field.text()),
                len(self.deviceManager.devices.keys()),
            )
            self.device = self.deviceManager.devices.get(str(self.general_tab.name_field.text()))
        else:
            self.device.set_name(str(self.general_tab.name_field.text()))
            self.device.set_ip(str(self.general_tab.ip_field.text()))
            self.device.username = str(self.general_tab.user_field.text())
            self.device.password = str(self.general_tab.pass_field.text())
            self.device.remote_root_dir = str(self.general_tab.root_field.text())
        self.write_xml(self.device.local_config_path)
        self.deviceManager.save_devices(MAIN_CONFIG_PATH)
        self.deviceManager.deviceTree.update()
        self.device.upload()
        self.accept()

    def write_xml(self, path):
        try:
            tree = et.parse(path)
        except IOError:
            try:
                tree = et.parse(DEFAULT_DEVICE_CONFIG_PATH)
            except IOError:
                print 'Could not find default config path.'
                return
        root = tree.getroot()
        self.camera_tab.get_xml(root.find('camera'))
        self.com_tab.get_xml(root.find('communicator'))
        self.board_tab.get_xml(root.find('calibrator'))
        file = open(path, 'w')
        file.write('<?xml version="1.0"?>\n')
        tree.write(file)
        file.close()

    def cancel(self):
        self.reject()
