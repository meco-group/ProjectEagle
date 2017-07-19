from PyQt4 import QtGui, QtCore

from PyQt4.QtCore import Qt


class Shelf(QtGui.QWidget):
    def __init__(self, parent):
        super(Shelf, self).__init__(parent)

        self.box = QtGui.QVBoxLayout()

        self.box.setAlignment(Qt.AlignTop)

        # Add buttons
        self.add_button = QtGui.QPushButton('Add', self)
        self.box.addWidget(self.add_button)

        self.edit_button = QtGui.QPushButton('Edit', self)
        self.box.addWidget(self.edit_button)

        self.connect_button = ConnectButton(self)
        self.box.addWidget(self.connect_button)

        self.vertical_spacer = QtGui.QSpacerItem(20, 40, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.box.addItem(self.vertical_spacer)

        self.calibrate_button = QtGui.QPushButton('Calibrate', self)
        self.box.addWidget(self.calibrate_button)

        self.st_calibrate_button = QtGui.QPushButton('Stereo Calibrate', self)
        self.box.addWidget(self.st_calibrate_button)

        # assign layout
        self.setLayout(self.box)

class ConnectButton(QtGui.QPushButton):
    def __init__(self, parent=None):
        super(ConnectButton, self).__init__("Connect", parent)
        self.is_connected = False

    def update(self, is_connected):
        self.is_connected = is_connected
        if is_connected:
            self.setText("Disconnect")
        else:
            self.setText("Connect")

