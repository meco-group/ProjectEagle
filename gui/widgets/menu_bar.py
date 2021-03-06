from PyQt4 import QtGui


class MenuBar(QtGui.QMenuBar):

    def __init__(self, parent, device_manager=None):
        super(MenuBar, self).__init__(parent)

        self.file = self.addMenu('&File')

        self.exit = QtGui.QAction(QtGui.QIcon(
            'icons/exit.png'), 'Exit', self.parent())
        self.exit.setShortcut('Ctrl+Q')
        self.exit.setStatusTip('Exit application')

        self.save = QtGui.QAction(QtGui.QIcon(
            'icons/save.png'), 'Save', self.parent())
        self.save.setShortcut('Ctrl+S')
        self.save.setStatusTip('Save devices')

        self.save_as = QtGui.QAction(QtGui.QIcon(
            'icons/save.png'), 'Save As', self.parent())
        self.save_as.setShortcut('Ctrl+Alt+S')
        self.save_as.setStatusTip('Save devices in a custom location')

        self.load = QtGui.QAction(QtGui.QIcon(
            'icons/open.png'), 'Open', self.parent())
        self.load.setShortcut('Ctrl+O')
        self.load.setStatusTip('Open devices')

        self.file.addAction(self.exit)
        self.file.addAction(self.save_as)
        self.file.addAction(self.save)
        self.file.addAction(self.load)
