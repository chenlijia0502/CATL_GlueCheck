from PyQt5 import QtWidgets, QtCore, QtGui


class CWidgetShowResultSFC(QtWidgets.QWidget):
    def __init__(self):
        super(CWidgetShowResultSFC, self).__init__()
        self.__initui()


    def __initui(self):
        widget1 = QtWidgets.QWidget(self)
        label1 = QtWidgets.QLabel(widget1)
        font = QtGui.QFont()
        font.setFamily("Arial")
        font.setPointSize(15)
        label1.setFont(font)
        label1.setText("SFC模组号")
        label1.setAlignment(QtCore.Qt.AlignCenter)
        font.setPointSize(20)
        self.label_sfc = QtWidgets.QLabel(widget1)
        self.label_sfc.setFont(font)
        verlayout = QtWidgets.QVBoxLayout(widget1)
        verlayout.setContentsMargins(0, 0, 0, 0)
        verlayout.setSpacing(0)
        verlayout.addWidget(label1)
        verlayout.addWidget(self.label_sfc)
        font.setPointSize(100)
        self.label_ngok = QtWidgets.QLabel(self)
        self.label_ngok.setFont(font)
        self.label_ngok.setAlignment(QtCore.Qt.AlignCenter)
        self.horlayout = QtWidgets.QHBoxLayout(self)
        self.horlayout.setContentsMargins(0, 0, 0, 0)
        self.horlayout.setSpacing(0)
        self.setStyleSheet("border: 1px solid black")
        self.horlayout.addWidget(widget1, 1)
        self.horlayout.addWidget(self.label_ngok, 1)


    def clear(self):
        self.label_ngok.setText("")
        self.label_sfc.setText("")

    def setsfc(self, sfc):
        self.label_sfc.setText(sfc)

    def setresult(self, bstatus):
        if bstatus:
            self.label_ngok.setStyleSheet("color:rgb(0, 255, 0)")
            self.label_ngok.setText("OK")
        else:
            self.label_ngok.setStyleSheet("color:rgb(255, 0, 0)")
            self.label_ngok.setText("NG")

if __name__ == "__main__":
    a = QtWidgets.QApplication([])
    w = CWidgetShowResultSFC()
    w.setsfc("00000000000000EEEEEEEEEEEAAAAA")
    w.setresult(0)
    w.show()
    a.exec_()
