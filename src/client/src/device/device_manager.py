import json
from PyQt4 import QtGui

import yaml

import configparser as configparser

from src.device.device import Device
from src.widgets.device_wizard import DeviceWizard


class DeviceManager:
    def __init__(self, device_tree):
        self.deviceTree = device_tree
        self.devices = {}

    def add_device(self, name, ip, username, password):
        self.devices[name] = Device(self, name, ip, username, password)
        self.deviceTree.add_device(self.devices[name])

    def connect_device(self, name):
        self.devices[name].connect()

    def connect_selected(self):
        indexes = self.deviceTree.selectionModel().selectedRows()
        for index in sorted(indexes):
            name = self.deviceTree.listOfDevices[index.row()].name
            if self.devices[name].is_connected():
                # print "disconnecting: "+name
                self.devices[name].disconnect()
            else:
                # print "connecting: "+name
                self.devices[name].connect()

    def edit_selected(self):
        indexes = self.deviceTree.selectionModel().selectedRows()
        if len(indexes) <= 0:
            return

        name = self.deviceTree.listOfDevices[sorted(indexes)[0].row()].name
        device = self.devices[name]
        DeviceWizard(self.deviceTree, self, device)

    def save_devices(self, path=None):
        # TODO we should be able to clear all devices that are not saved in main.conf
        if path is None:
            path = str(QtGui.QFileDialog.getSaveFileName(self.deviceTree.parent(), 'Save File', selectedFilter='*.conf'))

        print "Saving: "+path

        config = configparser.ConfigParser()
        config['Devices'] = {}
        saved_devices = config['Devices']
        for device in self.devices.values():
            saved_devices[device.name] = json.dumps({
                'name': device.name,
                'ip': device.ip,
                'username': device.username,
                'password': device.password
            })

        with open(path, 'w') as configfile:
            config.write(configfile)

    def load_devices(self, path=None):
        if path is None:
            path = str(QtGui.QFileDialog.getOpenFileName(self.deviceTree.parent(), 'Open File', selectedFilter='*.conf'))

        print "Loading: "+path

        config = configparser.ConfigParser()
        config.read(str(path))

        assert('Devices' in config)

        for device in config['Devices'].values():
            device_parsed = json.loads(device)
            self.add_device(device_parsed['name'], device_parsed['ip'], device_parsed['username'], device_parsed['password'])

    def close_all_connections(self):
        for device in self.devices.values():
            device.disconnect()


