import paramiko

# initialise ssh client
ssh = paramiko.SSHClient()

# add missing host keys automatically
ssh.set_missing_host_key_policy(
    paramiko.AutoAddPolicy())

# connect to the ssh server
ssh.connect('192.168.11.133', username='pi',
            password='raspberry')

# execute the command uptime
# we receive three different buffers
stdin, stdout, stderr = ssh.exec_command(
    "ls")
data = stdout.read().splitlines()
print data
for line in data:
    if line.split(':')[0] == 'AirPort':
        print line