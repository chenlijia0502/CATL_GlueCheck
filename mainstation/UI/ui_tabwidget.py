# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui_tabwidget.ui'
#
# Created by: PyQt5 UI code generator 5.14.1
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_ZCTabwidget(object):
    def setupUi(self, ZCTabwidget):
        ZCTabwidget.setObjectName("ZCTabwidget")
        ZCTabwidget.resize(1137, 822)
        ZCTabwidget.setStyleSheet("QWidget#ZCTabwidget\n"
"{\n"
"    border-image: url(res/4.jpg); \n"
"}")
        self.horizontalLayout = QtWidgets.QHBoxLayout(ZCTabwidget)
        self.horizontalLayout.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout.setSpacing(0)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.frame = QtWidgets.QFrame(ZCTabwidget)
        self.frame.setStyleSheet("")
        self.frame.setFrameShape(QtWidgets.QFrame.StyledPanel)
        self.frame.setFrameShadow(QtWidgets.QFrame.Raised)
        self.frame.setObjectName("frame")
        self.verticalLayout = QtWidgets.QVBoxLayout(self.frame)
        self.verticalLayout.setContentsMargins(0, 0, 0, 0)
        self.verticalLayout.setSpacing(0)
        self.verticalLayout.setObjectName("verticalLayout")
        self.topwidget = QtWidgets.QWidget(self.frame)
        self.topwidget.setMinimumSize(QtCore.QSize(0, 36))
        self.topwidget.setMaximumSize(QtCore.QSize(16777215, 36))
        self.topwidget.setObjectName("topwidget")
        self.verticalLayout.addWidget(self.topwidget)
        self.botwidget = QtWidgets.QWidget(self.frame)
        self.botwidget.setObjectName("botwidget")
        self.verticalLayout.addWidget(self.botwidget)
        self.horizontalLayout.addWidget(self.frame)

        self.retranslateUi(ZCTabwidget)
        QtCore.QMetaObject.connectSlotsByName(ZCTabwidget)

    def retranslateUi(self, ZCTabwidget):
        _translate = QtCore.QCoreApplication.translate
        ZCTabwidget.setWindowTitle(_translate("ZCTabwidget", "Form"))
