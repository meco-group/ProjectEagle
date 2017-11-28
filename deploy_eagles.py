#!/usr/bin/python

import subprocess
import optparse
import os
import paramiko
import xml.etree.ElementTree as et
import collections as col
import errno
import sys
import math
import socket

# parameters
user = 'odroid'
password = user
remote_root = os.path.join('/home/' + user, 'ProjectEagle/eagle/build')
current_dir = os.path.dirname(os.path.realpath(__file__))
local_root = os.path.join(current_dir, 'eagle/build')

hosts = ['eagle0', 'eagle1']
addresses = col.OrderedDict([('eagle0', '192.168.11.139'), ('eagle1', '192.168.11.123')])


def send_file(ftp, ssh, loc_file, rem_file):
    directory = os.path.dirname(rem_file)
    try:
        ftp.lstat(directory)
    except IOError, e:
        if e.errno == errno.ENOENT:
            stdin, stdout, stderr = ssh.exec_command('mkdir ' + directory)
    try:
        ftp.put(loc_file, rem_file)
    except:
        raise ValueError('Could not send ' + loc_file)


def send_files(ftp, ssh, loc_files, rem_files, fancy_print=False):
    n_blocks = len(loc_files)
    interval = int(math.ceil(len(loc_files)/n_blocks*1.))
    if fancy_print:
        string = '['
        for k in range(len(loc_files)/interval):
            string += ' '
        string += ']'
        cnt = 0
    for lf, rf in zip(loc_files, rem_files):
        send_file(ftp, ssh, lf, rf)
        if fancy_print:
            string2 = string
            for k in range(cnt/interval):
                string2 = string2[:k+1] + '=' + string2[k+2:]
            sys.stdout.flush()
            sys.stdout.write("\r"+string2)
            cnt += 1
    if fancy_print:
        print ''


def deploy(hosts):
    if len(hosts) == 0:
        return
    command = ['gnome-terminal']
    for host in hosts:
        address = addresses[host]
        command.extend(['--tab', '-e', '''
            bash -c '
            sshpass -p %s ssh %s@%s "
            killall -9 EagleTransmitter
            cd %s
            echo I am %s
            ./bin/EagleTransmitter %s
            "'
            ''' % (password, user, address, remote_root, host, host)
        ])
    subprocess.call(command)

if __name__ == "__main__":
    usage = ("Usage: %prog [options]")
    op = optparse.OptionParser(usage=usage)
    deploy(hosts)
