# running_client.py

import socket                   # Import socket module
import cv2
import numpy
import struct
import time

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)             # Create a socket object
host = socket.gethostname()     # Get local machine name
port = 60000                    # Reserve a port for your service.

s.bind(('', port))

wait_for_frame = False
frames_dropped = 0
frames_received = 0
current_frame_count = 0
frame_total = 0
last_chunk = -1
dropped = 0
frame_size = 0

begin = time.time()

while True:
	data, addr = s.recvfrom(port)
	if len(data) < 3:
		frame_total += 1
		if wait_for_frame:
			frames_dropped += 1
		wait_for_frame = True
		current_frame_count = 0
		last_chunk = -1
		frame_size = struct.unpack('h',data[0:2])[0]
		print 'Frame size: {0}'.format(frame_size)
	else:
		im = cv2.imdecode(numpy.fromstring(data, dtype=numpy.uint8),1)
		# print im
		if im is not None:
			#if current_frame_count == 0:
			frames_received += 1 
			current_frame_count += 1
			wait_for_frame = False
			cv2.imshow('capture',im)
			cv2.waitKey(1)
		else:
			print 'decoding failed'
		
	#print '{0}: {1} received, {2} dropped, {3} current count'.format(frame_total,frames_received,frames_dropped,current_frame_count)
	
	print '{0}: {1} received, {2} fps'.format(frame_total,frames_received,frames_received/(time.time()-begin))
	
	
f.close()
print('Successfully get the file')
s.close()
print('connection closed')
