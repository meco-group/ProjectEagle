from PyQt4 import QtGui, QtCore

from src.widgets.image_stream import ImageStream


class CalibrateWizard(QtGui.QDialog):
    def __init__(self, parent, device):
        super(CalibrateWizard, self).__init__(parent)

        self.device = device

        self.resize(640, 480)
        self.setWindowTitle('Calibrate Device')

        self.content = QtGui.QVBoxLayout(self)

        self.image_stream = ImageStream(self, device)
        self.content.addWidget(self.image_stream)

        self.setLayout(self.content)

        self.setModal(True)
        self.show()

        # Start image stream
        QtCore.QTimer.singleShot(0, self.image_stream.start)

    def closeEvent(self, QCloseEvent):
        self.image_stream.stop()
        super(CalibrateWizard, self).closeEvent(QCloseEvent)