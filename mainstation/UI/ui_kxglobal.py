# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui_kxglobal.ui'
#
# Created by: PyQt5 UI code generator 5.9.1
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_ParamPYWidget(object):
    def setupUi(self, ParamPYWidget):
        ParamPYWidget.setObjectName("ParamPYWidget")
        ParamPYWidget.resize(1171, 864)
        self.horizontalLayout = QtWidgets.QHBoxLayout(ParamPYWidget)
        self.horizontalLayout.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.h_gVShowRealImg = GraphicsView(ParamPYWidget)
        self.h_gVShowRealImg.setObjectName("h_gVShowRealImg")
        self.horizontalLayout.addWidget(self.h_gVShowRealImg)
        self.widget_1 = QtWidgets.QWidget(ParamPYWidget)
        self.widget_1.setObjectName("widget_1")
        self.verticalLayout = QtWidgets.QVBoxLayout(self.widget_1)
        self.verticalLayout.setContentsMargins(0, 0, 0, 0)
        self.verticalLayout.setSpacing(0)
        self.verticalLayout.setObjectName("verticalLayout")
        self.widget_param = QtWidgets.QWidget(self.widget_1)
        self.widget_param.setObjectName("widget_param")
        self.verticalLayout.addWidget(self.widget_param)
        self.widget = QtWidgets.QWidget(self.widget_1)
        self.widget.setObjectName("widget")
        self.horizontalLayout_2 = QtWidgets.QHBoxLayout(self.widget)
        self.horizontalLayout_2.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout_2.setSpacing(0)
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        spacerItem = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_2.addItem(spacerItem)
        self.h_pBtSave = QtWidgets.QPushButton(self.widget)
        self.h_pBtSave.setMinimumSize(QtCore.QSize(0, 30))
        self.h_pBtSave.setObjectName("h_pBtSave")
        self.horizontalLayout_2.addWidget(self.h_pBtSave)
        spacerItem1 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_2.addItem(spacerItem1)
        self.h_pBtCancel = QtWidgets.QPushButton(self.widget)
        self.h_pBtCancel.setMinimumSize(QtCore.QSize(0, 30))
        self.h_pBtCancel.setObjectName("h_pBtCancel")
        self.horizontalLayout_2.addWidget(self.h_pBtCancel)
        spacerItem2 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_2.addItem(spacerItem2)
        self.verticalLayout.addWidget(self.widget)
        self.verticalLayout.setStretch(0, 20)
        self.verticalLayout.setStretch(1, 1)
        self.horizontalLayout.addWidget(self.widget_1)
        self.horizontalLayout.setStretch(0, 8)
        self.horizontalLayout.setStretch(1, 3)

        self.retranslateUi(ParamPYWidget)
        QtCore.QMetaObject.connectSlotsByName(ParamPYWidget)

    def retranslateUi(self, ParamPYWidget):
        _translate = QtCore.QCoreApplication.translate
        ParamPYWidget.setWindowTitle(_translate("ParamPYWidget", "参数界面"))
        self.h_pBtSave.setText(_translate("ParamPYWidget", "保存"))
        self.h_pBtCancel.setText(_translate("ParamPYWidget", "取消"))

from pyqtgraph import GraphicsView
