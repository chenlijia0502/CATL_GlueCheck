# coding=utf-8
import os
import re
import pickle
from PyQt5 import  Qt, QtCore, QtGui,QtWidgets
import pyqtgraph as pg
from UI.ui_logindialog import Ui_Dialog
from UI.ui_changepassworddialog import Ui_PasswordDialog
from globalparam import ChineseWord
from globalparam import PermissionLevel
import struct
from library.common.usermanager.AddUserDialogNew import CAddUserDialogNew
import time

"""
    初始版本是比亚迪边检，后经宁德时代下箱体涂胶迭代
"""

class kxprivilege_management(QtWidgets.QDialog):
    '''
    权限管理类, 根据输入的list_slevel用户进行勾选该权限下拥有多少种权限

    默认第一位是密码管理权限，如果为0则不能进入账号管理界面

    '''
    sig_refresh_curpri = QtCore.pyqtSignal(int)
    def __init__(self, parent=None, list_slevel = ["权限A", "权限B"]):
        '''
        初始化
        '''
        super(kxprivilege_management, self).__init__(parent)
        self.ui = Ui_Dialog()
        self.ui.setupUi(self)
        self.setWindowTitle("权限")
        self.list_curpermission = 0
        self.ChangePasswordDialog = None
        self.ui.lineEdit.setEchoMode(QtWidgets.QLineEdit.Password)
        self.ui.lineEdit_2.setEchoMode(QtWidgets.QLineEdit.Password)
        self.list_slevel = list_slevel
        self.adduserdialog = CAddUserDialogNew(list_slevel)
        self.inputdialog = InputDialog()
        self.__ininconnection()
        self.setWindowFlags(QtCore.Qt.WindowStaysOnTopHint)#全屏
        #self.setWindowFlags(QtCore.Qt.WindowStaysOnTopHint)

        self.ui.lineEdit_2.setReadOnly(True)
        self.cur_account = None#当前账户
        self.ui.lineEdit.setReadOnly(True)
        self.text = ""

        # 定时器，确保对键盘输入进行定时清理
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


    def __ininconnection(self):
        # self.ui.pbt_quit.clicked.connect(self.close)

        self.ui.ptb_key.clicked.connect(self.slotManageuserBtnClicked)

        self.ui.pbt_ensure.clicked.connect(self._judgeacount)


    def keyPressEvent(self, a0: QtGui.QKeyEvent):
        self.nrecordtime = time.time()
        if a0.key() == QtCore.Qt.Key_Return:
            self.ui.lineEdit.setText(self.text)
            #print (self.text)
            self._judgeacount()
            self.text = ""
        else:
            self.text = self.text + a0.text()


    def _judgeacount(self):
        word = self.ui.lineEdit.text()
        if len(word) > 0:
            list_list_account = self.adduserdialog.getUserlist()
            b_status = False
            for account in list_list_account:
                if account[1] == word:
                    b_status = True
                    self.cur_account = account
                    break
            if b_status:
                self.ui.lineEdit.clear()
                self.close()
            else:
                respond = QtWidgets.QMessageBox.warning(self, u"警告", u"不在用户列表中", QtWidgets.QMessageBox.Cancel)
                self.ui.lineEdit.clear()
                return

    def slotManageuserBtnClicked(self):
        '''
        进入用户管理界面
        '''

        self.setWindowFlags(QtCore.Qt.FramelessWindowHint)  # 全屏

        self.inputdialog.show()

        self.inputdialog.exec()

        id = self.inputdialog.getinfo()

        self.inputdialog.clear()

        list_list_account = self.adduserdialog.getUserlist()

        bstatus = False

        for account in list_list_account:

            if id == account[1] and account[2][0] == "1":

                bstatus = True

                break

        if bstatus:

            self.adduserdialog.show()

            self.adduserdialog.exec_()

        else:

            respond = QtWidgets.QMessageBox.warning(self, u"警告", u"非管理员权限",QtWidgets.QMessageBox.Cancel)

        self.setWindowFlags(QtCore.Qt.FramelessWindowHint | QtCore.Qt.WindowStaysOnTopHint)  # 全屏


    def getpermissionlevel(self):
        ###补0， 目的是随着项目更新，权限会增加，可能出现新版本无法兼容旧的情况
        # if self.cur_account != None and len(self.cur_account[1]) < len(self.list_slevel):
        #     for i in range(len(self.list_slevel) - len(self.cur_account[1])):
        #         self.cur_account[1] += '0'
        return self.cur_account

    def clear(self):
        self.cur_account = None



class InputDialog(QtWidgets.QDialog):
    def __init__(self):
        super(InputDialog, self).__init__()
        self.horizonlayout = QtWidgets.QHBoxLayout(self)
        self.setWindowTitle("输入账号")
        self.label = QtWidgets.QLabel()
        self.label.setText("刷管理员卡号：")
        self.label.setStyleSheet("font: 14pt \"Arial Unicode MS\";")
        self.lineedit = QtWidgets.QLineEdit()
        self.horizonlayout.addWidget(self.label)
        self.horizonlayout.addWidget(self.lineedit)
        self.setLayout(self.horizonlayout)
        self.setMaximumSize(QtCore.QSize(400, 200))
        self.lineedit.setEchoMode(QtWidgets.QLineEdit.Password)
        self.s_input = ""
        self.setWindowFlags(QtCore.Qt.WindowStaysOnTopHint)
        self.lineedit.setReadOnly(True)

        # 定时器，确保对键盘输入进行定时清理
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
            self.s_input = ""


    def keyPressEvent(self, a0: QtGui.QKeyEvent):
        self.nrecordtime = time.time()
        if a0.key() == QtCore.Qt.Key_Return:
            self.lineedit.setText(self.s_input)
            self.close()
            #print (self.text)
        else:
            self.s_input = self.s_input + a0.text()


    def getinfo(self):
        return self.s_input


    def clear(self):
        self.s_input = ""
        self.lineedit.clear()


    def showEvent(self, a0: QtGui.QShowEvent):
        pass


    def closeEvent(self, a0: QtGui.QCloseEvent):
        pass


if __name__ == "__main__":
    app = QtWidgets.QApplication([])

    w = kxprivilege_management(list_slevel=["账号管理", "控制工具栏",  "配方选择", "参数修改"])
    w.show()

    app.exec_()