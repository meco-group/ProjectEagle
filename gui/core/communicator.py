import time
import struct
import cv2
import numpy as np
from pyre import Pyre
import zmq
import logging


class Communicator(Pyre):
    def __init__(self, name):
        super(Communicator, self).__init__(name)
        self.poller = zmq.Poller()
        self.poller.register(self.socket(), zmq.POLLIN)
        logging.getLogger("pyre.pyre").setLevel(logging.ERROR)

    def wait_for_peer(self, peer, timeout=None):
        t0 = time.time()
        while True:
            if (timeout is not None and time.time()-t0 > timeout):
                return False
            for p in self.peers():
                if self.get_peer_name(p) == peer:
                    return True
            time.sleep(0.01)
        return True

    def rcv_img(self):
        msg = self.recv()
        while msg[0] != 'SHOUT':
            msg = self.recv()
        data = msg[4]
        peer = msg[2]
        offset = 0
        # parse data
        while offset < len(data):
            # get the size of the header
            header_size = struct.unpack('@I', data[offset:offset+4])[0]
            offset += 4
            # parse the header
            h_id, h_time = struct.unpack('IL', data[offset:(offset + header_size)])
            offset += header_size
            # get the size of the data
            data_size = struct.unpack('@I', data[offset:offset+4])[0]
            offset += 4
            # get the data
            buf = data[offset:offset+data_size]
            offset += data_size
            # parse the data
            if h_id == 2:
                # decode openCv
                nparr = np.fromstring(buf, np.uint8)
                img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
                return img, peer
            else:
                return None, None

    def send_cmd(self, cmd, peer):
        h_id = 3
        h_time = 0
        msg = struct.pack('IIIII', 4+4, h_id, h_time, 4, cmd)
        peers = self.peers()
        peer_uuid = None
        for p in peers:
            if self.get_peer_name(p) == peer:
                peer_uuid = p
                break
        if peer_uuid is not None:
            self.whisper(peer_uuid, msg)
