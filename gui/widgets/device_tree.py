from PyQt4 import QtGui, QtCore
from xml.etree import ElementTree as et


class DeviceTree(QtGui.QTreeWidget):
    LABELS = ['name', 'ip', 'calibrated', 'integrated', 'online', 'origin']

    def __init__(self, parent):
        super(DeviceTree, self).__init__(parent)

        hLabels = QtCore.QStringList()

        self.setColumnCount(len(self.LABELS))
        for label in self.LABELS:
            hLabels.append(label)

        self.setHeaderLabels(hLabels)
        self.setContextMenuPolicy(QtCore.Qt.CustomContextMenu)

        self.listOfDevices = []

        self.header().setStretchLastSection(False)
        self.header().setResizeMode(0, QtGui.QHeaderView.Stretch)
        self.setColumnWidth(0, 150)
        self.setColumnWidth(1, 150)
        self.setColumnWidth(2, 80)
        self.setColumnWidth(3, 80)
        self.setColumnWidth(4, 80)
        self.setColumnWidth(5, 80)
        self.setBaseSize(680, 480)

        self.setSelectionMode(QtGui.QAbstractItemView.ExtendedSelection)
        self.itemChanged.connect(self.handle_checkbox)

    def add_device(self, device):
        self.listOfDevices.append(device.tree_item)

    def remove_device(self, device):
        self.listOfDevices.remove(device.tree_item)
        self.invisibleRootItem().removeChild(device.tree_item)

    def handle_checkbox(self, item, column):
        self.blockSignals(True)
        if column == 5:
            if item.checkState(column) == QtCore.Qt.Checked:
                item.set_origin(True)
                for it in self.listOfDevices:
                    if item != it:
                        it.set_origin(False)
            elif item.checkState(column) == QtCore.Qt.Unchecked:
                item.set_origin(False)
            self.update()
        self.blockSignals(False)

    def update(self):
        for it in self.listOfDevices:
            it.update()


class DeviceItem(QtGui.QTreeWidgetItem):

    def __init__(self, device, parent):
        super(DeviceItem, self).__init__(parent)
        self.device = device
        self.set_name(device.name)
        self.set_ip(device.ip)
        self.set_origin(device.origin)

    def update(self):
        tree = et.parse(self.device.local_config_path)
        root = tree.getroot()
        self.set_calibrated(root.find('calibrator').find('calibrated').text == '1')
        self.set_integrated(root.find('calibrator').find('integrated').text == '1')
        if self.origin:
            self.set_integrated(True)

    def set_name(self, value):
        self.name = value
        self.setText(0, value)

    def set_ip(self, value):
        self.ip = value
        self.setText(1, value)

    def set_calibrated(self, value):
        self.calibrated = value
        self.setText(2, str(value))

    def set_integrated(self, value):
        self.integrated = value
        self.setText(3, str(value))

    def set_online(self, value):
        self.online = value
        self.setText(4, str(value))

    def set_origin(self, value):
        self.origin = value
        if value:
            self.setCheckState(5, QtCore.Qt.Checked)
        else:
            self.setCheckState(5, QtCore.Qt.Unchecked)
