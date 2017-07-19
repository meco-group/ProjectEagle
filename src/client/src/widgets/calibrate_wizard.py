from PyQt4 import QtGui, QtCore

from src.widgets.image_stream import ImageStream


class CalibrateWizard(QtGui.QDialog):
    def __init__(self, parent, device):
        super(CalibrateWizard, self).__init__(parent)

        self.device = device

        self.resize(640, 480)
        self.setWindowTitle('Calibrate Device')

        self.grid = QtGui.QWidget(self)
        grid_layout = QtGui.QGridLayout(self)

        self.image_stream = ImageStream(self, device)
        grid_layout.addWidget(self.image_stream, 0, 0)

        self.grid.setLayout(grid_layout)

        box = QtGui.QVBoxLayout()
        box.addWidget(self.grid)

        self.setLayout(box)

        self.setModal(True)
        self.show()

        # Start image stream
        QtCore.QTimer.singleShot(0, self.image_stream.start)