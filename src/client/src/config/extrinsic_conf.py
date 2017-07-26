import os


class ExtrinsicConfig:
    def __init__(self):
        pass

    def write(self, target):
        with open(target, 'a'):
            os.utime(target, None)
