from PyQt5 import QtGui, QtWidgets, Qt, QtCore

import serial

class CHardwareIOwidget(QtWidgets.QWidget):
    def __init__(self):
        super(CHardwareIOwidget, self).__init__()

        self.verlayout = QtWidgets.QVBoxLayout(self)
        widget1 = self.createsingle()
        widget2 = self.createsingle()
        widget3 = self.createsingle()
        self.verlayout.addWidget(widget1)
        self.verlayout.addWidget(widget2)
        self.verlayout.addWidget(widget3)



    def createsingle(self):
        widget =  QtWidgets.QWidget()
        verlayout = QtWidgets.QHBoxLayout(widget)
        pushbutton = QtWidgets.QPushButton()
        box = QtWidgets.QCheckBox()
        verlayout.addWidget(pushbutton)
        verlayout.addWidget(box)
        return widget



if __name__ == "__main__":
    a = QtWidgets.QApplication([])

    w = CHardwareIOwidget()

    w.show()

    a.exec_()