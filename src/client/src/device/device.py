import paramiko
import time

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

        # initialise ssh
        self.__ssh = paramiko.SSHClient()

    def send_config(self, src, dest):
        sftp = self.__ssh.open_sftp()
        sftp.put(src, dest)

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

    def disconnect(self):
        self.connected = False
        self.treeItem.set_connected(self.connected)
        self.__ssh.close()

    def start_image_stream(self):
        assert(self.is_connected())

        print "Starting image transmitter"

        stdin, stdout, stderr = self.__ssh.exec_command(
            "/home/pi/ProjectEagle/build/bin/ImageTransmitter /home/pi/ProjectEagle/config/ceil2_cam.xml")

        data = stdout.read(200).splitlines()
        for line in data:
            print "    [ImageTransmitter] "+line
        print "    [ImageTransmitter] " + "..."

    def stop_image_stream(self):
        self.__ssh.exec_command("pkill ImageTransmitter")

    def take_snapshot(self):
        self.__ssh.exec_command("pkill ImageTransmitter")





# dev = Device("ceil2", "192.168.11.133", "pi", "raspberry")
# dev.connect()
# dev.send_config("/home/peter/Documents/Honours/ProjectEagle/src/client/src/test.xml", "/home/pi/test.xml")
