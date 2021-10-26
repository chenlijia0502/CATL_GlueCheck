# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui_logindialog.ui'
#
# Created by: PyQt5 UI code generator 5.14.1
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_Dialog(object):
    def setupUi(self, Dialog):
        Dialog.setObjectName("Dialog")
        Dialog.resize(885, 565)
        Dialog.setMinimumSize(QtCore.QSize(885, 565))
        Dialog.setMaximumSize(QtCore.QSize(885, 565))
        Dialog.setStyleSheet("QDialog#Dialog\n"
"{\n"
"border-image: url(res/loginbackground1.jpg); }")
        self.widget_4 = QtWidgets.QWidget(Dialog)
        self.widget_4.setGeometry(QtCore.QRect(240, 100, 371, 291))
        self.widget_4.setObjectName("widget_4")
        self.verticalLayout = QtWidgets.QVBoxLayout(self.widget_4)
        self.verticalLayout.setContentsMargins(0, 0, 0, 0)
        self.verticalLayout.setSpacing(50)
        self.verticalLayout.setObjectName("verticalLayout")
        self.label_4 = QtWidgets.QLabel(self.widget_4)
        self.label_4.setStyleSheet("font: 16pt \"Arial Unicode MS\";\n"
"color: rgb(255, 255, 255);")
        self.label_4.setAlignment(QtCore.Qt.AlignCenter)
        self.label_4.setObjectName("label_4")
        self.verticalLayout.addWidget(self.label_4)
        self.widget_2 = QtWidgets.QWidget(self.widget_4)
        self.widget_2.setObjectName("widget_2")
        self.horizontalLayout = QtWidgets.QHBoxLayout(self.widget_2)
        self.horizontalLayout.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.label = QtWidgets.QLabel(self.widget_2)
        self.label.setStyleSheet("font: 14pt \"Arial Unicode MS\";\n"
"color: rgb(53, 255, 255);")
        self.label.setObjectName("label")
        self.horizontalLayout.addWidget(self.label)
        self.textEdit = QtWidgets.QTextEdit(self.widget_2)
        self.textEdit.setObjectName("textEdit")
        self.horizontalLayout.addWidget(self.textEdit)
        self.horizontalLayout.setStretch(0, 1)
        self.horizontalLayout.setStretch(1, 3)
        self.verticalLayout.addWidget(self.widget_2)
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
        self.textEdit_password = QtWidgets.QTextEdit(self.widget_3)
        self.textEdit_password.setObjectName("textEdit_password")
        self.horizontalLayout_3.addWidget(self.textEdit_password)
        self.ptb_key = QtWidgets.QPushButton(self.widget_3)
        self.ptb_key.setMinimumSize(QtCore.QSize(35, 31))
        self.ptb_key.setSizeIncrement(QtCore.QSize(0, 0))
        self.ptb_key.setStyleSheet("QPushButton#ptb_key\n"
"{ border-image: url(res/key.png); }  \n"
"QPushButton:hover#ptb_key\n"
"{ border-image: url(res/key_hover.png); }  \n"
"\n"
"\n"
"")
        self.ptb_key.setText("")
        self.ptb_key.setObjectName("ptb_key")
        self.horizontalLayout_3.addWidget(self.ptb_key)
        self.horizontalLayout_3.setStretch(0, 1)
        self.horizontalLayout_3.setStretch(1, 3)
        self.verticalLayout.addWidget(self.widget_3)
        self.widget_5 = QtWidgets.QWidget(Dialog)
        self.widget_5.setGeometry(QtCore.QRect(290, 420, 261, 61))
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

        self.retranslateUi(Dialog)
        QtCore.QMetaObject.connectSlotsByName(Dialog)

    def retranslateUi(self, Dialog):
        _translate = QtCore.QCoreApplication.translate
        Dialog.setWindowTitle(_translate("Dialog", "Dialog"))
        self.label_4.setText(_translate("Dialog", "用户权限切换"))
        self.label.setText(_translate("Dialog", "用户名:"))
        self.textEdit.setHtml(_translate("Dialog", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:\'SimSun\'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:12pt;\">BYD-极片边检</span></p></body></html>"))
        self.label_2.setText(_translate("Dialog", "权限选择:"))
        self.label_3.setText(_translate("Dialog", "密码："))
        self.pbt_ensure.setText(_translate("Dialog", "确认"))
        self.pbt_quit.setText(_translate("Dialog", "退出"))
