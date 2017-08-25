import socket
from core.ssh_manager import SSHManager
from widgets.device_tree import DeviceItem
import core.paths as paths


class Device:

    def __init__(self, manager, name, ip, username, password, origin, root_dir, index):
        self.manager = manager
        self.name = name
        self.index = index
        self.ip = ip
        self.username = username
        self.password = password
        self.origin = origin
        self.remote_root_dir = root_dir
        self.ssh_manager = SSHManager(ip, username, password)

        # config path
        self.local_config_path = paths.get_device_config_path(self)

        # setup tree item
        self.tree_item = DeviceItem(device=self, parent=self.manager.deviceTree)

        # download config file
        self.download()

    def update(self):
        self.tree_item.update()

    def destroy(self):
        self.ssh_manager.close()

    def set_name(self, value):
        self.name = value
        self.tree_item.set_name(value)

    def set_ip(self, value):
        self.ip = value
        self.tree_item.set_ip(value)

    def set_calibrated(self, value):
        self.tree_item.set_calibrated(value)

    def set_integrated(self, value):
        self.tree_item.set_integrated(value)

    def set_online(self, value):
        self.tree_item.set_online(value)

    def set_origin(self, value):
        self.tree_item.set_origin(value)

    def is_integrated(self):
        return self.tree_item.integrated

    def is_online(self):
        return self.tree_item.online

    def is_origin(self):
        return self.tree_item.origin

    def upload(self):
        try:
            ssh = self.ssh_manager.get_ssh()
            sftp = ssh.open_sftp()
            # create folder if it doesn't exist yet
            sftp.mkdir(paths.REMOTE_CONFIG_DIR)
        except socket.error:
            self.set_online(False)
            return False
        except IOError:
            pass
        try:
            # send config file
            sftp.put(self.local_config_path, paths.get_remote_config_file(self))
        except IOError:
            print 'File ' + self.local_config_path + ' does not exist.'
            return False
        print 'Succesfully uploaded config file to device ' + self.name + '.'
        self.set_online(True)
        return True

    def download(self):
        try:
            ssh = self.ssh_manager.get_ssh()
            sftp = ssh.open_sftp()
            sftp.get(paths.get_remote_config_file(self), self.local_config_path)
        except socket.error:
            self.set_online(False)
            return False
        except IOError:
            print 'File ' + paths.get_remote_config_file(self) + ' does not exist.'
            return False
        print 'Succesfully downloaded config file from device ' + self.name + '.'
        self.set_online(True)
        return True
