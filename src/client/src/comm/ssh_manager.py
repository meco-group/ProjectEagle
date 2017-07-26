import os

import time
from paramiko import SSHClient, AutoAddPolicy, SSHException


class SSHManager:
    class SSHProcess:
        OUTPUT_PRINT = 20

        def __init__(self, name, ex_command, *args):
            self.name = name
            self.pid = ""

            self.running = False

            self.__monitor = SSHClient()
            self.__control = SSHClient()

            self.__monitor.set_missing_host_key_policy(AutoAddPolicy())
            self.__control.set_missing_host_key_policy(AutoAddPolicy())

            # Generate the command to execute
            self.command = ex_command
            for arg in args:
                self.command += " "+arg
            self.command += " & echo $! &"

        def connect(self, **kwargs):
            if self.running:
                return

            # TODO timeout
            self.__monitor.connect(**kwargs)
            self.__control.connect(**kwargs)

            print "Starting process "+self.name+" ..."
            print "with command: "+self.command
            _, stdout, _ = self.__control.exec_command(self.command)
            self.pid = str(int(stdout.readline()))
            # TODO log more process output
            print "PID of the remote process: " + self.pid
            for i in range(0, self.OUTPUT_PRINT):
                line = stdout.readline()
                if line == "":
                    break
                print str(i)+"\t["+self.name+"] "+line.replace('\n', ' ').replace('\r', '')

            self.running = True

        def is_active(self):
            if not self.running:
                return False

            try:
                _, stdout, _ = self.__monitor.exec_command("ps -p %s" % self.pid)
                res = stdout.read()
                res = self.pid in res
                return res
            except EOFError:
                return False
            except SSHException:
                return False

        def wait_until_ready(self):
            while self.is_active():
                time.sleep(.01)

        def close(self):
            if not self.running:
                return

            self.running = False
            print "Closing process "+self.name+" ..."

            # Kill the process
            self.__control.exec_command("kill -9 %s" % self.pid)
            self.wait_until_ready()

            self.__control.close()
            self.__monitor.close()

            print "Closed "+self.name

    def __init__(self, hostname, username=None, password=None):
        self.stack = {}

        self.hostname = hostname
        self.username = username
        self.password = password

    def close(self):
        for process in self.stack.values():
            process.close()

    def start_process(self, name, ex_command, *args):
        # Get the connection
        new_process = SSHManager.SSHProcess(name, ex_command, *args)
        self.stack[new_process.name] = new_process

        # Connect the process over ssh
        new_process.connect(hostname=self.hostname, username=self.username, password=self.password)

    def wait_for_process(self, name):
        if self.stack.has_key(name):
            self.stack[name].wait_until_ready()

    def is_active(self, name):
        return self.stack[name].is_active()

    def end_process(self, name):
        if self.stack.has_key(name):
            self.stack[name].close()

            if self.stack.has_key(name):
                del self.stack[name]

    def get_ssh(self):
        ssh = SSHClient()

        # Connect the process over ssh
        ssh.set_missing_host_key_policy(AutoAddPolicy())
        ssh.connect(hostname=self.hostname, username=self.username, password=self.password)

        return ssh



