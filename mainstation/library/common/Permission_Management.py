# coding=utf-8
import os
import re
import pickle
from PyQt5 import  QtCore, QtGui,QtWidgets
import pyqtgraph as pg
from UI.ui_logindialog import Ui_Dialog
from UI.ui_changepassworddialog import Ui_PasswordDialog
from globalparam import ChineseWord
from globalparam import PermissionLevel
import struct



class kxprivilege_management(QtWidgets.QDialog):
    '''
    权限管理类
    '''
    sig_refresh_curpri = QtCore.pyqtSignal(int)
    def __init__(self, parent=None):
        '''
        初始化
        '''
        super(kxprivilege_management, self).__init__(parent)
        self.ui = Ui_Dialog()
        self.ui.setupUi(self)
        self.list_strpassword = ['kexin2008' for i in range(2)]#默认初始密码
        self.ncurrentpermission = 0
        self.ChangePasswordDialog = None
        self.__ininconnection()
        self.__initcombox()
        self.setWindowFlags(QtCore.Qt.FramelessWindowHint)#全屏

    def __ininconnection(self):
        self.ui.pbt_quit.clicked.connect(self.close)
        self.ui.pbt_ensure.clicked.connect(self.slotconfirmpassword)
        self.ui.ptb_key.clicked.connect(self.slotchangePasswordBtnClicked)


    def __initcombox(self):
        self.ui.comboBox.addItem("")
        self.ui.comboBox.setItemText(0, ChineseWord.PRODUCER)
        self.ui.comboBox.addItem("")
        self.ui.comboBox.setItemText(1, ChineseWord.MANAGER)

    def __initchangepassworddialog(self):
        self.ChangePasswordDialog = KxUserChangePasswordDialog(self.str_passwordpath)
        self.ChangePasswordDialog.signal_refresh.connect(self.refreshpassword)

    def setpasswordpath(self, passwordpath=''):
        '''
        设置密码保存路径
        '''
        self.str_passwordpath = passwordpath
        if passwordpath == '':
            self.str_passwordpath = 'd:\\'
        self.getpassword()

    def getpermissionlevel(self):
        return self.ncurrentpermission

    def getpassword(self):
        '''
        获得密码
        '''
        if not os.path.exists(self.str_passwordpath):
            os.makedirs(self.str_passwordpath)
            self.setpassword()
            return
        if os.path.isfile(self.str_passwordpath + '/password.dat'):
            with open(self.str_passwordpath + '/password.dat', "rb") as fp:
                data = fp.read()
                data = data[struct.calcsize('i'):]
                self.list_strpassword = pickle.loads(data)
        else:
            self.setpassword()

    def setpassword(self):
        '''
        设置密码
        '''
        if os.path.exists(self.str_passwordpath):
            b_passwordobject = pickle.dumps(self.list_strpassword)
            data = struct.pack("i", len(b_passwordobject))
            data += struct.pack(str(len(b_passwordobject)) + "s", b_passwordobject)
            with open(self.str_passwordpath + '/password.dat', "wb") as fp:
                fp.write(data)

    #
    # def slotchangeprivilege(self):
    #     if self.bool_privilege_status == False:
    #         editPathDialog = KxLineEditDialog(self)
    #         editPathDialog.setWindowTitle(QtCore.QString.fromUtf8(self.tr(u'Upgrade permissions level')))
    #         editPathDialog.setlabeltext(self.tr(u'Password:'))
    #         editPathDialog.setEchoMode(QtGui.QLineEdit.Password)
    #         if editPathDialog.exec_() == QtGui.QDialog.Accepted:
    #             passwordStr = editPathDialog.gettext()
    #             if not re.match('^[0-9a-zA-Z]*$', passwordStr):
    #                 QtGui.QMessageBox.warning(self, self.tr('Warning'),
    #                                           u'密码必须是数字或字符',
    #                                           QtGui.QMessageBox.Ok)
    #                 return
    #             if passwordStr == self.str_password:
    #                 self.setprivilege_status(True)
    #                 if self.parent is not None:
    #                     self.parent.setprivilege(u'管理员')
    #             else:
    #                 QtGui.QMessageBox.warning(self, self.tr('Warning'), self.tr(u'The password is incorrect!'),
    #                                           QtGui.QMessageBox.Ok)
    #                 return
    #
    #     elif self.bool_privilege_status == True:
    #         self.setprivilege_status(False)
    #         if self.parent is not None:
    #             self.parent.setprivilege(u'操作员')

    def slotconfirmpassword(self):
        nindex = self.ui.comboBox.currentIndex()
        s_word = str(self.ui.textEdit_password.toPlainText())
        if self.list_strpassword[nindex] != s_word:
            QtWidgets.QMessageBox.warning(self, self.tr('Warning'), u'密码输入错误',
                                      QtWidgets.QMessageBox.Ok)
            return
        elif self.list_strpassword[nindex] == s_word and nindex != self.ncurrentpermission:
            QtWidgets.QMessageBox.warning(self, u'提示', u'权限修改成功',
                                      QtWidgets.QMessageBox.Ok)
            self.ncurrentpermission = nindex
            self.sig_refresh_curpri.emit(self.ncurrentpermission)

    def slotchangePasswordBtnClicked(self):
        '''
        改变密码槽函数
        '''
        if self.ChangePasswordDialog is None:
            self.__initchangepassworddialog()
        self.ChangePasswordDialog.exec_()

    def refreshpassword(self, list_sword):
        self.list_strpassword = list_sword

    def showEvent(self, QShowEvent):
        self.ui.textEdit_password.clear()



