# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui_kxtestmonitor.ui'
#
# Created by: PyQt5 UI code generator 5.9.1
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_ParamPYWidget(object):
    def setupUi(self, ParamPYWidget):
        ParamPYWidget.setObjectName("ParamPYWidget")
        ParamPYWidget.resize(1171, 864)
        self.horizontalLayout_4 = QtWidgets.QHBoxLayout(ParamPYWidget)
        self.horizontalLayout_4.setObjectName("horizontalLayout_4")
        self.widget_5 = GraphicsView(ParamPYWidget)
        self.widget_5.setObjectName("widget_5")
        self.horizontalLayout_4.addWidget(self.widget_5)
        self.widget_4 = QtWidgets.QWidget(ParamPYWidget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.widget_4.sizePolicy().hasHeightForWidth())
        self.widget_4.setSizePolicy(sizePolicy)
        self.widget_4.setObjectName("widget_4")
        self.verticalLayout = QtWidgets.QVBoxLayout(self.widget_4)
        self.verticalLayout.setObjectName("verticalLayout")
        spacerItem = QtWidgets.QSpacerItem(20, 40, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        self.verticalLayout.addItem(spacerItem)
        self.widget = QtWidgets.QWidget(self.widget_4)
        self.widget.setObjectName("widget")
        self.horizontalLayout = QtWidgets.QHBoxLayout(self.widget)
        self.horizontalLayout.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout.setSpacing(0)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.label_2 = QtWidgets.QLabel(self.widget)
        font = QtGui.QFont()
        font.setFamily("Andalus")
        font.setPointSize(22)
        self.label_2.setFont(font)
        self.label_2.setObjectName("label_2")
        self.horizontalLayout.addWidget(self.label_2)
        self.label_id = QtWidgets.QLabel(self.widget)
        font = QtGui.QFont()
        font.setFamily("Andalus")
        font.setPointSize(22)
        self.label_id.setFont(font)
        self.label_id.setText("")
        self.label_id.setObjectName("label_id")
        self.horizontalLayout.addWidget(self.label_id)
        self.horizontalLayout.setStretch(0, 1)
        self.horizontalLayout.setStretch(1, 1)
        self.verticalLayout.addWidget(self.widget)
        spacerItem1 = QtWidgets.QSpacerItem(20, 40, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        self.verticalLayout.addItem(spacerItem1)
        self.widget_2 = QtWidgets.QWidget(self.widget_4)
        self.widget_2.setObjectName("widget_2")
        self.horizontalLayout_2 = QtWidgets.QHBoxLayout(self.widget_2)
        self.horizontalLayout_2.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout_2.setSpacing(0)
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        self.label_5 = QtWidgets.QLabel(self.widget_2)
        font = QtGui.QFont()
        font.setFamily("Andalus")
        font.setPointSize(22)
        self.label_5.setFont(font)
        self.label_5.setObjectName("label_5")
        self.horizontalLayout_2.addWidget(self.label_5)
        self.label_dot = QtWidgets.QLabel(self.widget_2)
        font = QtGui.QFont()
        font.setFamily("Andalus")
        font.setPointSize(22)
        self.label_dot.setFont(font)
        self.label_dot.setText("")
        self.label_dot.setObjectName("label_dot")
        self.horizontalLayout_2.addWidget(self.label_dot)
        self.horizontalLayout_2.setStretch(0, 1)
        self.horizontalLayout_2.setStretch(1, 1)
        self.verticalLayout.addWidget(self.widget_2)
        spacerItem2 = QtWidgets.QSpacerItem(20, 40, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        self.verticalLayout.addItem(spacerItem2)
        self.widget_3 = QtWidgets.QWidget(self.widget_4)
        self.widget_3.setObjectName("widget_3")
        self.horizontalLayout_3 = QtWidgets.QHBoxLayout(self.widget_3)
        self.horizontalLayout_3.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout_3.setSpacing(0)
        self.horizontalLayout_3.setObjectName("horizontalLayout_3")
        self.label_7 = QtWidgets.QLabel(self.widget_3)
        font = QtGui.QFont()
        font.setFamily("Andalus")
        font.setPointSize(22)
        self.label_7.setFont(font)
        self.label_7.setObjectName("label_7")
        self.horizontalLayout_3.addWidget(self.label_7)
        self.label_width = QtWidgets.QLabel(self.widget_3)
        font = QtGui.QFont()
        font.setFamily("Andalus")
        font.setPointSize(22)
        self.label_width.setFont(font)
        self.label_width.setText("")
        self.label_width.setObjectName("label_width")
        self.horizontalLayout_3.addWidget(self.label_width)
        self.horizontalLayout_3.setStretch(0, 1)
        self.horizontalLayout_3.setStretch(1, 1)
        self.verticalLayout.addWidget(self.widget_3)
        spacerItem3 = QtWidgets.QSpacerItem(20, 40, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        self.verticalLayout.addItem(spacerItem3)
        self.horizontalLayout_4.addWidget(self.widget_4)
        self.horizontalLayout_4.setStretch(0, 20)
        self.horizontalLayout_4.setStretch(1, 1)

        self.retranslateUi(ParamPYWidget)
        QtCore.QMetaObject.connectSlotsByName(ParamPYWidget)

    def retranslateUi(self, ParamPYWidget):
        _translate = QtCore.QCoreApplication.translate
        ParamPYWidget.setWindowTitle(_translate("ParamPYWidget", "参数界面"))
        self.label_2.setText(_translate("ParamPYWidget", "ID："))
        self.label_5.setText(_translate("ParamPYWidget", "点数："))
        self.label_7.setText(_translate("ParamPYWidget", "站点："))

from pyqtgraph import GraphicsView
