import json
from PyQt4 import QtGui
import configparser as configparser
from core.device import Device
from widgets.device_wizard import DeviceWizard


class DeviceManager:

    def __init__(self, device_tree):
        self.deviceTree = device_tree
        self.devices = {}

    def add_device(self, name, ip, username, password, origin, root_dir, index = -1):
        if index < 0:
            index = self.deviceTree.topLevelItemCount()
        self.devices[name] = Device(self, name, ip, username, password, origin, root_dir, index)
        self.deviceTree.add_device(self.devices[name])

    def edit_selected(self):
        indices = self.deviceTree.selectionModel().selectedRows()
        if len(indices) <= 0:
            return
        name = self.deviceTree.listOfDevices[sorted(indices)[0].row()].name
        device = self.devices[name]
        DeviceWizard(self.deviceTree, self, device)

    def remove_selected(self):
        indices = self.deviceTree.selectionModel().selectedRows()
        for index in sorted(indices):
            name = self.deviceTree.listOfDevices[sorted(indices)[0].row()].name
            self.deviceTree.remove_device(self.devices[name])
            os.remove(self.devices[name].config_path)
            self.devices[name].destroy()
            del self.devices[name]

    def upload_selected(self):
        indices = self.deviceTree.selectionModel().selectedRows()
        for index in sorted(indices):
            name = self.deviceTree.listOfDevices[index.row()].name
            self.devices[name].upload()

    def download_selected(self):
        indices = self.deviceTree.selectionModel().selectedRows()
        for index in sorted(indices):
            name = self.deviceTree.listOfDevices[index.row()].name
            self.devices[name].download()

    def save_devices(self, path=None):
        if path is None:
            path = str(QtGui.QFileDialog.getSaveFileName(self.deviceTree.parent(), 'Save File', selectedFilter='*.conf'))
        config = configparser.ConfigParser()
        config['Devices'] = {}
        saved_devices = config['Devices']
        for device in self.devices.values():
            saved_devices[device.name] = json.dumps({
                'name': device.name,
                'ip': device.ip,
                'username': device.username,
                'password': device.password,
                'origin': device.is_origin(),
                'root_dir': device.remote_root_dir,
                'index': device.index
            })
        with open(path, 'w') as file:
            config.write(file)
        print "Saved " + path

    def load_devices(self, path=None):
        if path is None:
            path = str(QtGui.QFileDialog.getOpenFileName(self.deviceTree.parent(), 'Open File', selectedFilter='*.conf'))
        config = configparser.ConfigParser()
        config.read(str(path))
        if 'Devices' not in config:
            return
        for k, device in enumerate(config['Devices'].values()):
            device_parsed = json.loads(device)
            self.add_device(device_parsed['name'], device_parsed['ip'], device_parsed['username'],
                device_parsed['password'], device_parsed['origin'], device_parsed['root_dir'], device_parsed['index'])
        self.deviceTree.update()
        print "Loaded " + path

    def close_devices(self):
        for device in self.devices.values():
            device.destroy()
