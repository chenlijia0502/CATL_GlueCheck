

from PyQt5 import QtCore, QtGui, QtWidgets


class showwidget(QtWidgets.QWidget):
    def __init__(self):
        super(showwidget, self).__init__()
        self._initui()


    def _initui(self):
        self.pushbutton = QtWidgets.QPushButton(self)
        self.horlayout = QtWidgets.QHBoxLayout(self)
        self.horlayout.addWidget(self.pushbutton)
        self.pushbutton.setText("ceshi")
        self.pushbutton.clicked.connect(self.test)


    def test(self):
        msgbox = QtWidgets.QMessageBox
        msgbox.setMinimumWidth(300)
        msgbox.setMinimumHeight(300)
        msgbox.warning(self, u"错误", "！！！")



if __name__ == "__main__":
    a = QtWidgets.QApplication([])
    w = showwidget()
    w.show()
    a.exec_()