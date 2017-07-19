from PyQt4 import QtGui, QtCore


class DeviceTree(QtGui.QTreeWidget):
    LABELS = ["name", "ip", "calibrated", "enabled", "connected"]

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
        self.setColumnWidth(0, 200)
        self.setColumnWidth(1, 200)
        self.setColumnWidth(2, 80)
        self.setColumnWidth(3, 80)
        self.setColumnWidth(4, 80)
        self.setBaseSize(680, 480)

        self.setSelectionMode(QtGui.QAbstractItemView.ExtendedSelection)

    def add_device(self, device):
        self.listOfDevices.append(device.treeItem)


class DeviceItem(QtGui.QTreeWidgetItem):
    def __init__(self, device, parent):
        super(DeviceItem, self).__init__(parent)

        self.device = device

        self.set_name(device.name)
        self.set_ip(device.ip)
        self.set_calibrated(False)
        self.set_enabled(False)
        self.set_connected(False)

    def set_name(self, value):
        self.name = value
        self.setText(0, value)

    def set_ip(self, value):
        self.ip = value
        self.setText(1, value)

    def set_calibrated(self, value):
        self.calibrated = value
        self.setText(2, str(value))

    def set_enabled(self, value):
        self.enabled = value
        self.setText(3, str(value))

    def set_connected(self, value):
        self.connected = value
        self.setText(4, str(value))


