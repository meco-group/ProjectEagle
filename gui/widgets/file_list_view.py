from PyQt4 import QtGui
import os
from os import listdir
from os.path import isfile, join, abspath
import re


class FileListView(QtGui.QListView):
    def __init__(self, path, parent=None):
        super(FileListView, self).__init__(parent)
        self.path = os.path.realpath(path)
        self.setMinimumWidth(150)

        self.file_count = 0

        self.model = QtGui.QStandardItemModel(self)
        self.setModel(self.model)

        self.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)

        self.setSelectionMode(QtGui.QAbstractItemView.ExtendedSelection)

        self.read_files()

    def get_file_list(self):
        return [f for f in listdir(self.path) if isfile(join(self.path, f))]

    def read_files(self):
        self.model.clear()

        files = self.get_file_list()

        def cmp_file(x, y):
            # look for all groups of integers in x
            x_ints = re.findall(r'\d+', x)
            y_ints = re.findall(r'\d+', y)
            if len(x_ints) == 0:
                return -1
            elif len(y_ints) == 0:
                return 1
            elif len(x_ints) == 0 and len(y_ints) == 0:
                return 0
            else:
                return cmp(int(x_ints[-1]), int(y_ints[-1]))

        files.sort(cmp=cmp_file)
        for f in files:
            item = QtGui.QStandardItem(f)
            self.model.appendRow(item)

        if len(files) == 0:
            self.file_count = 0
        else:
            ints = re.findall(r'\d+', files[-1])
            if len(ints) == 0:
                self.file_count = len(files)
            else:
                self.file_count = int(ints[-1])+1

    def get_file_count(self):
        return self.file_count

    def get_selected(self, root=None):
        if root is None:
            root = self.path

        if len(self.selectionModel().selectedIndexes()) > 0:
            return [
                abspath(join(
                    root,
                    str(self.model.itemFromIndex(f).text())
                )) for f in self.selectionModel().selectedIndexes()
            ]
        else:
            return [
                abspath(join(
                    root,
                    str(self.model.item(index).text())
                )) for index in range(self.model.rowCount())
            ]
