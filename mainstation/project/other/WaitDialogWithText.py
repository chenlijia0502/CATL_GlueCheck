'''
Created on 2017年07月10日

@author: huber yao
'''
import time

from PyQt5 import QtWidgets, QtCore

class WaitDialogWithText(QtWidgets.QDialog):
    def __init__(self, title='正在建模，请稍候...', parent=None):
        super(WaitDialogWithText, self).__init__(parent)
        self.setFixedSize(QtCore.QSize(600, 150))
        #self.setAttribute(QtCore.Qt.WA_DeleteOnClose)
        self.setWindowFlags(QtCore.Qt.FramelessWindowHint | QtCore.Qt.WindowStaysOnTopHint)
        #self.setWindowModality(QtCore.Qt.ApplicationModal)
        self.setStyleSheet('background-color:rgb(40,143,218)')
        self.tipsLabel = QtWidgets.QLabel(title)
        self.tipsLabel.setAlignment(QtCore.Qt.AlignCenter)
        self.tipsLabel.setStyleSheet('color:white;font-family:Consolas;font-size:38px;font:bold;')
        self.tipsLabel.setFixedHeight(120)
        self.processInfoLabel = QtWidgets.QLabel()
        self.processInfoLabel.setStyleSheet('color:white;font-family:Consolas;font-size:14px;')
        self.processBar = QtWidgets.QProgressBar()
        self.processBar.setValue(0)
        self.processBar.setAlignment(QtCore.Qt.AlignCenter)
        layout = QtWidgets.QVBoxLayout(self)

        layout.addWidget(self.tipsLabel)
        layout.addStretch()
        layout.addWidget(self.processInfoLabel)
        layout.addWidget(self.processBar)
        
    def setProcessBarRange(self, min, max):
        self.processBar.setRange(min, max)
        
    def setProcessBarVal(self, val):
        self.processBar.setValue(self.processBar.value() + val)
        
    def setProcessInfos(self, rate, info):
        self.processBar.setValue(rate)
        self.processInfoLabel.setText(info)
        #QtCore.QCoreApplication.processEvents(QtCore.QEventLoop.ExcludeUserInputEvents)

    def clear(self):
        self.processBar.setValue(0)
        
if __name__ == '__main__':
    app = QtWidgets.QApplication([])
    w = WaitDialogWithText()
    w.show()

    time.sleep(5)
    w.close()
    app.exec_() 
