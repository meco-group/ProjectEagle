from PyQt4 import QtGui, QtCore


class MainWindow(QtGui.QMainWindow):
    close_trigger = QtCore.pyqtSignal()

    def __init__(self, parent=None):
        super(MainWindow, self).__init__(parent)

    def closeEvent(self, QCloseEvent):
        self.close_trigger.emit()
        super(MainWindow, self).closeEvent(QCloseEvent)
