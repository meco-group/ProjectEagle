from PyQt4 import QtGui
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

        # self.remove_button = QtGui.QPushButton('Remove', self)
        # self.box.addWidget(self.remove_button)

        self.upload_button = QtGui.QPushButton('Upload', self)
        self.box.addWidget(self.upload_button)

        self.download_button = QtGui.QPushButton('Download', self)
        self.box.addWidget(self.download_button)

        self.vertical_spacer = QtGui.QSpacerItem(
            20, 40, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.box.addItem(self.vertical_spacer)

        self.calibrate_button = QtGui.QPushButton('Calibrate', self)
        self.box.addWidget(self.calibrate_button)

        self.st_calibrate_button = QtGui.QPushButton('Stereo Calibrate', self)
        self.box.addWidget(self.st_calibrate_button)

        # assign layout
        self.setLayout(self.box)
