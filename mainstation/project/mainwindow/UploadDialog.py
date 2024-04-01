from PyQt5 import Qt, QtGui,QtCore, QtWidgets
import time


class CUploadDialog(QtWidgets.QDialog):
    """
    上传窗口
    """
    def __init__(self):
        super(CUploadDialog, self).__init__()
        self.__initui()
        self.setMinimumHeight(200)
        # 定时器，确保对键盘输入进行定时清理
        self.text = ""
        self.curuser = ""
        self.nstatus = 0 #1为放行， 0为判废
        self.setWindowFlags(QtCore.Qt.FramelessWindowHint | QtCore.Qt.WindowStaysOnTopHint)
        self.nrecordtime = time.time()
        self.timer = QtCore.QTimer(self)
        self.timer.timeout.connect(self.__timeout2clear)
        self.timer.start(1000)


    def __timeout2clear(self):
        """
        键盘输入的时候会记录一个输入时间self.nrecordtime，当键盘在连续输入的text不会被清零
        因为读卡器输入的时候是连续的。加入这个可以将键盘输入的影响减小
        :return:
        """
        curtime = time.time()
        if curtime - self.nrecordtime > 1:
            self.text = ""



    def __initui(self):
        self.widget1 = QtWidgets.QWidget()

        self.horlayout1 = QtWidgets.QHBoxLayout(self.widget1)

        label = QtWidgets.QLabel(self.widget1)
        label.setText("输入卡号： ")
        self.lineEdit = QtWidgets.QLineEdit(self.widget1)
        self.lineEdit.setReadOnly(True)
        self.lineEdit.setEchoMode(QtWidgets.QLineEdit.Password)

        self.horlayout1.addWidget(label)
        self.horlayout1.addWidget(self.lineEdit)

        self.widget2 = QtWidgets.QWidget()
        self.horlayout2 = QtWidgets.QHBoxLayout(self.widget2)
        self.pushbutton1 = QtWidgets.QPushButton()
        self.pushbutton1.setText("OK放行")
        self.pushbutton2 = QtWidgets.QPushButton()
        self.pushbutton2.setText("NG")
        # self.horlayout2.addWidget(self.pushbutton1)
        # self.horlayout2.addWidget(self.pushbutton2)
        self.buttonbox = QtWidgets.QDialogButtonBox(self)

        self.buttonbox.addButton(self.pushbutton1,QtWidgets.QDialogButtonBox.YesRole)
        self.buttonbox.addButton(self.pushbutton2,QtWidgets.QDialogButtonBox.NoRole)
        self.buttonbox.clicked.connect(self._slot_buttonclick)

        self.horlayout2.addWidget(self.buttonbox)

        self.verlayout = QtWidgets.QVBoxLayout(self)

        self.labeltext = QtWidgets.QLabel(self)
        self.labeltext.setStyleSheet("font: 16pt \"Arial\";")
        self.labeltext.setText("！！！当前检测为废品，请选择是否手动放行！！！")
        self.verlayout.addWidget(self.labeltext)
        self.verlayout.addWidget(self.widget1)
        self.verlayout.addWidget(self.widget2)


    def _slot_buttonclick(self, button):
        if self.lineEdit.text() == "":
            return
        if button == self.pushbutton1:
            self.nstatus = 1
        else:
            self.nstatus = 0
        self.curuser = self.lineEdit.text()
        self.lineEdit.clear()
        self.close()


    def keyPressEvent(self, a0: QtGui.QKeyEvent):
        self.nrecordtime = time.time()
        if a0.key() == QtCore.Qt.Key_Return:
            self.lineEdit.setText(self.text)
            self.text = ""
        else:
            self.text = self.text + a0.text()

    def getstatusAnduser(self):
        nstatus = self.nstatus
        curuser = self.curuser
        self.nstatus = 0
        self.curuser = ""
        return nstatus, curuser


if __name__ == "__main__":
    a = QtWidgets.QApplication([])

    w = CUploadDialog()

    w.show()

    a.exec_()