# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui_loginInterface.ui'
#
# Created by: PyQt5 UI code generator 5.14.1
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_LoginDialog(object):
    def setupUi(self, LoginDialog):
        LoginDialog.setObjectName("LoginDialog")
        LoginDialog.resize(700, 380)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(LoginDialog.sizePolicy().hasHeightForWidth())
        LoginDialog.setSizePolicy(sizePolicy)
        LoginDialog.setMinimumSize(QtCore.QSize(700, 380))
        LoginDialog.setMaximumSize(QtCore.QSize(700, 380))
        LoginDialog.setStyleSheet("QDialog#LoginDialog\n"
"{\n"
"border-image: url(res/loginbackground3.jpg); }")
        self.pushButton_main = QtWidgets.QPushButton(LoginDialog)
        self.pushButton_main.setGeometry(QtCore.QRect(10, 10, 30, 30))
        self.pushButton_main.setStyleSheet("QPushButton#pushButton_main\n"
"{ \n"
"border-image: url(res/mainset.png);\n"
"color: rgb(255, 255, 255);\n"
"}  \n"
"QPushButton:hover#pushButton_main\n"
"{ \n"
"border-image: url(res/mainset_hover.png);\n"
"color: rgb(85, 255, 0);\n"
"\n"
"} ")
        self.pushButton_main.setText("")
        self.pushButton_main.setObjectName("pushButton_main")
        self.pushButton_sub = QtWidgets.QPushButton(LoginDialog)
        self.pushButton_sub.setGeometry(QtCore.QRect(41, 10, 30, 30))
        self.pushButton_sub.setStyleSheet("\n"
"QPushButton#pushButton_sub\n"
"{ \n"
"border-image: url(res/mainset.png);\n"
"}  \n"
"QPushButton:hover#pushButton_sub\n"
"{ \n"
"border-image: url(res/mainset_hover.png);\n"
"\n"
"\n"
"} ")
        self.pushButton_sub.setText("")
        self.pushButton_sub.setObjectName("pushButton_sub")
        self.pushButton_login = QtWidgets.QPushButton(LoginDialog)
        self.pushButton_login.setGeometry(QtCore.QRect(160, 280, 191, 51))
        self.pushButton_login.setStyleSheet("QPushButton#pushButton_login\n"
"{ \n"
"background-color: rgba(255, 255, 255,5%);\n"
"font: 14pt \"Arial\";\n"
"color:rgb(210, 210, 210); \n"
"}  \n"
"QPushButton:hover#pushButton_login\n"
"{ \n"
"background-color: rgba(255, 255, 255,5%);\n"
"font: 18pt \"Arial\";\n"
"color: rgb(0, 0, 127);\n"
"} ")
        self.pushButton_login.setObjectName("pushButton_login")
        self.pushButton_quit = QtWidgets.QPushButton(LoginDialog)
        self.pushButton_quit.setGeometry(QtCore.QRect(370, 280, 191, 51))
        self.pushButton_quit.setStyleSheet("QPushButton#pushButton_quit\n"
"{ \n"
"background-color: rgba(255, 255, 255,5%);\n"
"font: 14pt \"Arial\";\n"
"color: rgb(210, 210, 210); \n"
"}  \n"
"QPushButton:hover#pushButton_quit\n"
"{ \n"
"background-color: rgba(255, 255, 255,5%);\n"
"font: 18pt \"Arial\";\n"
"color: rgb(0, 0, 127);\n"
"} ")
        self.pushButton_quit.setObjectName("pushButton_quit")

        self.retranslateUi(LoginDialog)
        QtCore.QMetaObject.connectSlotsByName(LoginDialog)

    def retranslateUi(self, LoginDialog):
        _translate = QtCore.QCoreApplication.translate
        LoginDialog.setWindowTitle(_translate("LoginDialog", "Dialog"))
        self.pushButton_main.setToolTip(_translate("LoginDialog", "主站配置"))
        self.pushButton_sub.setToolTip(_translate("LoginDialog", "子站配置"))
        self.pushButton_login.setText(_translate("LoginDialog", "登录"))
        self.pushButton_quit.setText(_translate("LoginDialog", "退出"))
