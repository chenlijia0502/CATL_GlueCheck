#coding=utf-8
'''
Created on 2017年07月10日

@author: huber yao
'''
from PyQt5 import QtGui, QtCore, QtWidgets
from library.mainwindow.ui_KxOneLineEditDialog import Ui_KxOneLineEditDialog

class KxLineEditDialog(QtWidgets.QDialog):
    '''
    只有一个QLineEdit和QLabel的对话框
    '''
    def __init__(self, parent = None):
        '''
        初始化
        '''
        super(KxLineEditDialog, self).__init__(parent)
        self.ui = Ui_KxOneLineEditDialog()
        self.ui.setupUi(self)
        self.setWindowFlags(QtCore.Qt.Window | QtCore.Qt.WindowTitleHint | QtCore.Qt.WindowCloseButtonHint)
        
    def settext(self, text):
        '''
        设置编辑框文本
        '''

        self.ui.lineEdit.setText(text)
            
            
    def setlabeltext(self, text):
        '''
        设置说明文本
        '''
        self.ui.label.setText(text)

            
    def setEchoMode(self, mode):
        '''
        设置回显模式
        '''
        self.ui.lineEdit.setEchoMode(mode)
    
    def gettext(self):
        '''
        得到编辑框文本
        '''
        return self.ui.lineEdit.text()
    
    
    def showEvent(self, event):
        '''
        对话框显示时触发此函数，设置焦点到编辑框上，选中所有字符方便再编辑
        '''
        if event.type() == QtGui.QShowEvent.Show:
            self.ui.lineEdit.setFocus()
            # self.ui.lineEdit.setSelection(0, self.ui.lineEdit.text().length())
        return QtWidgets.QDialog.showEvent(self, event)