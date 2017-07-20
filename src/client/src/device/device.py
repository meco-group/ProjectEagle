import os

import paramiko
import time

from pathlib import Path

from src.widgets.core.device_tree import DeviceItem


class Device:
    def __init__(self, manager, id, ip, username, password):
        # store properties
        self.manager = manager
        self.name = id
        self.ip = ip
        self.username = username
        self.password = password
        self.treeItem = DeviceItem(device=self, parent=self.manager.deviceTree)
        self.connected = False
        self.image_stream_stdin = None

        # create root folder
        if not os.path.isdir(self.get_root()):
            os.mkdir(self.get_root())

        # initialise ssh
        self.__ssh = paramiko.SSHClient()

    def set_name(self, value):
        self.name = value
        self.treeItem.set_name(value)

    def set_ip(self, value):
        self.ip = value
        self.treeItem.set_ip(value)

    def is_connected(self):
        return self.connected

    def connect(self):
        # add missing host keys automatically TODO ask for permission from user
        self.__ssh.set_missing_host_key_policy(
            paramiko.AutoAddPolicy())

        # connect to the ssh server
        self.__ssh.connect(self.ip, username=self.username,
                    password=self.password)

        self.connected = True
        self.treeItem.set_connected(self.connected)

        # TODO do this somewhere else?
        self.sync_remote()

    def disconnect(self):
        self.connected = False
        self.treeItem.set_connected(self.connected)
        self.__ssh.close()

    def start_image_stream(self):
        assert(self.is_connected())

        print "Starting image transmitter"

        stdin, stdout, stderr = self.__ssh.exec_command(
            "/home/pi/ProjectEagle/build/bin/ImageTransmitter /home/pi/ProjectEagle/config/ceil2_cam.xml &")

        data = stdout.read(200).splitlines()
        for line in data:
            print "    [ImageTransmitter] "+line
        print "    [ImageTransmitter] " + "..."

    def stop_image_stream(self):
        self.__ssh.exec_command("pkill ImageTransmitter &")

    def take_snapshot(self, target_path):
        assert(self.is_connected())

        print "Taking snapshot "+target_path

        target = str(Path(self.get_conf_path()).relative_to(os.path.abspath("../../..")))
        target = os.path.join("/home/pi/ProjectEagle", target)
        stdin, stdout, stderr = self.__ssh.exec_command(
            "/home/pi/ProjectEagle/build/bin/Snapshot "+target+" /home/pi/ProjectEagle/snapshot.png &")

        data = stdout.read().splitlines()
        for line in data:
            print "    [Snapshot] "+line
        print "    [Snapshot] " + "..."

        print "Downloading snapshot"

        sftp = self.__ssh.open_sftp()
        sftp.get("/home/pi/ProjectEagle/snapshot.png", target_path)

        print "Done"

    def sync_remote(self):
        sftp = self.__ssh.open_sftp()

        # make sure the directory exists
        target = str(Path(self.get_root()).relative_to(os.path.abspath("../../..")))
        print os.path.join("/home/pi/ProjectEagle", target)
        try:
            sftp.mkdir(os.path.join("/home/pi/ProjectEagle", target))
        except IOError:
            pass

        # Send general config
        target = str(Path(self.get_conf_path()).relative_to(os.path.abspath("../../..")))
        print target
        sftp.put(
            self.get_conf_path(),
            os.path.join("/home/pi/ProjectEagle", target)
        )

    def send_cal_settings(self):
        sftp = self.__ssh.open_sftp()

        # make sure the directory exists
        target = str(Path(self.get_root()).relative_to(os.path.abspath("../../..")))
        print os.path.join("/home/pi/ProjectEagle", target)
        try:
            sftp.mkdir(os.path.join("/home/pi/ProjectEagle", target))
        except IOError:
            pass

        # Send general config
        target = str(Path(self.get_cal_path()).relative_to(os.path.abspath("../../..")))
        print target
        sftp.put(
            self.get_cal_path(),
            os.path.join("/home/pi/ProjectEagle", target)
        )

    def get_root(self):
        return os.path.abspath(os.path.join("../config/devices/", self.name))

    def get_snap_path(self):
        return os.path.abspath(os.path.join(self.get_root(), 'intrinsic_snaps'))

    def get_snap_conf_path(self):
        return os.path.abspath(os.path.join(self.get_root(), 'images.xml'))

    def get_cal_path(self):
        return os.path.abspath(os.path.join(self.get_root(), 'cal.xml'))

    def get_cal_conf_path(self):
        return os.path.abspath(os.path.join(self.get_root(), 'cal_conf.xml'))

    def get_conf_path(self):
        return os.path.abspath(os.path.join(self.get_root(), 'conf.xml'))






# dev = Device("ceil2", "192.168.11.133", "pi", "raspberry")
# dev.connect()
# dev.send_config("/home/peter/Documents/Honours/ProjectEagle/src/client/src/test.xml", "/home/pi/test.xml")
