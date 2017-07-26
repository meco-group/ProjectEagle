import os

from pathlib import Path

from src.comm.ssh_manager import SSHManager
from src.config.extrinsic_conf import ExtrinsicConfig
from src.widgets.core.device_tree import DeviceItem


class Device:
    ORIGIN = "origin"
    SAVE_PATH = "../config/main.conf"

    def __init__(self, manager, name, ip, username, password):
        # store properties
        self.manager = manager
        self.name = name

        # store ssh properties
        self.ip = ip
        self.username = username
        self.password = password

        # create root folder
        if not os.path.isdir(self.get_root()):
            os.mkdir(self.get_root())

        # check if this device is the origin
        if self.name == Device.ORIGIN:
            ExtrinsicConfig().write(self.get_extrinsic_path())

        # Setup tree item
        self.treeItem = DeviceItem(device=self, parent=self.manager.deviceTree)

        # initialise ssh
        self.ssh_manager = SSHManager(ip, username, password)

    def update(self):
        self.treeItem.update()

    def destroy(self):
        self.ssh_manager.close()

    def set_name(self, value):
        self.name = value
        self.treeItem.set_name(value)

    def set_ip(self, value):
        self.ip = value

    def set_calibrated(self, value):
        self.treeItem.set_calibrated(value)

    def set_integrated(self, value):
        self.treeItem.set_integrated(value)

    def is_integrated(self):
        return self.treeItem.integrated

    def get_root(self):
        return os.path.abspath(os.path.join("../config/devices/", self.name))

    def get_config_path(self):
        return os.path.join(self.get_root(), "config.xml")

    def get_calibration_path(self):
        return os.path.join(self.get_root(), "calibration.xml")

    def get_extrinsic_path(self):
        return os.path.join(self.get_root(), "extrinsic.xml")

    def sync_config(self):
        ssh = self.ssh_manager.get_ssh()
        sftp = ssh.open_sftp()

        # make sure the directory exists
        target = str(Path(self.get_root()).relative_to(os.path.abspath("../../..")))
        print "Sending config to remote:"
        try:            sftp.mkdir(os.path.join("/home/pi/ProjectEagle", target))
        except IOError:
            pass

        # Send general config
        target = str(Path(self.get_config_path()).relative_to(os.path.abspath("../../..")))
        print target
        sftp.put(
            self.get_config_path(),
            os.path.join("/home/pi/ProjectEagle", target)
        )

        ssh.close()







# dev = Device("ceil2", "192.168.11.133", "pi", "raspberry")
# dev.connect()
# dev.send_config("/home/peter/Documents/Honours/ProjectEagle/src/client/src/test.xml", "/home/pi/test.xml")
