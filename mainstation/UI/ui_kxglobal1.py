# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui_kxglobal1.ui'
#
# Created by: PyQt5 UI code generator 5.9.1
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_ParamPYLoadWidget(object):
    def setupUi(self, ParamPYLoadWidget):
        ParamPYLoadWidget.setObjectName("ParamPYLoadWidget")
        ParamPYLoadWidget.resize(1171, 864)
        self.horizontalLayout = QtWidgets.QHBoxLayout(ParamPYLoadWidget)
        self.horizontalLayout.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.h_gVShowRealImg = GraphicsView(ParamPYLoadWidget)
        self.h_gVShowRealImg.setObjectName("h_gVShowRealImg")
        self.horizontalLayout.addWidget(self.h_gVShowRealImg)
        self.widget_1 = QtWidgets.QWidget(ParamPYLoadWidget)
        self.widget_1.setObjectName("widget_1")
        self.verticalLayout = QtWidgets.QVBoxLayout(self.widget_1)
        self.verticalLayout.setContentsMargins(0, 5, 0, 0)
        self.verticalLayout.setSpacing(5)
        self.verticalLayout.setObjectName("verticalLayout")
        self.h_pBtLoad = QtWidgets.QPushButton(self.widget_1)
        self.h_pBtLoad.setMinimumSize(QtCore.QSize(0, 30))
        self.h_pBtLoad.setObjectName("h_pBtLoad")
        self.verticalLayout.addWidget(self.h_pBtLoad)
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
        self.verticalLayout.setStretch(1, 20)
        self.verticalLayout.setStretch(2, 1)
        self.horizontalLayout.addWidget(self.widget_1)
        self.horizontalLayout.setStretch(0, 8)
        self.horizontalLayout.setStretch(1, 3)

        self.retranslateUi(ParamPYLoadWidget)
        QtCore.QMetaObject.connectSlotsByName(ParamPYLoadWidget)

    def retranslateUi(self, ParamPYLoadWidget):
        _translate = QtCore.QCoreApplication.translate
        ParamPYLoadWidget.setWindowTitle(_translate("ParamPYLoadWidget", "参数界面"))
        self.h_pBtLoad.setText(_translate("ParamPYLoadWidget", "载入图像"))
        self.h_pBtSave.setText(_translate("ParamPYLoadWidget", "保存"))
        self.h_pBtCancel.setText(_translate("ParamPYLoadWidget", "取消"))

from pyqtgraph import GraphicsView
