import socket
import time
from paramiko import SSHClient, AutoAddPolicy, SSHException


class SSHManager:
    class SSHProcess:
        OUTPUT_PRINT = 20

        def __init__(self, name, command, *args):
            self.name = name
            self.pid = ''

            self.running = False
            self.started = False

            self.__monitor = SSHClient()
            self.__control = SSHClient()

            self.__monitor.set_missing_host_key_policy(AutoAddPolicy())
            self.__control.set_missing_host_key_policy(AutoAddPolicy())

            # Generate the command to execute
            self.command = command
            for arg in args:
                self.command += " "+arg
            self.command += " & echo $! &"

        def start(self, hostname, username, password, timeout=0.5):
            if self.running:
                return
            try:
                self.__monitor.connect(hostname=hostname, username=username, password=password, timeout=timeout)
                self.__control.connect(hostname=hostname, username=username, password=password, timeout=timeout)
                _, stdout, stderr = self.__control.exec_command(self.command)
            except socket.timeout:
                return False
            self.pid = str(int(stdout.readline()))
            print 'Started process ' + self.name + ' [' + self.pid + ']' + ' on ' + username + '@' + hostname + '.'
            for i in range(self.OUTPUT_PRINT):
                line = stdout.readline()
                if line == '':
                    break
                print str(i)+"\t["+self.name+"] "+line.replace('\n', ' ').replace('\r', '')
            self.running = True
            self.started = True
            return True

        def is_active(self):
            if not self.running:
                return False
            try:
                _, stdout, _ = self.__monitor.exec_command('ps -p %s' % self.pid)
                res = stdout.read()
                res = self.pid in res
                return res
            except EOFError:
                return False
            except SSHException:
                return False

        def wait_until_started(self, timeout=None):
            t0 = time.time()
            while not self.started:
                if (timeout is not None and time.time()-t0 > timeout):
                    return False
                time.sleep(.01)
            return True

        def wait_until_finished(self, timeout=None):
            t0 = time.time()
            while self.is_active():
                if (timeout is not None and time.time()-t0 > timeout):
                    return False
                time.sleep(.01)
            return True

        def stop(self):
            if not self.wait_until_started(timeout=1):
                return
            if not self.running:
                return
            print 'Closing process ' + self.name + ' ...',
            # kill the process
            self.__control.exec_command("kill -9 %s" % self.pid)
            if not self.wait_until_finished(timeout=3):
                print 'failed.'
            else:
                print 'done.'
            self.running = False
            self.__control.close()
            self.__monitor.close()

    def __init__(self, hostname, username, password):
        self.processes = {}
        self.hostname = hostname
        self.username = username
        self.password = password

    def close(self):
        for process in self.processes.values():
            process.close()

    def start_process(self, name, command, *args):
        process = SSHManager.SSHProcess(name, command, *args)
        self.processes[process.name] = process
        return process.start(self.hostname, self.username, self.password)

    def wait_for_process(self, name):
        if name in self.processes:
            self.processes[name].wait_until_ready()

    def is_active(self, name):
        return self.processes[name].is_active()

    def end_process(self, name):
        if name in self.processes:
            self.processes[name].stop()
            if name in self.processes:
                del self.processes[name]

    def get_ssh(self):
        ssh = SSHClient()
        # Connect the process over ssh
        ssh.set_missing_host_key_policy(AutoAddPolicy())
        ssh.connect(hostname=self.hostname, username=self.username, password=self.password, timeout=0.5)
        return ssh
