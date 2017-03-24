import struct
import pyre

n = pyre.Pyre('EAGLE')
n.join('mygroup')
n.start()

message = n.recv()
while(message[0] != 'SHOUT'):
    message = n.recv()

data = message[4]
offset = 0
while (offset < len(data)):
    size = struct.unpack('Q', data[offset:offset+8])[0]
    offset += 8
    data_int = struct.unpack('i', data[offset:offset+size])[0]
    offset += size
    print data_int

n.stop()
