import os

from pathlib import Path

from src.comm.ssh_manager import SSHManager
from src.config.extrinsic_conf import ExtrinsicConfig
from src.config.path_finder import PathFinder
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

        # get the pathfinders
        self.local_path_finder = PathFinder(self, False)
        self.remote_path_finder = PathFinder(self, True)

        # create root folder
        if not os.path.isdir(self.local_path_finder.get_path("[DEVICE_FOLDER]")):
            os.mkdir(self.local_path_finder.get_path("[DEVICE_FOLDER]"))

        # check if this device is the origin
        if self.name == Device.ORIGIN:
            ExtrinsicConfig().write(self.local_path_finder.get_path("[EXTRINSIC_CALIBRATION]"))

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

    def sync_config(self):
        ssh = self.ssh_manager.get_ssh()
        sftp = ssh.open_sftp()

        # make sure the directory exists
        print "Sending config to remote:"
        print "from "+self.local_path_finder.get_path("[DEVICE_CONFIG]") + " to " + self.remote_path_finder.get_path("[DEVICE_CONFIG]")
        try:
            sftp.mkdir(self.remote_path_finder.get_path("[DEVICE_FOLDER]"))
        except IOError:
            pass

        # Send general config
        sftp.put(
            self.local_path_finder.get_path("[DEVICE_CONFIG]"),
            self.remote_path_finder.get_path("[DEVICE_CONFIG]")
        )

        ssh.close()

    def upload(self):
        self.sync_config()

        ssh = self.ssh_manager.get_ssh()
        sftp = ssh.open_sftp()

        print "Sending detector settings file to remote:"
        print "from "+self.local_path_finder.get_path("[DETECTOR_CONFIG]") \
              + " to " + self.remote_path_finder.get_path("[DETECTOR_CONFIG]")
        try:
            sftp.put(
                self.local_path_finder.get_path("[DETECTOR_CONFIG]"),
                self.remote_path_finder.get_path("[DETECTOR_CONFIG]")
            )
        except OSError:
            print "File does not exist"

        print "Sending calibration files to remote:"
        print "from "+self.local_path_finder.get_path("[INTRINSIC_CALIBRATION]") \
              + " to " + self.remote_path_finder.get_path("[INTRINSIC_CALIBRATION]")
        try:
            sftp.put(
                self.local_path_finder.get_path("[INTRINSIC_CALIBRATION]"),
                self.remote_path_finder.get_path("[INTRINSIC_CALIBRATION]")
            )
        except OSError:
            print "File does not exist"

        print "from "+self.local_path_finder.get_path("[EXTRINSIC_CALIBRATION]") \
              + " to " + self.remote_path_finder.get_path("[EXTRINSIC_CALIBRATION]")

        try:
            sftp.put(
                self.local_path_finder.get_path("[EXTRINSIC_CALIBRATION]"),
                self.remote_path_finder.get_path("[EXTRINSIC_CALIBRATION]")
            )
        except OSError:
            print "File does not exist"

        ssh.close()
