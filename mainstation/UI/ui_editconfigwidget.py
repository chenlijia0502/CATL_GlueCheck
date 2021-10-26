# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui_editconfigwidget.ui'
#
# Created by: PyQt5 UI code generator 5.14.1
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_editconfigwidget(object):
    def setupUi(self, editconfigwidget):
        editconfigwidget.setObjectName("editconfigwidget")
        editconfigwidget.resize(1156, 723)
        editconfigwidget.setStyleSheet("")
        self.horizontalLayout = QtWidgets.QHBoxLayout(editconfigwidget)
        self.horizontalLayout.setContentsMargins(1, 1, 1, 1)
        self.horizontalLayout.setSpacing(1)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.textEdit = QtWidgets.QTextEdit(editconfigwidget)
        self.textEdit.setStyleSheet("font: 14pt \"Garamond\";")
        self.textEdit.setObjectName("textEdit")
        self.horizontalLayout.addWidget(self.textEdit)
        self.funwidget = QtWidgets.QWidget(editconfigwidget)
        self.funwidget.setStyleSheet("font: 16pt \"Arial\";")
        self.funwidget.setObjectName("funwidget")
        self.verticalLayout = QtWidgets.QVBoxLayout(self.funwidget)
        self.verticalLayout.setContentsMargins(1, 1, 1, 4)
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setObjectName("verticalLayout")
        self.pushButton = QtWidgets.QPushButton(self.funwidget)
        self.pushButton.setMinimumSize(QtCore.QSize(130, 80))
        self.pushButton.setMaximumSize(QtCore.QSize(80, 16777215))
        self.pushButton.setObjectName("pushButton")
        self.verticalLayout.addWidget(self.pushButton)
        self.pushButton_reset = QtWidgets.QPushButton(self.funwidget)
        self.pushButton_reset.setMinimumSize(QtCore.QSize(130, 80))
        self.pushButton_reset.setMaximumSize(QtCore.QSize(80, 16777215))
        self.pushButton_reset.setObjectName("pushButton_reset")
        self.verticalLayout.addWidget(self.pushButton_reset)
        self.pushButton_2 = QtWidgets.QPushButton(self.funwidget)
        self.pushButton_2.setMinimumSize(QtCore.QSize(130, 80))
        self.pushButton_2.setMaximumSize(QtCore.QSize(130, 80))
        self.pushButton_2.setObjectName("pushButton_2")
        self.verticalLayout.addWidget(self.pushButton_2)
        spacerItem = QtWidgets.QSpacerItem(28, 514, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        self.verticalLayout.addItem(spacerItem)
        self.horizontalLayout.addWidget(self.funwidget)

        self.retranslateUi(editconfigwidget)
        QtCore.QMetaObject.connectSlotsByName(editconfigwidget)

    def retranslateUi(self, editconfigwidget):
        _translate = QtCore.QCoreApplication.translate
        editconfigwidget.setWindowTitle(_translate("editconfigwidget", "Form"))
        self.pushButton.setText(_translate("editconfigwidget", "保存"))
        self.pushButton_reset.setText(_translate("editconfigwidget", "重置"))
        self.pushButton_2.setText(_translate("editconfigwidget", "关闭"))
