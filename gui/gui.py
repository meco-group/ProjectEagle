#!/usr/bin/env python

import sys
from PyQt4 import QtGui, QtCore
import os
from core.device_manager import DeviceManager
from widgets.calibrate_wizard import CalibrateWizard
from widgets.stereo_calibrate_wizard import StereoCalibrateWizard
from widgets.device_tree import DeviceTree
from widgets.main_window import MainWindow
from widgets.menu_bar import MenuBar
from widgets.shelf import Shelf
from widgets.device_wizard import DeviceWizard
from core.paths import MAIN_CONFIG_PATH


class Application(QtGui.QApplication):
    def __init__(self):
        super(Application, self).__init__(sys.argv)

        # create config folder
        if not os.path.isdir(os.path.join(os.getcwd(), 'config')):
            os.mkdir(os.path.join(os.getcwd()), 'config')

        # setup the main window
        self.window = MainWindow()
        self.setup_window()

        # setup the main widget
        self.box = QtGui.QHBoxLayout()

        # setup the device tree
        self.deviceTree = DeviceTree(self.window)
        self.deviceManager = DeviceManager(self.deviceTree)
        self.setup_device_tree()
        self.deviceManager.load_devices(MAIN_CONFIG_PATH)

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
        self.connect(self.menu_bar.save, QtCore.SIGNAL('triggered()'), lambda: self.deviceManager.save_devices(self.config_path))
        self.connect(self.menu_bar.save_as, QtCore.SIGNAL('triggered()'), self.deviceManager.save_devices)
        self.connect(self.menu_bar.load, QtCore.SIGNAL('triggered()'), self.deviceManager.load_devices)

    def setup_device_tree(self):
        # on selection change event
        self.deviceTree.connect(self.deviceTree.selectionModel(),
                     QtCore.SIGNAL("selectionChanged(QItemSelection, QItemSelection)"),
                     self.handle_selection_update)

        self.deviceTree.doubleClicked.connect(self.handle_double_click)

    def setup_shelf(self):
        self.shelf.add_button.clicked.connect(self.handle_add)
        self.shelf.edit_button.clicked.connect(self.handle_edit)
        # self.shelf.remove_button.clicked.connect(self.handle_remove)
        self.shelf.upload_button.clicked.connect(self.handle_upload)
        self.shelf.download_button.clicked.connect(self.handle_download)
        self.shelf.calibrate_button.clicked.connect(self.handle_calibrate)
        self.shelf.st_calibrate_button.clicked.connect(self.handle_stereo_calibrate)
        self.handle_selection_update()

    def start(self):
        # show gui
        self.window.show()

        # start application (this ends the script)
        sys.exit(self.exec_())

    def stop(self):
        self.deviceManager.save_devices(MAIN_CONFIG_PATH)
        self.deviceManager.close_devices()

    def handle_add(self):
        DeviceWizard(self.window, self.deviceManager)

    def handle_edit(self):
        self.deviceManager.edit_selected()

    def handle_remove(self):
        self.deviceManager.remove_selected()

    def handle_upload(self):
        self.deviceManager.upload_selected()

    def handle_download(self):
        self.deviceManager.download_selected()

    def handle_calibrate(self):
        indexes = self.deviceTree.selectionModel().selectedRows()
        device = self.deviceTree.itemFromIndex(indexes[0]).device
        CalibrateWizard(self.window, device)

    def handle_stereo_calibrate(self):
        indexes = self.deviceTree.selectionModel().selectedRows()
        left_device = self.deviceTree.itemFromIndex(indexes[0]).device
        right_device = self.deviceTree.itemFromIndex(indexes[1]).device
        StereoCalibrateWizard(self.window, left_device, right_device)

    def handle_selection_update(self):
        # Button enable logic
        indexes = self.deviceTree.selectionModel().selectedRows()
        count = len(indexes)

        # self.shelf.remove_button.setEnabled(count >= 1)
        self.shelf.upload_button.setEnabled(count >= 1)
        self.shelf.download_button.setEnabled(count >= 1)

        if count == 1:
            self.shelf.edit_button.setEnabled(True)
            self.shelf.calibrate_button.setEnabled(True)
        else:
            self.shelf.edit_button.setEnabled(False)
            self.shelf.calibrate_button.setEnabled(False)

        if count == 2:
            left_device = self.deviceTree.itemFromIndex(indexes[0])
            right_device = self.deviceTree.itemFromIndex(indexes[1])

            cond = ((left_device.integrated and not right_device.integrated) or
                   (not left_device.integrated and right_device.integrated)) and \
                   (left_device.calibrated and right_device.calibrated)
            self.shelf.st_calibrate_button.setEnabled(cond)
        else:
            self.shelf.st_calibrate_button.setEnabled(False)

    def handle_double_click(self, index):
        pass


app = Application()
app.start()
