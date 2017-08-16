import time

import struct
from PyQt4 import QtGui

import cv2

import numpy as np
from pyre import Pyre
import zmq


class Communicator(Pyre):
    def __init__(self, name):
        super(Communicator, self).__init__(name)
        self.poller = zmq.Poller()
        self.poller.register(self.socket(), zmq.POLLIN)

    def wait_for_peers(self):
        while len(self.peers()) <= 0:
            time.sleep(1)

    def read(self):
        # Obtain information from camera.
        msg = self.recv()

        while msg[0] != 'SHOUT':
            msg = self.recv()

        # Initiate elements
        data = msg[4]
        peer = msg[2]
        offset = 0

        # Parse data
        while offset < len(data):
            # Get the size of the header
            header_size = struct.unpack('@I', data[offset:offset+4])[0]
            offset += 4

            # Parse the header
            h_id, h_time = struct.unpack('IL', data[offset:(offset + header_size)])
            offset += header_size

            # Get the size of the data
            data_size = struct.unpack('@I', data[offset:offset+4])[0]
            offset += 4

            # Get the data
            buffer = data[offset:offset+data_size]
            offset += data_size

            # Parse the data
            if h_id == 2:
                # Decode openCv
                nparr = np.fromstring(buffer, np.uint8)
                img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)

                return img, peer
            else:
                # TODO parser for other cases
                return None, None
