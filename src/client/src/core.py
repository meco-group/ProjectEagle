#!/usr/bin/env python

from PyQt4 import QtGui, QtCore
import sys

from PyQt4.QtCore import Qt

from src.device.device_manager import DeviceManager
from src.widgets.calibrate_wizard import CalibrateWizard
from src.widgets.device_tree import DeviceTree
from src.widgets.device_wizard import DeviceWizard


class Application(QtGui.QApplication):

    def __init__(self):
        super(Application, self).__init__(sys.argv)

        # setup the main window
        self.window = QtGui.QMainWindow()
        self.window.resize(720, 480)
        self.window.setWindowTitle('Camera Devices')

        # setup the main widget
        self.box = QtGui.QHBoxLayout()

        # setup the device tree
        deviceTree = DeviceTree(self.window)
        self.deviceManager = DeviceManager(deviceTree)

        # setup the menu bar
        self.window.setMenuBar(MenuBar(self.window, device_manager=self.deviceManager))

        self.box.addWidget(deviceTree)
        self.box.setStretch(0, 6)

        # setup the buttons on the right
        self.shelf = Shelf(self.window, self.deviceManager)
        self.box.addWidget(self.shelf)
        self.box.setStretch(1, 0)

        self.centralWidget = QtGui.QWidget(self.window)
        self.centralWidget.setLayout(self.box)

        self.window.setCentralWidget(self.centralWidget)

    def start(self):
        # show gui
        self.window.show()

        # start application (this ends the script)
        sys.exit(self.exec_())


class MenuBar(QtGui.QMenuBar):
    def __init__(self, parent, device_manager=None):
        super(MenuBar, self).__init__(parent)

        self.file = self.addMenu('&File')

        exit = QtGui.QAction(QtGui.QIcon('icons/exit.png'), 'Exit', self.parent())
        exit.setShortcut('Ctrl+Q')
        exit.setStatusTip('Exit application')

        save = QtGui.QAction(QtGui.QIcon('icons/save.png'), 'Save', self.parent())
        save.setShortcut('Ctrl+S')
        save.setStatusTip('Save devices')

        load = QtGui.QAction(QtGui.QIcon('icons/open.png'), 'Open', self.parent())
        load.setShortcut('Ctrl+O')
        load.setStatusTip('Open devices')

        self.parent().connect(exit, QtCore.SIGNAL('triggered()'), QtCore.SLOT('close()'))
        self.parent().connect(save, QtCore.SIGNAL('triggered()'), device_manager.save_devices)
        self.parent().connect(load, QtCore.SIGNAL('triggered()'), device_manager.load_devices)
        self.file.addAction(exit)
        self.file.addAction(save)
        self.file.addAction(load)

class Shelf(QtGui.QWidget):
    def __init__(self, parent, device_manager):
        super(Shelf, self).__init__(parent)

        self.deviceManager = device_manager

        self.box = QtGui.QVBoxLayout()

        self.box.setAlignment(Qt.AlignTop)

        # Add buttons
        self.add_button = QtGui.QPushButton('Add', self)
        self.box.addWidget(self.add_button)
        self.add_button.clicked.connect(self.handle_add)

        self.edit_button = QtGui.QPushButton('Edit', self)
        self.box.addWidget(self.edit_button)
        self.edit_button.clicked.connect(self.handle_edit)

        self.connect_button = QtGui.QPushButton('Connect', self)
        self.box.addWidget(self.connect_button)
        self.connect_button.clicked.connect(self.handle_connect)

        self.vertical_spacer = QtGui.QSpacerItem(20, 40, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.box.addItem(self.vertical_spacer)

        self.calibrate_button = QtGui.QPushButton('Calibrate', self)
        self.box.addWidget(self.calibrate_button)
        self.calibrate_button.clicked.connect(self.handle_calibrate)

        self.st_calibrate_button = QtGui.QPushButton('Stereo Calibrate', self)
        self.box.addWidget(self.st_calibrate_button)

        # on selection change event
        self.deviceManager.deviceTree.connect(self.deviceManager.deviceTree.selectionModel(),
                     QtCore.SIGNAL("selectionChanged(QItemSelection, QItemSelection)"),
                     self.handle_selection_update)

        # set all buttons to correct state
        self.handle_selection_update()

        # assign layout
        self.setLayout(self.box)

    def handle_add(self):
        DeviceWizard(self, self.deviceManager)

    def handle_connect(self):
        self.deviceManager.connect_selected()
        self.handle_selection_update()

    def handle_edit(self):
        self.deviceManager.edit_selected()

    def handle_calibrate(self):
        device = self.deviceManager.deviceTree.listOfDevices[0].device
        print "calibrate "+device.name
        CalibrateWizard(self, device)

    def handle_selection_update(self):
        # Button enable logic
        indexes = self.deviceManager.deviceTree.selectionModel().selectedRows()
        count = len(indexes)

        if (count == 1):
            self.edit_button.setEnabled(True)
            self.connect_button.setEnabled(True)

            deviceItem = self.deviceManager.deviceTree.listOfDevices[0]
            self.calibrate_button.setEnabled(deviceItem.device.is_connected())
        else:
            self.edit_button.setEnabled(False)
            self.connect_button.setEnabled(False)
            self.calibrate_button.setEnabled(False)

        self.st_calibrate_button.setEnabled(count == 2)



app = Application()
app.start()