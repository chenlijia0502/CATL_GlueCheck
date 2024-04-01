from PyQt5 import Qt, QtWidgets, QtGui


class CDebugWidget(QtWidgets.QWidget):
    def __init__(self, h_parent):
        super(CDebugWidget, self).__init__()
        self.hparent = h_parent
        self._initui()
        self.pushbutton1.clicked.connect(self.sendagvmsg1)
        self.pushbutton2.clicked.connect(self.sendagvmsg2)


    def _initui(self):
        self.verlayout = QtWidgets.QVBoxLayout(self)
        self.pushbutton1 = QtWidgets.QPushButton(self)
        self.pushbutton2 = QtWidgets.QPushButton(self)
        self.verlayout.addWidget(self.pushbutton1)
        self.verlayout.addWidget(self.pushbutton2)
        font = QtGui.QFont()
        font.setFamily("Arial")
        font.setPointSize(18)
        self.pushbutton1.setFont(font)
        self.pushbutton2.setFont(font)
        self.pushbutton1.setMaximumWidth(600)
        self.pushbutton2.setMaximumWidth(600)
        self.pushbutton1.setMinimumHeight(200)
        self.pushbutton2.setMinimumHeight(200)
        self.pushbutton1.setText("放小车进站")
        self.pushbutton2.setText("放小车出站")


    def sendagvmsg1(self):
        self.hparent.sendagvmsg(0)


    def sendagvmsg2(self):
        self.hparent.sendagvmsg(1)
