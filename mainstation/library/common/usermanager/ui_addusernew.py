# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui_addusernew.ui'
#
# Created by: PyQt5 UI code generator 5.15.4
#
# WARNING: Any manual changes made to this file will be lost when pyuic5 is
# run again.  Do not edit this file unless you know what you are doing.


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_addusernew(object):
    def setupUi(self, addusernew):
        addusernew.setObjectName("addusernew")
        addusernew.resize(412, 419)
        addusernew.setStyleSheet("")
        self.verticalLayout = QtWidgets.QVBoxLayout(addusernew)
        self.verticalLayout.setObjectName("verticalLayout")
        spacerItem = QtWidgets.QSpacerItem(20, 40, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        self.verticalLayout.addItem(spacerItem)
        self.widget_8 = QtWidgets.QWidget(addusernew)
        self.widget_8.setObjectName("widget_8")
        self.horizontalLayout_7 = QtWidgets.QHBoxLayout(self.widget_8)
        self.horizontalLayout_7.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout_7.setSpacing(0)
        self.horizontalLayout_7.setObjectName("horizontalLayout_7")
        spacerItem1 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_7.addItem(spacerItem1)
        self.label_2 = QtWidgets.QLabel(self.widget_8)
        font = QtGui.QFont()
        font.setFamily("Arial")
        font.setPointSize(18)
        font.setBold(False)
        font.setWeight(50)
        self.label_2.setFont(font)
        self.label_2.setObjectName("label_2")
        self.horizontalLayout_7.addWidget(self.label_2)
        self.userlineEdit_name = QtWidgets.QLineEdit(self.widget_8)
        self.userlineEdit_name.setObjectName("userlineEdit_name")
        self.horizontalLayout_7.addWidget(self.userlineEdit_name)
        spacerItem2 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_7.addItem(spacerItem2)
        self.verticalLayout.addWidget(self.widget_8)
        self.widget = QtWidgets.QWidget(addusernew)
        self.widget.setObjectName("widget")
        self.horizontalLayout = QtWidgets.QHBoxLayout(self.widget)
        self.horizontalLayout.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout.setSpacing(0)
        self.horizontalLayout.setObjectName("horizontalLayout")
        spacerItem3 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem3)
        self.label = QtWidgets.QLabel(self.widget)
        font = QtGui.QFont()
        font.setFamily("Arial")
        font.setPointSize(18)
        font.setBold(False)
        font.setWeight(50)
        self.label.setFont(font)
        self.label.setObjectName("label")
        self.horizontalLayout.addWidget(self.label)
        self.userlineEdit = QtWidgets.QLineEdit(self.widget)
        self.userlineEdit.setObjectName("userlineEdit")
        self.horizontalLayout.addWidget(self.userlineEdit)
        spacerItem4 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem4)
        self.verticalLayout.addWidget(self.widget)
        self.widget_2 = QtWidgets.QWidget(addusernew)
        self.widget_2.setObjectName("widget_2")
        self.horizontalLayout_4 = QtWidgets.QHBoxLayout(self.widget_2)
        self.horizontalLayout_4.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout_4.setSpacing(0)
        self.horizontalLayout_4.setObjectName("horizontalLayout_4")
        spacerItem5 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_4.addItem(spacerItem5)
        self.label_3 = QtWidgets.QLabel(self.widget_2)
        font = QtGui.QFont()
        font.setFamily("Arial")
        font.setPointSize(18)
        font.setBold(False)
        font.setWeight(50)
        self.label_3.setFont(font)
        self.label_3.setObjectName("label_3")
        self.horizontalLayout_4.addWidget(self.label_3)
        self.comboBox = QtWidgets.QComboBox(self.widget_2)
        self.comboBox.setMinimumSize(QtCore.QSize(0, 37))
        self.comboBox.setObjectName("comboBox")
        self.horizontalLayout_4.addWidget(self.comboBox)
        spacerItem6 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_4.addItem(spacerItem6)
        self.horizontalLayout_4.setStretch(0, 1)
        self.horizontalLayout_4.setStretch(1, 2)
        self.horizontalLayout_4.setStretch(2, 2)
        self.horizontalLayout_4.setStretch(3, 1)
        self.verticalLayout.addWidget(self.widget_2)
        spacerItem7 = QtWidgets.QSpacerItem(20, 40, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        self.verticalLayout.addItem(spacerItem7)
        self.widget_3 = QtWidgets.QWidget(addusernew)
        self.widget_3.setObjectName("widget_3")
        self.horizontalLayout_2 = QtWidgets.QHBoxLayout(self.widget_3)
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        self.pushButton_confirm = QtWidgets.QPushButton(self.widget_3)
        self.pushButton_confirm.setMinimumSize(QtCore.QSize(0, 35))
        self.pushButton_confirm.setObjectName("pushButton_confirm")
        self.horizontalLayout_2.addWidget(self.pushButton_confirm)
        spacerItem8 = QtWidgets.QSpacerItem(152, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_2.addItem(spacerItem8)
        self.pushButton_cancel = QtWidgets.QPushButton(self.widget_3)
        self.pushButton_cancel.setMinimumSize(QtCore.QSize(0, 35))
        self.pushButton_cancel.setObjectName("pushButton_cancel")
        self.horizontalLayout_2.addWidget(self.pushButton_cancel)
        spacerItem9 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_2.addItem(spacerItem9)
        self.pushButton_level = QtWidgets.QPushButton(self.widget_3)
        self.pushButton_level.setMinimumSize(QtCore.QSize(0, 35))
        self.pushButton_level.setObjectName("pushButton_level")
        self.horizontalLayout_2.addWidget(self.pushButton_level)
        spacerItem10 = QtWidgets.QSpacerItem(152, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_2.addItem(spacerItem10)
        self.pushButton_sub = QtWidgets.QPushButton(self.widget_3)
        self.pushButton_sub.setMinimumSize(QtCore.QSize(0, 35))
        self.pushButton_sub.setObjectName("pushButton_sub")
        self.horizontalLayout_2.addWidget(self.pushButton_sub)
        self.horizontalLayout_2.setStretch(0, 1)
        self.horizontalLayout_2.setStretch(1, 1)
        self.horizontalLayout_2.setStretch(2, 1)
        self.horizontalLayout_2.setStretch(3, 1)
        self.horizontalLayout_2.setStretch(4, 1)
        self.horizontalLayout_2.setStretch(5, 1)
        self.horizontalLayout_2.setStretch(6, 1)
        self.verticalLayout.addWidget(self.widget_3)

        self.retranslateUi(addusernew)
        QtCore.QMetaObject.connectSlotsByName(addusernew)

    def retranslateUi(self, addusernew):
        _translate = QtCore.QCoreApplication.translate
        addusernew.setWindowTitle(_translate("addusernew", "Form"))
        self.label_2.setText(_translate("addusernew", "用户名称    "))
        self.label.setText(_translate("addusernew", "用户卡号    "))
        self.label_3.setText(_translate("addusernew", "权限等级"))
        self.pushButton_confirm.setText(_translate("addusernew", "确定添加"))
        self.pushButton_cancel.setText(_translate("addusernew", "取消"))
        self.pushButton_level.setText(_translate("addusernew", "等级管理"))
        self.pushButton_sub.setText(_translate("addusernew", "删除账号"))