class KxUserChangePasswordDialog(QtWidgets.QDialog):
    signal_refresh = QtCore.pyqtSignal(list)
    def __init__(self, passwordpath):
        super(KxUserChangePasswordDialog, self).__init__()
        self.ui = Ui_PasswordDialog()
        self.ui.setupUi(self)
        self.str_passwordpath = passwordpath
        self.setWindowIcon(QtGui.QIcon('res\\key.png'))
        self.setWindowTitle(ChineseWord.CHANGEPASSWORD)
        self.__initnconnection()
        self.__initcombox()
        self.getpassword()
        # self.setWindowFlags(QtCore.Qt.FramelessWindowHint)#全屏

    def __initnconnection(self):
        self.ui.pbt_quit.clicked.connect(self.close)
        self.ui.pbt_ensure.clicked.connect(self.slotconfirm)


    def __initcombox(self):
        self.ui.comboBox.addItem("")
        self.ui.comboBox.setItemText(0, ChineseWord.PRODUCER)
        self.ui.comboBox.addItem("")
        self.ui.comboBox.setItemText(1, ChineseWord.MANAGER)

    def getpassword(self):
        '''
        获得密码
        '''
        if not os.path.exists(self.str_passwordpath):
            os.makedirs(self.str_passwordpath)
            self.setpassword()
            return
        if os.path.isfile(self.str_passwordpath + '/password.dat'):
            with open(self.str_passwordpath + '/password.dat', "rb") as fp:
                data = fp.read()
                # n_passwordlen = struct.unpack('i', data[:struct.calcsize('i')])[0]
                data = data[struct.calcsize('i'):]
                self.list_strpassword = pickle.loads(data)
        else:
            QtWidgets.QMessageBox.warning(self, self.tr('Warning'), u'密码管理数据丢失，请重新打开密码管理',
                                      QtWidgets.QMessageBox.Ok)
            self.close()

    def setpassword(self):
        '''
        设置密码
        '''
        if os.path.exists(self.str_passwordpath):
            b_passwordobject = pickle.dumps(self.list_strpassword)
            data = struct.pack("i", len(b_passwordobject))
            data += struct.pack(str(len(b_passwordobject)) + "s", b_passwordobject)
            with open(self.str_passwordpath + '/password.dat', "wb") as fp:
                fp.write(data)

    def slotconfirm(self):
        nindex = self.ui.comboBox.currentIndex()
        s_word0 = str(self.ui.textEdit_password0.toPlainText())
        s_word1 = str(self.ui.textEdit_password1.toPlainText())
        s_word2 = str(self.ui.textEdit_password2.toPlainText())
        s_word3 = str(self.ui.textEdit_password3.toPlainText())

        if s_word0 != self.list_strpassword[PermissionLevel.MANAGER]: #
            QtWidgets.QMessageBox.warning(self, self.tr('Warning'), u'管理员密码错误',
                                      QtWidgets.QMessageBox.Ok)
            return
        if s_word1 != self.list_strpassword[nindex]:
            QtWidgets.QMessageBox.warning(self, self.tr('Warning'), self.ui.comboBox.currentText() + u'密码错误',
                                      QtWidgets.QMessageBox.Ok)
            return

        if s_word2 != s_word3:
            QtWidgets.QMessageBox.warning(self, self.tr('Warning'), u'两次密码不一致，请重新确认',
                                      QtWidgets.QMessageBox.Ok)
            return

        if not re.match('^[0-9a-zA-Z]*$', s_word3):
            QtWidgets.QMessageBox.warning(self, self.tr('Warning'), u'密码只能由数字跟字母组成',
                                      QtWidgets.QMessageBox.Ok)
            return

        self.list_strpassword[nindex] = s_word3
        self.setpassword()
        QtWidgets.QMessageBox.warning(self, u'提示', u'密码修改成功',
                                  QtWidgets.QMessageBox.Ok)
        self.signal_refresh.emit(self.list_strpassword)
