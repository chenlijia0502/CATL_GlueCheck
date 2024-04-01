# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui_modelmanage.ui'
#
# Created by: PyQt5 UI code generator 5.14.1
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_ParamSetGraphic(object):
    def setupUi(self, ParamSetGraphic):
        ParamSetGraphic.setObjectName("ParamSetGraphic")
        ParamSetGraphic.resize(1183, 827)
        ParamSetGraphic.setStyleSheet("")
        self.horizontalLayout_5 = QtWidgets.QHBoxLayout(ParamSetGraphic)
        self.horizontalLayout_5.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout_5.setSpacing(0)
        self.horizontalLayout_5.setObjectName("horizontalLayout_5")
        self.frame = QtWidgets.QFrame(ParamSetGraphic)
        self.frame.setStyleSheet("QFrame#frame\n"
"{\n"
"border-image: url(res/4.jpg);\n"
"background-color: rgba(255, 255, 255, 10%);\n"
"}")
        self.frame.setFrameShape(QtWidgets.QFrame.StyledPanel)
        self.frame.setFrameShadow(QtWidgets.QFrame.Raised)
        self.frame.setObjectName("frame")
        self.verticalLayout_3 = QtWidgets.QVBoxLayout(self.frame)
        self.verticalLayout_3.setContentsMargins(0, 0, 0, 0)
        self.verticalLayout_3.setSpacing(0)
        self.verticalLayout_3.setObjectName("verticalLayout_3")
        self.widget = QtWidgets.QWidget(self.frame)
        self.widget.setStyleSheet("")
        self.widget.setObjectName("widget")
        self.horizontalLayout_4 = QtWidgets.QHBoxLayout(self.widget)
        self.horizontalLayout_4.setObjectName("horizontalLayout_4")
        self.label = QtWidgets.QLabel(self.widget)
        font = QtGui.QFont()
        font.setPointSize(11)
        self.label.setFont(font)
        self.label.setStyleSheet("color: rgb(255, 255, 255);")
        self.label.setObjectName("label")
        self.horizontalLayout_4.addWidget(self.label)
        self.h_searchedit = QtWidgets.QLineEdit(self.widget)
        self.h_searchedit.setMaximumSize(QtCore.QSize(189, 16777215))
        self.h_searchedit.setStyleSheet("background-color: rgba(255, 255, 255, 50%);")
        self.h_searchedit.setObjectName("h_searchedit")
        self.horizontalLayout_4.addWidget(self.h_searchedit)
        self.h_label = QtWidgets.QLabel(self.widget)
        self.h_label.setObjectName("h_label")
        self.horizontalLayout_4.addWidget(self.h_label)
        spacerItem = QtWidgets.QSpacerItem(885, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_4.addItem(spacerItem)
        self.verticalLayout_3.addWidget(self.widget)
        self.horizontalLayout_3 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_3.setContentsMargins(-1, -1, 3, -1)
        self.horizontalLayout_3.setSpacing(0)
        self.horizontalLayout_3.setObjectName("horizontalLayout_3")
        self.h_modeltableview = QtWidgets.QTableView(self.frame)
        self.h_modeltableview.setStyleSheet("QTableView#h_modeltableview\n"
"{\n"
"    background-color:transparent;\n"
"    background-color: rgba(255, 255, 255, 30%);\n"
"    \n"
"    \n"
"    gridline-color: rgb(255, 255, 255);\n"
"}")
        self.h_modeltableview.setObjectName("h_modeltableview")
        self.horizontalLayout_3.addWidget(self.h_modeltableview)
        self.verticalLayout_2 = QtWidgets.QVBoxLayout()
        self.verticalLayout_2.setSpacing(1)
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.h_widgetview = QtWidgets.QWidget(self.frame)
        self.h_widgetview.setStyleSheet("QWidget#h_widgetview\n"
"{\n"
"    background-color: rgba(255, 255, 255, 30%);\n"
"}\n"
"")
        self.h_widgetview.setObjectName("h_widgetview")
        self.verticalLayout_4 = QtWidgets.QVBoxLayout(self.h_widgetview)
        self.verticalLayout_4.setContentsMargins(1, 1, 1, 1)
        self.verticalLayout_4.setSpacing(1)
        self.verticalLayout_4.setObjectName("verticalLayout_4")
        self.layout_imageview = QtWidgets.QHBoxLayout()
        self.layout_imageview.setObjectName("layout_imageview")
        self.verticalLayout_4.addLayout(self.layout_imageview)
        self.verticalLayout_2.addWidget(self.h_widgetview)
        self.funwidget = QtWidgets.QWidget(self.frame)
        self.funwidget.setStyleSheet("QWidget#funwidget\n"
"{\n"
"    background-color: rgba(255, 255, 255, 30%);\n"
"}\n"
"")
        self.funwidget.setObjectName("funwidget")
        self.verticalLayout = QtWidgets.QVBoxLayout(self.funwidget)
        self.verticalLayout.setContentsMargins(3, 3, 3, 3)
        self.verticalLayout.setObjectName("verticalLayout")
        self.horizontalLayout = QtWidgets.QHBoxLayout()
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.h_addnewbtn = QtWidgets.QPushButton(self.funwidget)
        self.h_addnewbtn.setMinimumSize(QtCore.QSize(90, 70))
        self.h_addnewbtn.setMaximumSize(QtCore.QSize(95, 72))
        font = QtGui.QFont()
        font.setPointSize(23)
        self.h_addnewbtn.setFont(font)
        self.h_addnewbtn.setStyleSheet("background-color: rgba(181, 181, 181, 50%);\n"
"color: rgb(255, 255, 255);")
        self.h_addnewbtn.setObjectName("h_addnewbtn")
        self.horizontalLayout.addWidget(self.h_addnewbtn)
        self.h_clonebtn = QtWidgets.QPushButton(self.funwidget)
        self.h_clonebtn.setMinimumSize(QtCore.QSize(90, 70))
        self.h_clonebtn.setMaximumSize(QtCore.QSize(95, 72))
        font = QtGui.QFont()
        font.setPointSize(23)
        self.h_clonebtn.setFont(font)
        self.h_clonebtn.setStyleSheet("background-color: rgba(181, 181, 181, 50%);\n"
"color: rgb(255, 255, 255);")
        self.h_clonebtn.setObjectName("h_clonebtn")
        self.horizontalLayout.addWidget(self.h_clonebtn)
        self.verticalLayout.addLayout(self.horizontalLayout)
        self.horizontalLayout_2 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        self.h_delbtn = QtWidgets.QPushButton(self.funwidget)
        self.h_delbtn.setMinimumSize(QtCore.QSize(90, 70))
        self.h_delbtn.setMaximumSize(QtCore.QSize(95, 72))
        font = QtGui.QFont()
        font.setPointSize(23)
        self.h_delbtn.setFont(font)
        self.h_delbtn.setStyleSheet("background-color: rgba(181, 181, 181, 50%);\n"
"color: rgb(255, 255, 255);")
        self.h_delbtn.setObjectName("h_delbtn")
        self.horizontalLayout_2.addWidget(self.h_delbtn)
        self.h_loadbtn = QtWidgets.QPushButton(self.funwidget)
        self.h_loadbtn.setMinimumSize(QtCore.QSize(90, 70))
        self.h_loadbtn.setMaximumSize(QtCore.QSize(95, 72))
        font = QtGui.QFont()
        font.setPointSize(23)
        self.h_loadbtn.setFont(font)
        self.h_loadbtn.setStyleSheet("background-color: rgba(181, 181, 181, 50%);\n"
"color: rgb(255, 255, 255);")
        self.h_loadbtn.setObjectName("h_loadbtn")
        self.horizontalLayout_2.addWidget(self.h_loadbtn)
        self.verticalLayout.addLayout(self.horizontalLayout_2)
        self.h_progressBar = QtWidgets.QProgressBar(self.funwidget)
        self.h_progressBar.setProperty("value", 20)
        self.h_progressBar.setTextVisible(True)
        self.h_progressBar.setObjectName("h_progressBar")
        self.verticalLayout.addWidget(self.h_progressBar)
        self.verticalLayout_2.addWidget(self.funwidget)
        self.verticalLayout_2.setStretch(0, 2)
        self.verticalLayout_2.setStretch(1, 1)
        self.horizontalLayout_3.addLayout(self.verticalLayout_2)
        self.horizontalLayout_3.setStretch(0, 2)
        self.horizontalLayout_3.setStretch(1, 1)
        self.verticalLayout_3.addLayout(self.horizontalLayout_3)
        self.horizontalLayout_5.addWidget(self.frame)

        self.retranslateUi(ParamSetGraphic)
        QtCore.QMetaObject.connectSlotsByName(ParamSetGraphic)

    def retranslateUi(self, ParamSetGraphic):
        _translate = QtCore.QCoreApplication.translate
        ParamSetGraphic.setWindowTitle(_translate("ParamSetGraphic", "Form"))
        self.label.setText(_translate("ParamSetGraphic", "查找栏"))
        self.h_label.setText(_translate("ParamSetGraphic", "图标"))
        self.h_addnewbtn.setText(_translate("ParamSetGraphic", "新建"))
        self.h_clonebtn.setText(_translate("ParamSetGraphic", "复制"))
        self.h_delbtn.setText(_translate("ParamSetGraphic", "删除"))
        self.h_loadbtn.setText(_translate("ParamSetGraphic", "载入"))
