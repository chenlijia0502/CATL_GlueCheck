# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'tabwidgetry.ui'
#
# Created by: PyQt5 UI code generator 5.14.1
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_Tabwidget(object):
    def setupUi(self, Tabwidget):
        Tabwidget.setObjectName("Tabwidget")
        Tabwidget.resize(1137, 822)
        Tabwidget.setStyleSheet("")
        self.horizontalLayout = QtWidgets.QHBoxLayout(Tabwidget)
        self.horizontalLayout.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout.setSpacing(0)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.frame = QtWidgets.QFrame(Tabwidget)
        self.frame.setStyleSheet("QFrame#frame\n"
"{\n"
"border-image: url(res/4.jpg);\n"
"}")
        self.frame.setFrameShape(QtWidgets.QFrame.StyledPanel)
        self.frame.setFrameShadow(QtWidgets.QFrame.Raised)
        self.frame.setObjectName("frame")
        self.tabWidget_2 = QtWidgets.QTabWidget(self.frame)
        self.tabWidget_2.setGeometry(QtCore.QRect(160, 130, 891, 611))
        self.tabWidget_2.setStyleSheet("QTabWidget::pane { /* The tab widget frame */\n"
"\n"
"    position: absolute;\n"
"\n"
"}\n"
"               QTabBar::tab { \n"
"                border-color: black; \n"
"                border-width: 3px; \n"
"                border-top-left-radius: 6px; \n"
"                border-top-right-radius: 6px; \n"
"                background:lightGray; \n"
"                color:black; \n"
"                min-width:30ex; \n"
"                min-height:10ex; \n"
"                font:10pt \'Consolas\';\n"
"                } \n"
"                QTabBar::tab:selected{ \n"
"                background:rgb(69,160,178); \n"
"                color:white; \n"
"                } \n"
"                QTabBar::tab:!selected{ \n"
"                background:none; \n"
"                color:black; \n"
"                } \n"
"                QTabBar::tab:!selected:hover { \n"
"                margin-left: 5px; \n"
"                }")
        self.tabWidget_2.setTabsClosable(False)
        self.tabWidget_2.setTabBarAutoHide(False)
        self.tabWidget_2.setObjectName("tabWidget_2")
        self.tab_3 = QtWidgets.QWidget()
        self.tab_3.setStyleSheet("QWidget#tab_3\n"
"{\n"
"    border-image: url(res/4.jpg);\n"
"}")
        self.tab_3.setObjectName("tab_3")
        self.pushButton = QtWidgets.QPushButton(self.tab_3)
        self.pushButton.setGeometry(QtCore.QRect(240, 170, 75, 23))
        self.pushButton.setObjectName("pushButton")
        self.tabWidget_2.addTab(self.tab_3, "")
        self.tab_4 = QtWidgets.QWidget()
        self.tab_4.setStyleSheet("\\")
        self.tab_4.setObjectName("tab_4")
        self.tabWidget_2.addTab(self.tab_4, "")
        self.horizontalLayout.addWidget(self.frame)

        self.retranslateUi(Tabwidget)
        self.tabWidget_2.setCurrentIndex(0)
        QtCore.QMetaObject.connectSlotsByName(Tabwidget)

    def retranslateUi(self, Tabwidget):
        _translate = QtCore.QCoreApplication.translate
        Tabwidget.setWindowTitle(_translate("Tabwidget", "Form"))
        self.pushButton.setText(_translate("Tabwidget", "PushButton"))
        self.tabWidget_2.setTabText(self.tabWidget_2.indexOf(self.tab_3), _translate("Tabwidget", "Tab 1"))
        self.tabWidget_2.setTabText(self.tabWidget_2.indexOf(self.tab_4), _translate("Tabwidget", "Tab 2"))
