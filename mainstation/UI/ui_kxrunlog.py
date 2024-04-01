# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui_kxrunlog.ui'
#
# Created by: PyQt5 UI code generator 5.14.1
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_RunLog(object):
    def setupUi(self, RunLog):
        RunLog.setObjectName("RunLog")
        RunLog.resize(635, 534)
        self.verticalLayout = QtWidgets.QVBoxLayout(RunLog)
        self.verticalLayout.setObjectName("verticalLayout")
        self.h_tableWidget = TableWidget(RunLog)
        self.h_tableWidget.setObjectName("h_tableWidget")
        self.h_tableWidget.setColumnCount(0)
        self.h_tableWidget.setRowCount(0)
        self.verticalLayout.addWidget(self.h_tableWidget)

        self.retranslateUi(RunLog)
        QtCore.QMetaObject.connectSlotsByName(RunLog)

    def retranslateUi(self, RunLog):
        _translate = QtCore.QCoreApplication.translate
        RunLog.setWindowTitle(_translate("RunLog", "运行日志"))
from pyqtgraph import TableWidget
