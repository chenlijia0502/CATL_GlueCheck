from PyQt5 import QtWidgets, QtGui, QtCore


class CInputDialog(QtWidgets.QDialog):
    def __init__(self):
        super(CInputDialog, self).__init__()
        self.setWindowTitle("调机权限")
        self._initui()


    def _initui(self):
        self.lineedit = QtWidgets.QLineEdit(self)
        self.label = QtWidgets.QLabel(self)
        font = QtGui.QFont()
        font.setPointSizeF(20)
        font.setBold(True)
        self.label.setFont(font)
        self.label.setText("输入调机账号: ")
        self.horlayout = QtWidgets.QHBoxLayout(self)
        self.horlayout.addWidget(self.label, 1)
        self.horlayout.addWidget(self.lineedit, 1)


    def gettext(self):
        return self.lineedit.text()

    def keyPressEvent(self, a0: QtGui.QKeyEvent):
        if a0.key() == QtCore.Qt.Key_Return:
            self.close()


if __name__ == "__main__":
    a = QtWidgets.QApplication([])
    w = CInputDialog()
    w.show()
    a.exec_()