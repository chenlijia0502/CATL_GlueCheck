# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui_mainwindow_lunzhuan.ui'
#
# Created by: PyQt5 UI code generator 5.15.4
#
# WARNING: Any manual changes made to this file will be lost when pyuic5 is
# run again.  Do not edit this file unless you know what you are doing.


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_MainStation_lunzhuan(object):
    def setupUi(self, MainStation_lunzhuan):
        MainStation_lunzhuan.setObjectName("MainStation_lunzhuan")
        MainStation_lunzhuan.resize(1633, 1015)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Ignored, QtWidgets.QSizePolicy.Ignored)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(MainStation_lunzhuan.sizePolicy().hasHeightForWidth())
        MainStation_lunzhuan.setSizePolicy(sizePolicy)
        MainStation_lunzhuan.setMinimumSize(QtCore.QSize(0, 0))
        self.horizontalLayout_3 = QtWidgets.QHBoxLayout(MainStation_lunzhuan)
        self.horizontalLayout_3.setSizeConstraint(QtWidgets.QLayout.SetDefaultConstraint)
        self.horizontalLayout_3.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout_3.setSpacing(0)
        self.horizontalLayout_3.setObjectName("horizontalLayout_3")
        self.widget_2 = QtWidgets.QWidget(MainStation_lunzhuan)
        self.widget_2.setObjectName("widget_2")
        self.verticalLayout = QtWidgets.QVBoxLayout(self.widget_2)
        self.verticalLayout.setContentsMargins(0, 0, 0, 0)
        self.verticalLayout.setSpacing(0)
        self.verticalLayout.setObjectName("verticalLayout")
        self.widget_top = QtWidgets.QWidget(self.widget_2)
        self.widget_top.setStyleSheet("QWidget#widget_top\n"
"{background-color:#188bd1;}")
        self.widget_top.setObjectName("widget_top")
        self.horizontalLayout_4 = QtWidgets.QHBoxLayout(self.widget_top)
        self.horizontalLayout_4.setObjectName("horizontalLayout_4")
        self.widget_company = QtWidgets.QWidget(self.widget_top)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Minimum)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.widget_company.sizePolicy().hasHeightForWidth())
        self.widget_company.setSizePolicy(sizePolicy)
        self.widget_company.setMinimumSize(QtCore.QSize(80, 80))
        self.widget_company.setMaximumSize(QtCore.QSize(230, 200))
        self.widget_company.setStyleSheet("")
        self.widget_company.setObjectName("widget_company")
        self.horizontalLayout = QtWidgets.QHBoxLayout(self.widget_company)
        self.horizontalLayout.setContentsMargins(9, 0, 0, 0)
        self.horizontalLayout.setSpacing(0)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.label_3 = QtWidgets.QLabel(self.widget_company)
        self.label_3.setMinimumSize(QtCore.QSize(160, 0))
        self.label_3.setMaximumSize(QtCore.QSize(180, 16777215))
        self.label_3.setStyleSheet("background-color:transparent;border:0px;\n"
"\n"
"border-image: url(res/catllogo.jpg);")
        self.label_3.setText("")
        self.label_3.setObjectName("label_3")
        self.horizontalLayout.addWidget(self.label_3)
        self.horizontalLayout_4.addWidget(self.widget_company)
        self.label_2 = QtWidgets.QLabel(self.widget_top)
        self.label_2.setMinimumSize(QtCore.QSize(300, 0))
        font = QtGui.QFont()
        font.setFamily("华文中宋")
        font.setPointSize(20)
        self.label_2.setFont(font)
        self.label_2.setStyleSheet("color: rgb(255, 255, 255);")
        self.label_2.setAlignment(QtCore.Qt.AlignCenter)
        self.label_2.setObjectName("label_2")
        self.horizontalLayout_4.addWidget(self.label_2)
        self.widget_5 = QtWidgets.QWidget(self.widget_top)
        self.widget_5.setObjectName("widget_5")
        self.horizontalLayout_2 = QtWidgets.QHBoxLayout(self.widget_5)
        self.horizontalLayout_2.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout_2.setSpacing(50)
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        self.pbt_realtime = QtWidgets.QPushButton(self.widget_5)
        self.pbt_realtime.setMinimumSize(QtCore.QSize(80, 90))
        self.pbt_realtime.setMaximumSize(QtCore.QSize(80, 90))
        self.pbt_realtime.setStyleSheet("QPushButton#pbt_realtime\n"
"{background-color:transparent;border:0px;\n"
"border-image: url(res/主界面.png)\n"
"}\n"
"\n"
"\n"
"QPushButton:hover#pbt_realtime\n"
" {background-color:transparent;border:0px;\n"
"background-color:lightBlue ;\n"
"}\n"
"\n"
"QPushButton:checked#pbt_realtime\n"
" { \n"
"background-color:lightBlue;border:0px;\n"
"border-image: url(res/mainpage_normal.png)\n"
"\n"
"}\n"
"\n"
" \n"
"")
        self.pbt_realtime.setText("")
        self.pbt_realtime.setCheckable(True)
        self.pbt_realtime.setObjectName("pbt_realtime")
        self.horizontalLayout_2.addWidget(self.pbt_realtime)
        self.pushButton_worklist = QtWidgets.QPushButton(self.widget_5)
        self.pushButton_worklist.setMinimumSize(QtCore.QSize(80, 90))
        self.pushButton_worklist.setMaximumSize(QtCore.QSize(80, 90))
        font = QtGui.QFont()
        font.setFamily("Algerian")
        font.setPointSize(14)
        self.pushButton_worklist.setFont(font)
        self.pushButton_worklist.setStyleSheet("QPushButton#pushButton_worklist\n"
"{background-color:transparent;border:0px;\n"
"border-image: url(res/producerecord.png);\n"
"}\n"
"\n"
"\n"
"QPushButton:hover#pushButton_worklist\n"
" {background-color:transparent;border:0px;\n"
"background-color:lightBlue;\n"
"}\n"
"\n"
"QPushButton:checked#pushButton_worklist\n"
" { \n"
"background-color:lightBlue;border:0px;\n"
"border-image: url(res/producerecord.png);\n"
"\n"
"}\n"
"\n"
" \n"
"")
        self.pushButton_worklist.setText("")
        self.pushButton_worklist.setObjectName("pushButton_worklist")
        self.horizontalLayout_2.addWidget(self.pushButton_worklist)
        self.pbt_paramset = QtWidgets.QPushButton(self.widget_5)
        self.pbt_paramset.setMinimumSize(QtCore.QSize(80, 90))
        self.pbt_paramset.setMaximumSize(QtCore.QSize(80, 90))
        self.pbt_paramset.setStyleSheet("QPushButton#pbt_paramset\n"
"{background-color:transparent;border:0px;\n"
"border-image: url(res/参数设置.png);\n"
"}\n"
"\n"
"\n"
"QPushButton:hover#pbt_paramset\n"
" {background-color:transparent;border:0px;\n"
"background-color:lightBlue;\n"
"}\n"
"\n"
"QPushButton:checked#pbt_paramset\n"
" { \n"
"background-color:lightBlue;border:0px;\n"
"border-image: url(res/参数设置.png);\n"
"\n"
"}\n"
"\n"
" \n"
"")
        self.pbt_paramset.setText("")
        self.pbt_paramset.setCheckable(True)
        self.pbt_paramset.setObjectName("pbt_paramset")
        self.horizontalLayout_2.addWidget(self.pbt_paramset)
        self.pbt_logview = QtWidgets.QPushButton(self.widget_5)
        self.pbt_logview.setMinimumSize(QtCore.QSize(80, 90))
        self.pbt_logview.setMaximumSize(QtCore.QSize(80, 90))
        self.pbt_logview.setStyleSheet("QPushButton#pbt_logview\n"
"{background-color:transparent;border:0px;\n"
"border-image: url(res/日志记录.png);\n"
"}\n"
"\n"
"\n"
"QPushButton:hover#pbt_logview\n"
" {background-color:transparent;border:0px;\n"
"background-color:lightBlue;\n"
"}\n"
"\n"
"QPushButton:checked#pbt_logview\n"
" { \n"
"background-color:lightBlue;border:0px;\n"
"border-image: url(res/日志记录.png);\n"
"}\n"
"\n"
" \n"
"")
        self.pbt_logview.setText("")
        self.pbt_logview.setCheckable(True)
        self.pbt_logview.setObjectName("pbt_logview")
        self.horizontalLayout_2.addWidget(self.pbt_logview)
        self.horizontalLayout_4.addWidget(self.widget_5)
        spacerItem = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_4.addItem(spacerItem)
        self.widget_7 = QtWidgets.QWidget(self.widget_top)
        self.widget_7.setMinimumSize(QtCore.QSize(150, 0))
        self.widget_7.setObjectName("widget_7")
        self.label_5 = QtWidgets.QLabel(self.widget_7)
        self.label_5.setGeometry(QtCore.QRect(0, 10, 161, 31))
        font = QtGui.QFont()
        font.setFamily("Arial")
        font.setPointSize(12)
        self.label_5.setFont(font)
        self.label_5.setStyleSheet("color: rgb(255, 255, 255);")
        self.label_5.setObjectName("label_5")
        self.label_6 = QtWidgets.QLabel(self.widget_7)
        self.label_6.setGeometry(QtCore.QRect(0, 40, 161, 31))
        font = QtGui.QFont()
        font.setFamily("Arial")
        font.setPointSize(12)
        self.label_6.setFont(font)
        self.label_6.setStyleSheet("color: rgb(255, 255, 255);")
        self.label_6.setObjectName("label_6")
        self.horizontalLayout_4.addWidget(self.widget_7)
        self.label_rootnum = QtWidgets.QLabel(self.widget_top)
        self.label_rootnum.setMinimumSize(QtCore.QSize(100, 0))
        font = QtGui.QFont()
        font.setPointSize(36)
        font.setBold(True)
        font.setWeight(75)
        self.label_rootnum.setFont(font)
        self.label_rootnum.setStyleSheet("color: rgb(255, 10, 14);")
        self.label_rootnum.setText("")
        self.label_rootnum.setAlignment(QtCore.Qt.AlignCenter)
        self.label_rootnum.setObjectName("label_rootnum")
        self.horizontalLayout_4.addWidget(self.label_rootnum)
        spacerItem1 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_4.addItem(spacerItem1)
        self.toolButton_userlevel = QtWidgets.QToolButton(self.widget_top)
        self.toolButton_userlevel.setMinimumSize(QtCore.QSize(60, 60))
        self.toolButton_userlevel.setMaximumSize(QtCore.QSize(60, 60))
        self.toolButton_userlevel.setStyleSheet("QToolButton#toolButton_userlevel\n"
"{ border-image: url(res/lock.png); }  \n"
"\n"
"QToolButton:hover#toolButton_userlevel\n"
" {background-color:transparent;border:0px;\n"
"background-color:lightBlue;\n"
"}\n"
"")
        self.toolButton_userlevel.setText("")
        self.toolButton_userlevel.setToolButtonStyle(QtCore.Qt.ToolButtonFollowStyle)
        self.toolButton_userlevel.setObjectName("toolButton_userlevel")
        self.horizontalLayout_4.addWidget(self.toolButton_userlevel)
        self.widget = QtWidgets.QWidget(self.widget_top)
        self.widget.setObjectName("widget")
        self.verticalLayout_5 = QtWidgets.QVBoxLayout(self.widget)
        self.verticalLayout_5.setContentsMargins(9, 0, 0, -1)
        self.verticalLayout_5.setObjectName("verticalLayout_5")
        self.widget_6 = QtWidgets.QWidget(self.widget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.widget_6.sizePolicy().hasHeightForWidth())
        self.widget_6.setSizePolicy(sizePolicy)
        self.widget_6.setMinimumSize(QtCore.QSize(0, 0))
        self.widget_6.setLayoutDirection(QtCore.Qt.LeftToRight)
        self.widget_6.setObjectName("widget_6")
        self.horizontalLayout_5 = QtWidgets.QHBoxLayout(self.widget_6)
        self.horizontalLayout_5.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout_5.setObjectName("horizontalLayout_5")
        self.ptb_help = QtWidgets.QPushButton(self.widget_6)
        self.ptb_help.setMinimumSize(QtCore.QSize(40, 30))
        self.ptb_help.setMaximumSize(QtCore.QSize(40, 30))
        font = QtGui.QFont()
        font.setFamily("SimSun-ExtB")
        font.setPointSize(14)
        font.setBold(True)
        font.setWeight(75)
        self.ptb_help.setFont(font)
        self.ptb_help.setStyleSheet("QPushButton#ptb_help\n"
"{background-color:transparent;border:0px;\n"
"color: rgb(255, 255, 255);\n"
"\n"
"}\n"
"\n"
"\n"
"QPushButton:hover#ptb_help\n"
" {background-color:transparent;border:0px;\n"
"background-color:lightBlue;\n"
"}\n"
"\n"
"QPushButton:checked#ptb_help\n"
" { \n"
"background-color:lightBlue;border:0px;\n"
"\n"
"}\n"
" \n"
"")
        self.ptb_help.setCheckable(False)
        self.ptb_help.setObjectName("ptb_help")
        self.horizontalLayout_5.addWidget(self.ptb_help)
        self.pbt_minimize = QtWidgets.QPushButton(self.widget_6)
        self.pbt_minimize.setMinimumSize(QtCore.QSize(35, 35))
        self.pbt_minimize.setMaximumSize(QtCore.QSize(35, 35))
        self.pbt_minimize.setStyleSheet("QPushButton#pbt_minimize\n"
"{background-color:transparent;border:0px;\n"
"border-image: url(res/最小化.png)\n"
"}\n"
"\n"
"\n"
"QPushButton:hover#pbt_minimize\n"
" {background-color:transparent;border:0px;\n"
"background-color:lightBlue;\n"
"}\n"
"\n"
"QPushButton:checked#pbt_minimize\n"
" { \n"
"background-color:lightBlue;border:0px;\n"
"border-image: url(res/最小化.png)\n"
"\n"
"}\n"
"\n"
" \n"
"")
        self.pbt_minimize.setText("")
        self.pbt_minimize.setCheckable(False)
        self.pbt_minimize.setObjectName("pbt_minimize")
        self.horizontalLayout_5.addWidget(self.pbt_minimize)
        self.pbt_quit = QtWidgets.QPushButton(self.widget_6)
        self.pbt_quit.setMinimumSize(QtCore.QSize(35, 35))
        self.pbt_quit.setMaximumSize(QtCore.QSize(35, 35))
        self.pbt_quit.setStyleSheet("QPushButton#pbt_quit\n"
"{background-color:transparent;border:0px;\n"
"border-image: url(res/关闭.png)\n"
"}\n"
"\n"
"\n"
"QPushButton:hover#pbt_quit\n"
" {background-color:transparent;border:0px;\n"
"background-color:lightBlue;\n"
"}\n"
"\n"
"QPushButton:checked#pbt_quit\n"
" { \n"
"background-color:lightBlue;border:0px;\n"
"border-image: url(res/关闭.png)\n"
"\n"
"}\n"
"\n"
" \n"
"")
        self.pbt_quit.setText("")
        self.pbt_quit.setCheckable(False)
        self.pbt_quit.setObjectName("pbt_quit")
        self.horizontalLayout_5.addWidget(self.pbt_quit)
        self.verticalLayout_5.addWidget(self.widget_6)
        self.horizontalLayout_4.addWidget(self.widget)
        self.verticalLayout.addWidget(self.widget_top)
        self.widget_tool = QtWidgets.QWidget(self.widget_2)
        self.widget_tool.setObjectName("widget_tool")
        self.horizontalLayout_6 = QtWidgets.QHBoxLayout(self.widget_tool)
        self.horizontalLayout_6.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout_6.setSpacing(0)
        self.horizontalLayout_6.setObjectName("horizontalLayout_6")
        self.stackedWidget = QtWidgets.QStackedWidget(self.widget_tool)
        self.stackedWidget.setObjectName("stackedWidget")
        self.horizontalLayout_6.addWidget(self.stackedWidget)
        self.widget_4 = QtWidgets.QWidget(self.widget_tool)
        self.widget_4.setStyleSheet("background-color:#188bd1;")
        self.widget_4.setObjectName("widget_4")
        self.verticalLayout_3 = QtWidgets.QVBoxLayout(self.widget_4)
        self.verticalLayout_3.setContentsMargins(0, 0, 0, 0)
        self.verticalLayout_3.setSpacing(0)
        self.verticalLayout_3.setObjectName("verticalLayout_3")
        self.label = QtWidgets.QLabel(self.widget_4)
        self.label.setMaximumSize(QtCore.QSize(16777215, 42))
        font = QtGui.QFont()
        font.setFamily("华文仿宋")
        font.setPointSize(16)
        font.setBold(True)
        font.setWeight(75)
        self.label.setFont(font)
        self.label.setStyleSheet("\n"
"background-color: rgb(92, 195, 253);\n"
"color: rgb(255, 255, 255);\n"
"")
        self.label.setAlignment(QtCore.Qt.AlignCenter)
        self.label.setObjectName("label")
        self.verticalLayout_3.addWidget(self.label)
        self.widget_3 = QtWidgets.QWidget(self.widget_4)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.widget_3.sizePolicy().hasHeightForWidth())
        self.widget_3.setSizePolicy(sizePolicy)
        self.widget_3.setMinimumSize(QtCore.QSize(150, 0))
        self.widget_3.setMaximumSize(QtCore.QSize(150, 10000))
        font = QtGui.QFont()
        font.setKerning(False)
        self.widget_3.setFont(font)
        self.widget_3.setLayoutDirection(QtCore.Qt.LeftToRight)
        self.widget_3.setAutoFillBackground(False)
        self.widget_3.setStyleSheet("QWidget#widget_3\n"
"{background-color:#188bd1;}")
        self.widget_3.setObjectName("widget_3")
        self.verticalLayout_2 = QtWidgets.QVBoxLayout(self.widget_3)
        self.verticalLayout_2.setContentsMargins(40, 0, -1, -1)
        self.verticalLayout_2.setSpacing(9)
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.toolbtn_onlinerun = QtWidgets.QPushButton(self.widget_3)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.toolbtn_onlinerun.sizePolicy().hasHeightForWidth())
        self.toolbtn_onlinerun.setSizePolicy(sizePolicy)
        self.toolbtn_onlinerun.setMinimumSize(QtCore.QSize(80, 90))
        self.toolbtn_onlinerun.setMaximumSize(QtCore.QSize(75, 80))
        self.toolbtn_onlinerun.setLayoutDirection(QtCore.Qt.LeftToRight)
        self.toolbtn_onlinerun.setStyleSheet("QPushButton#toolbtn_onlinerun\n"
"{ border-image: url(res/开始检查.png); }  \n"
"QPushButton:hover#toolbtn_onlinerun\n"
"{ border-image: url(res/开始检查.png); \n"
"} \n"
"QPushButton:checked#toolbtn_onlinerun\n"
" { border-image: url(res/停止检查.png); } \n"
"\n"
"QPushButton{background-color:transparent;border:0px;}\n"
"\n"
"QPushButton:hover{background-color:white;border:0px;}\n"
"\n"
"QPushButtons:pressed{background-color:white;border-style:insert;}")
        self.toolbtn_onlinerun.setText("")
        self.toolbtn_onlinerun.setCheckable(True)
        self.toolbtn_onlinerun.setAutoDefault(False)
        self.toolbtn_onlinerun.setObjectName("toolbtn_onlinerun")
        self.verticalLayout_2.addWidget(self.toolbtn_onlinerun)
        self.toolbtn_offlinerun = QtWidgets.QPushButton(self.widget_3)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.toolbtn_offlinerun.sizePolicy().hasHeightForWidth())
        self.toolbtn_offlinerun.setSizePolicy(sizePolicy)
        self.toolbtn_offlinerun.setMinimumSize(QtCore.QSize(80, 90))
        self.toolbtn_offlinerun.setMaximumSize(QtCore.QSize(80, 90))
        self.toolbtn_offlinerun.setStyleSheet("QPushButton#toolbtn_offlinerun\n"
"{ border-image: url(res/开始模拟.png); }  \n"
"QPushButton:hover#toolbtn_offlinerun\n"
"{ border-image: url(res/开始模拟.png); \n"
"} \n"
"QPushButton:checked#toolbtn_offlinerun\n"
" { border-image: url(res/停止检查.png); } \n"
"\n"
"QPushButton{background-color:transparent;border:0px;}\n"
"\n"
"QPushButton:hover{background-color:white;border:0px;}\n"
"\n"
"QPushButtons:pressed{background-color:white;border-style:insert;}")
        self.toolbtn_offlinerun.setText("")
        self.toolbtn_offlinerun.setCheckable(True)
        self.toolbtn_offlinerun.setObjectName("toolbtn_offlinerun")
        self.verticalLayout_2.addWidget(self.toolbtn_offlinerun)
        self.toolbtn_savebadimage = QtWidgets.QPushButton(self.widget_3)
        self.toolbtn_savebadimage.setEnabled(True)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Maximum)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.toolbtn_savebadimage.sizePolicy().hasHeightForWidth())
        self.toolbtn_savebadimage.setSizePolicy(sizePolicy)
        self.toolbtn_savebadimage.setMinimumSize(QtCore.QSize(80, 90))
        self.toolbtn_savebadimage.setMaximumSize(QtCore.QSize(80, 90))
        self.toolbtn_savebadimage.setStyleSheet("QPushButton#toolbtn_savebadimage\n"
"{ border-image: url(res/保存数据.png); }  \n"
"\n"
"\n"
"QPushButton{background-color:transparent;border:0px;}\n"
"\n"
"QPushButton:hover{background-color:white;border:0px;}\n"
"\n"
"QPushButtons:pressed{background-color:white;border-style:insert;}")
        self.toolbtn_savebadimage.setText("")
        self.toolbtn_savebadimage.setObjectName("toolbtn_savebadimage")
        self.verticalLayout_2.addWidget(self.toolbtn_savebadimage)
        self.verticalLayout_3.addWidget(self.widget_3)
        self.horizontalLayout_6.addWidget(self.widget_4)
        self.horizontalLayout_6.setStretch(0, 10)
        self.horizontalLayout_6.setStretch(1, 1)
        self.verticalLayout.addWidget(self.widget_tool)
        self.labelwidget = QtWidgets.QWidget(self.widget_2)
        self.labelwidget.setMinimumSize(QtCore.QSize(0, 45))
        self.labelwidget.setMaximumSize(QtCore.QSize(16777215, 45))
        self.labelwidget.setStyleSheet("QWidget#labelwidget\n"
"{background-color: rgb(92, 195, 253);}")
        self.labelwidget.setObjectName("labelwidget")
        self.verticalLayout.addWidget(self.labelwidget)
        self.widget_tool.raise_()
        self.labelwidget.raise_()
        self.widget_top.raise_()
        self.horizontalLayout_3.addWidget(self.widget_2)

        self.retranslateUi(MainStation_lunzhuan)
        QtCore.QMetaObject.connectSlotsByName(MainStation_lunzhuan)

    def retranslateUi(self, MainStation_lunzhuan):
        _translate = QtCore.QCoreApplication.translate
        MainStation_lunzhuan.setWindowTitle(_translate("MainStation_lunzhuan", "Form"))
        self.label_2.setText(_translate("MainStation_lunzhuan", "涂胶气泡检查"))
        self.label_5.setText(_translate("MainStation_lunzhuan", "软件版本：V1.0"))
        self.label_6.setText(_translate("MainStation_lunzhuan", "操作用户：CATL"))
        self.ptb_help.setToolTip(_translate("MainStation_lunzhuan", "帮助"))
        self.ptb_help.setText(_translate("MainStation_lunzhuan", "关于"))
        self.pbt_minimize.setToolTip(_translate("MainStation_lunzhuan", "最小化"))
        self.pbt_quit.setToolTip(_translate("MainStation_lunzhuan", "关闭"))
        self.label.setText(_translate("MainStation_lunzhuan", "工具区"))
