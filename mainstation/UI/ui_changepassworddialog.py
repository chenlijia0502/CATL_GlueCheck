# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui_changepassworddialog.ui'
#
# Created by: PyQt5 UI code generator 5.14.1
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_PasswordDialog(object):
    def setupUi(self, PasswordDialog):
        PasswordDialog.setObjectName("PasswordDialog")
        PasswordDialog.resize(750, 510)
        PasswordDialog.setMinimumSize(QtCore.QSize(750, 510))
        PasswordDialog.setMaximumSize(QtCore.QSize(750, 510))
        PasswordDialog.setStyleSheet("QDialog#PasswordDialog\n"
"{\n"
"border-image: url(res/loginbackground2.jpg); }")
        self.widget_4 = QtWidgets.QWidget(PasswordDialog)
        self.widget_4.setGeometry(QtCore.QRect(60, 40, 611, 371))
        self.widget_4.setObjectName("widget_4")
        self.verticalLayout = QtWidgets.QVBoxLayout(self.widget_4)
        self.verticalLayout.setContentsMargins(0, 0, 0, 0)
        self.verticalLayout.setSpacing(40)
        self.verticalLayout.setObjectName("verticalLayout")
        self.label_4 = QtWidgets.QLabel(self.widget_4)
        self.label_4.setStyleSheet("font: 16pt \"Arial Unicode MS\";\n"
"color: rgb(255, 255, 255);")
        self.label_4.setAlignment(QtCore.Qt.AlignCenter)
        self.label_4.setObjectName("label_4")
        self.verticalLayout.addWidget(self.label_4)
        self.widget_8 = QtWidgets.QWidget(self.widget_4)
        self.widget_8.setObjectName("widget_8")
        self.horizontalLayout_7 = QtWidgets.QHBoxLayout(self.widget_8)
        self.horizontalLayout_7.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout_7.setObjectName("horizontalLayout_7")
        self.label_7 = QtWidgets.QLabel(self.widget_8)
        self.label_7.setMinimumSize(QtCore.QSize(87, 0))
        self.label_7.setStyleSheet("font: 14pt \"Arial Unicode MS\";\n"
"color: rgb(53, 255, 255);")
        self.label_7.setObjectName("label_7")
        self.horizontalLayout_7.addWidget(self.label_7)
        self.textEdit_password0 = QtWidgets.QTextEdit(self.widget_8)
        self.textEdit_password0.setObjectName("textEdit_password0")
        self.horizontalLayout_7.addWidget(self.textEdit_password0)
        self.horizontalLayout_7.setStretch(0, 1)
        self.horizontalLayout_7.setStretch(1, 3)
        self.verticalLayout.addWidget(self.widget_8)
        self.widget = QtWidgets.QWidget(self.widget_4)
        self.widget.setObjectName("widget")
        self.horizontalLayout_2 = QtWidgets.QHBoxLayout(self.widget)
        self.horizontalLayout_2.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        self.label_2 = QtWidgets.QLabel(self.widget)
        self.label_2.setStyleSheet("font: 14pt \"Arial Unicode MS\";\n"
"color: rgb(53, 255, 255);")
        self.label_2.setObjectName("label_2")
        self.horizontalLayout_2.addWidget(self.label_2)
        self.comboBox = QtWidgets.QComboBox(self.widget)
        self.comboBox.setMinimumSize(QtCore.QSize(0, 38))
        self.comboBox.setMaximumSize(QtCore.QSize(1111, 11111111))
        self.comboBox.setStyleSheet("border:1px solid rgb(255, 255, 255);\n"
"QComboBox::drop-down {height:60px;}")
        self.comboBox.setFrame(True)
        self.comboBox.setObjectName("comboBox")
        self.horizontalLayout_2.addWidget(self.comboBox)
        self.horizontalLayout_2.setStretch(0, 1)
        self.horizontalLayout_2.setStretch(1, 3)
        self.verticalLayout.addWidget(self.widget)
        self.widget_3 = QtWidgets.QWidget(self.widget_4)
        self.widget_3.setObjectName("widget_3")
        self.horizontalLayout_3 = QtWidgets.QHBoxLayout(self.widget_3)
        self.horizontalLayout_3.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout_3.setObjectName("horizontalLayout_3")
        self.label_3 = QtWidgets.QLabel(self.widget_3)
        self.label_3.setMinimumSize(QtCore.QSize(87, 0))
        self.label_3.setStyleSheet("font: 14pt \"Arial Unicode MS\";\n"
"color: rgb(53, 255, 255);")
        self.label_3.setObjectName("label_3")
        self.horizontalLayout_3.addWidget(self.label_3)
        self.textEdit_password1 = QtWidgets.QTextEdit(self.widget_3)
        self.textEdit_password1.setObjectName("textEdit_password1")
        self.horizontalLayout_3.addWidget(self.textEdit_password1)
        self.horizontalLayout_3.setStretch(0, 1)
        self.horizontalLayout_3.setStretch(1, 3)
        self.verticalLayout.addWidget(self.widget_3)
        self.widget_6 = QtWidgets.QWidget(self.widget_4)
        self.widget_6.setObjectName("widget_6")
        self.horizontalLayout_5 = QtWidgets.QHBoxLayout(self.widget_6)
        self.horizontalLayout_5.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout_5.setObjectName("horizontalLayout_5")
        self.label_5 = QtWidgets.QLabel(self.widget_6)
        self.label_5.setMinimumSize(QtCore.QSize(87, 0))
        self.label_5.setStyleSheet("font: 14pt \"Arial Unicode MS\";\n"
"color: rgb(53, 255, 255);")
        self.label_5.setObjectName("label_5")
        self.horizontalLayout_5.addWidget(self.label_5)
        self.textEdit_password2 = QtWidgets.QTextEdit(self.widget_6)
        self.textEdit_password2.setObjectName("textEdit_password2")
        self.horizontalLayout_5.addWidget(self.textEdit_password2)
        self.horizontalLayout_5.setStretch(0, 1)
        self.horizontalLayout_5.setStretch(1, 3)
        self.verticalLayout.addWidget(self.widget_6)
        self.widget_7 = QtWidgets.QWidget(self.widget_4)
        self.widget_7.setObjectName("widget_7")
        self.horizontalLayout_6 = QtWidgets.QHBoxLayout(self.widget_7)
        self.horizontalLayout_6.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout_6.setObjectName("horizontalLayout_6")
        self.label_6 = QtWidgets.QLabel(self.widget_7)
        self.label_6.setMinimumSize(QtCore.QSize(87, 0))
        self.label_6.setStyleSheet("font: 14pt \"Arial Unicode MS\";\n"
"color: rgb(53, 255, 255);")
        self.label_6.setObjectName("label_6")
        self.horizontalLayout_6.addWidget(self.label_6)
        self.textEdit_password3 = QtWidgets.QTextEdit(self.widget_7)
        self.textEdit_password3.setObjectName("textEdit_password3")
        self.horizontalLayout_6.addWidget(self.textEdit_password3)
        self.horizontalLayout_6.setStretch(0, 1)
        self.horizontalLayout_6.setStretch(1, 3)
        self.verticalLayout.addWidget(self.widget_7)
        self.widget_5 = QtWidgets.QWidget(PasswordDialog)
        self.widget_5.setGeometry(QtCore.QRect(220, 420, 261, 61))
        self.widget_5.setObjectName("widget_5")
        self.horizontalLayout_4 = QtWidgets.QHBoxLayout(self.widget_5)
        self.horizontalLayout_4.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout_4.setObjectName("horizontalLayout_4")
        self.pbt_ensure = QtWidgets.QPushButton(self.widget_5)
        self.pbt_ensure.setMinimumSize(QtCore.QSize(0, 30))
        self.pbt_ensure.setStyleSheet("background-color: rgb(247, 247, 247);\n"
"font: 75 10pt \"Agency FB\";")
        self.pbt_ensure.setObjectName("pbt_ensure")
        self.horizontalLayout_4.addWidget(self.pbt_ensure)
        spacerItem = QtWidgets.QSpacerItem(78, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_4.addItem(spacerItem)
        self.pbt_quit = QtWidgets.QPushButton(self.widget_5)
        self.pbt_quit.setMinimumSize(QtCore.QSize(0, 30))
        self.pbt_quit.setStyleSheet("background-color: rgb(247, 247, 247);\n"
"font: 75 10pt \"Agency FB\";")
        self.pbt_quit.setObjectName("pbt_quit")
        self.horizontalLayout_4.addWidget(self.pbt_quit)

        self.retranslateUi(PasswordDialog)
        QtCore.QMetaObject.connectSlotsByName(PasswordDialog)

    def retranslateUi(self, PasswordDialog):
        _translate = QtCore.QCoreApplication.translate
        PasswordDialog.setWindowTitle(_translate("PasswordDialog", "Dialog"))
        self.label_4.setText(_translate("PasswordDialog", "修改密码"))
        self.label_7.setText(_translate("PasswordDialog", "输入管理员密码："))
        self.label_2.setText(_translate("PasswordDialog", "权限选择:"))
        self.label_3.setText(_translate("PasswordDialog", "输入旧密码："))
        self.label_5.setText(_translate("PasswordDialog", "输入新密码："))
        self.label_6.setText(_translate("PasswordDialog", "再输入新密码："))
        self.pbt_ensure.setText(_translate("PasswordDialog", "确认"))
        self.pbt_quit.setText(_translate("PasswordDialog", "退出"))
