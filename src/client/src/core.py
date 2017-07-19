#!/usr/bin/env python

import sys
from PyQt4 import QtGui, QtCore

from src.device.device_manager import DeviceManager
from src.widgets.calibrate_wizard import CalibrateWizard
from src.widgets.core.device_tree import DeviceTree
from src.widgets.core.main_window import MainWindow
from src.widgets.core.menu_bar import MenuBar
from src.widgets.core.shelf import Shelf
from src.widgets.device_wizard import DeviceWizard


class Application(QtGui.QApplication):

    def __init__(self):
        super(Application, self).__init__(sys.argv)

        # setup the main window
        self.window = MainWindow()
        self.setup_window()

        # setup the main widget
        self.box = QtGui.QHBoxLayout()

        # setup the device tree
        self.deviceTree = DeviceTree(self.window)
        self.deviceManager = DeviceManager(self.deviceTree)
        self.setup_device_tree()

        # setup the menu bar
        self.menu_bar = MenuBar(self.window)
        self.setup_menu()

        self.window.setMenuBar(self.menu_bar)

        self.box.addWidget(self.deviceTree)
        self.box.setStretch(0, 6)

        # setup the buttons on the right
        self.shelf = Shelf(self.window)
        self.setup_shelf()

        self.box.addWidget(self.shelf)
        self.box.setStretch(1, 0)

        self.centralWidget = QtGui.QWidget(self.window)
        self.centralWidget.setLayout(self.box)

        self.window.setCentralWidget(self.centralWidget)

    def setup_window(self):
        self.window.resize(720, 480)
        self.window.setWindowTitle('Camera Devices')

        self.window.close_trigger.connect(self.stop)

    def setup_menu(self):
        self.window.connect(self.menu_bar.exit, QtCore.SIGNAL('triggered()'), QtCore.SLOT('close()'))
        self.connect(self.menu_bar.save, QtCore.SIGNAL('triggered()'), self.deviceManager.save_devices)
        self.connect(self.menu_bar.load, QtCore.SIGNAL('triggered()'), self.deviceManager.load_devices)

    def setup_device_tree(self):
        # on selection change event
        self.deviceTree.connect(self.deviceTree.selectionModel(),
                     QtCore.SIGNAL("selectionChanged(QItemSelection, QItemSelection)"),
                     self.handle_selection_update)

    def setup_shelf(self):
        self.shelf.add_button.clicked.connect(self.handle_add)
        self.shelf.edit_button.clicked.connect(self.handle_edit)
        self.shelf.connect_button.clicked.connect(self.handle_connect)
        self.shelf.calibrate_button.clicked.connect(self.handle_calibrate)
        self.handle_selection_update()

    def start(self):
        # show gui
        self.window.show()

        # start application (this ends the script)
        sys.exit(self.exec_())

    def stop(self):
        self.deviceManager.close_all_connections()

    def handle_add(self):
        DeviceWizard(self.window, self.deviceManager)

    def handle_connect(self):
        def handle_connect_delayed():
            self.deviceManager.connect_selected()
            self.handle_selection_update()

        # Start connecting
        QtCore.QTimer.singleShot(0, handle_connect_delayed)

    def handle_edit(self):
        self.deviceManager.edit_selected()

    def handle_calibrate(self):
        device = self.deviceManager.deviceTree.listOfDevices[0].device
        print "calibrate " + device.name
        CalibrateWizard(self.window, device)

    def handle_selection_update(self):
        # Button enable logic
        indexes = self.deviceTree.selectionModel().selectedRows()
        count = len(indexes)

        if count == 1:
            self.shelf.edit_button.setEnabled(True)
            self.shelf.connect_button.setEnabled(True)

            deviceItem = self.deviceTree.listOfDevices[0]
            self.shelf.connect_button.update(deviceItem.device.is_connected())
            self.shelf.calibrate_button.setEnabled(deviceItem.device.is_connected())
        else:
            self.shelf.edit_button.setEnabled(False)
            self.shelf.connect_button.setEnabled(False)
            self.shelf.calibrate_button.setEnabled(False)

        self.shelf.st_calibrate_button.setEnabled(count == 2)

app = Application()
app.start()