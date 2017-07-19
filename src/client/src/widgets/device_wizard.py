from PyQt4 import QtGui


class DeviceWizard(QtGui.QDialog):
    def __init__(self, parent, device_manager, device=None):
        super(DeviceWizard, self).__init__(parent)
        self.resize(360, 120)
        self.setWindowTitle('Add Device')

        self.device = device

        self.deviceManager = device_manager

        self.grid = QtGui.QWidget(self)

        grid_layout = QtGui.QGridLayout(self)
        grid_layout.setColumnStretch(1, 4)
        grid_layout.setColumnStretch(2, 4)

        self.name_label = QtGui.QLabel(self)
        self.ip_label = QtGui.QLabel(self)
        self.user_label = QtGui.QLabel(self)
        self.pass_label = QtGui.QLabel(self)

        self.name_label.setText("Name:")
        self.ip_label.setText("Ip:")
        self.user_label.setText("Username:")
        self.pass_label.setText("Password:")

        self.name_field = QtGui.QLineEdit(self)
        self.ip_field = QtGui.QLineEdit(self)
        self.user_field = QtGui.QLineEdit(self)
        self.pass_field = QtGui.QLineEdit(self)
        self.pass_field.setEchoMode(QtGui.QLineEdit.Password)

        if device is not None:
            self.name_field.setText(device.name)
            self.ip_field.setText(device.ip)
            self.user_field.setText(device.username)
            self.pass_field.setText(device.password)

        grid_layout.addWidget(self.name_label, 0, 0)
        grid_layout.addWidget(self.ip_label, 1, 0)
        grid_layout.addWidget(self.user_label, 2, 0)
        grid_layout.addWidget(self.pass_label, 3, 0)
        grid_layout.addWidget(self.name_field, 0, 1)
        grid_layout.addWidget(self.ip_field, 1, 1)
        grid_layout.addWidget(self.user_field, 2, 1)
        grid_layout.addWidget(self.pass_field, 3, 1)

        grid_layout.setColumnStretch(1, 10)
        grid_layout.setMargin(0)

        self.grid.setLayout(grid_layout)

        box = QtGui.QVBoxLayout()
        box.addWidget(self.grid)

        self.dialog = QtGui.QWidget(self)
        dialog_layout = QtGui.QHBoxLayout()

        self.confirm_button = QtGui.QPushButton(self)
        self.confirm_button.setText("Confirm")
        self.confirm_button.clicked.connect(self.confirm)

        self.cancel_button = QtGui.QPushButton(self)
        self.cancel_button.setText("Cancel")
        self.cancel_button.clicked.connect(self.cancel)

        dialog_layout.addWidget(self.confirm_button)
        dialog_layout.addWidget(self.cancel_button)

        self.dialog.setLayout(dialog_layout)

        box.addWidget(self.dialog)

        self.setLayout(box)

        self.setModal(True)
        self.show()

    def confirm(self):
        if self.device is None:
            # add the device
            self.deviceManager.add_device(
                str(self.name_field.text()),
                str(self.ip_field.text()),
                str(self.user_field.text()),
                str(self.pass_field.text())
            )
        else:
            self.device.set_name(str(self.name_field.text()))
            self.device.set_ip(str(self.ip_field.text()))
            self.device.username = str(self.user_field.text())
            self.device.password = str(self.pass_field.text())

        self.close()

    def cancel(self):
        self.close()