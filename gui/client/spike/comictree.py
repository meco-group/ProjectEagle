#!/usr/bin/env python

from PyQt4 import QtGui, QtCore
import sys

gui = {}

class MainWindow(QtGui.QMainWindow):
    def __init__(self, *args):
        QtGui.QMainWindow.__init__(self)

        gui['browser'] = QtGui.QTextBrowser()
        gui['mainHBox'].addWidget(gui['browser'])


def mainGui():
    gui['mainWindow'] = QtGui.QMainWindow()
    QtGui.QMainWindow.__init__(gui['mainWindow'])
    gui['mainWindow'].resize(640, 480)
    gui['mainWindow'].setWindowTitle('mainwindow')

    setupMenubar()  # Add the menuBar
    gui['mainHBox'] = QtGui.QHBoxLayout()

    setupComicTree()  # Add the ComicTree

    gui['browser'] = QtGui.QTextBrowser()
    gui['mainHBox'].addWidget(gui['browser'])

    gui['centralWidget'] = QtGui.QWidget()
    gui['centralWidget'].setLayout(gui['mainHBox'])
    gui['mainWindow'].setCentralWidget(gui['centralWidget'])
    gui['mainWindow'].show()
    sys.exit(app.exec_())


def setupMenubar():
    gui['menubar'] = {}
    gui['menubar']['self'] = gui['mainWindow'].menuBar()
    gui['menubar']['file'] = gui['menubar']['self'].addMenu('&File')
    exit = QtGui.QAction(QtGui.QIcon('icons/exit.png'), 'Exit', gui['mainWindow'])
    exit.setShortcut('Ctrl+Q')
    exit.setStatusTip('Exit application')
    gui['mainWindow'].connect(exit, QtCore.SIGNAL('triggered()'), QtCore.SLOT('close()'))
    gui['menubar']['file'].addAction(exit)


def setupComicTree():
    gui['comicTree'] = QtGui.QTreeWidget()
    gui['comicTree'].setColumnCount(2)
    labels = QtCore.QStringList()
    labels.append('Name')
    labels.append('Date')
    gui['comicTree'].setHeaderLabels(QtCore.QStringList(labels))

    gui['comicTree'].ListOfComics = {}
    gui['comicTree'].comics = {}
    for comicList in ["hello", "number", "oh noes"]:
        item = QtGui.QTreeWidgetItem(gui['comicTree'])
        item.setText(0, comicList[0])
        item.setText(2, '0|' + comicList[0])

        gui['comicTree'].ListOfComics[comicList[0]] = item
        gui['comicTree'].comics[comicList[0]] = []
        retVar = [["fwwfsdfsdf"], ["fdsfewsf"], ["gsgwresdf"]]
        for curComic in retVar:
            item = QtGui.QTreeWidgetItem(gui['comicTree'].ListOfComics[comicList[0]])
            item.setText(0, curComic[0])
            item.setText(1,"placeholder")
            item.setText(2, '1|' + comicList[0] + '|' + str(len(gui['comicTree'].comics[comicList[0]])))
            gui['comicTree'].comics[comicList[0]].append(item)
    gui['mainHBox'].addWidget(gui['comicTree'])
    # Signals
    gui['comicTree'].itemDoubleClicked.connect(testFunc)
    gui['comicTree'].itemClicked.connect(testFunc2)
    gui['comicTree'].setContextMenuPolicy(QtCore.Qt.CustomContextMenu)
    gui['comicTree'].customContextMenuRequested.connect(testFunc3)


def testFunc(value):
    print 'DoubleClicked:'
    print value.text(2)


def testFunc2(value):
    print 'SingleClicked:'
    print value.text(2)


def testFunc3(value):
    print 'RightClicked:'
    print value


app = QtGui.QApplication(sys.argv)
mainGui()
# app.connect(app, SIGNAL("lastWindowClosed()"),app, SLOT("quit()"))