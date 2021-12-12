import time
import threading
import logging
import serial
from library.mainwindow.BaseMainWindow import KXBaseMainWidget
from PyQt5 import QtGui,QtCore,QtWidgets
from PyQt5.QtWidgets import QWidget
from library.common.BaseRunLog import KxBaseRunLog
from library.monitoring.BaseMonitoringWidget import  KxBaseMonitoringWidget
from library.parametersetting.BaseParameterSetting import KxBaseParameterSetting
from library.common.Permission_Management import kxprivilege_management
from project.monitoring import * #这行import保证实时界面能够挂载
from project.param import * #这行保证参数设置界面能挂载
from project.other.WorkList import WorkListWidget
from project.mainwindow.DotCheckResultWidget import DotCheckResultWidget
from library.ipc import ipc_tool
from project.mainwindow.ControlManager import ControlManager
from project.mainwindow.SerialManager import SerialManager


class kxmainwindow(KXBaseMainWidget):
    _BAUDRATE = 115200
    _HARDWARE_QUEUELEN = 7
    def __init__(self, dict_config):
        super(kxmainwindow, self).__init__(dict_config)
        self.widget_Realtime = KxBaseMonitoringWidget.create(name=dict_config["mointoringwidget_classname"], h_parent=self)
        self.widget_Paramsetting = KxBaseParameterSetting(hparent=self, dict_config=dict_config)#参数设置
        self.widget_runlog = KxBaseRunLog(self)#日志
        self.widget_permission = kxprivilege_management()#权限管理
        self.widget_worklist = WorkListWidget(self)

        self.mySeria = SerialManager(h_parent=self, port=dict_config['hardwarecom'], baudrate=self._BAUDRATE, nreadbuffersize=self._HARDWARE_QUEUELEN)# 波特率比较固定，没必要配置
        self.h_control = ControlManager(self)
        self.h_control.setserial(self.mySeria)
        self.h_checkcontrolthread = CheckControlThread()
        self.h_checkcontrolthread.start()

        self._initstackwidget([self.ui.pbt_realtime, self.ui.pbt_paramset, self.ui.pbt_logview, self.ui.pushButton_worklist],
                              [self.widget_Realtime, self.widget_Paramsetting, self.widget_runlog, self.widget_worklist])
        self._completeui()
        self._completeconnect()
        self.ui.label_2.setText("下箱体托盘检测")



    def  _completeui(self):
        self.toolbutton_move = QtWidgets.QToolButton(self)
        self.toolbutton_move.setMinimumSize(QtCore.QSize(100, 100))
        self.toolbutton_move.setMaximumSize(QtCore.QSize(100, 100))
        self.toolbutton_move.setIcon(QtGui.QIcon('res/设备点检.png'))
        self.toolbutton_move.setStyleSheet('color:white;border:none;')
        self.toolbutton_move.setIconSize(QtCore.QSize(100, 90))
        self.toolbutton_move.setToolButtonStyle(QtCore.Qt.ToolButtonTextUnderIcon)
        self.ui.verticalLayout_2.addWidget(self.toolbutton_move)

        self.toolbutton_test = QtWidgets.QToolButton(self)
        self.toolbutton_test.setMinimumSize(QtCore.QSize(100, 100))
        self.toolbutton_test.setMaximumSize(QtCore.QSize(100, 100))
        self.toolbutton_test.setIcon(QtGui.QIcon('res/设备自启测试.png'))
        self.toolbutton_test.setStyleSheet('color:white;border:none;')
        self.toolbutton_test.setIconSize(QtCore.QSize(100, 90))
        self.toolbutton_test.setToolButtonStyle(QtCore.Qt.ToolButtonTextUnderIcon)
        self.ui.verticalLayout_2.addWidget(self.toolbutton_test)

        self.QPixmap_disMES = QtGui.QPixmap('res\\MES.png')
        self.label_MES = QtWidgets.QLabel(self.ui.labelwidget)  # 硬件网络连接
        self.label_MES.setAlignment(QtCore.Qt.AlignCenter)
        self.label_MES.setFixedWidth(70)
        self.label_MES.setPixmap(self.QPixmap_disMES)
        self.statusBar.addWidget(self.label_MES)

        #self.ui.toolButton_userlevel.setStyleSheet('color:white;border:none;')
        #self.ui.toolButton_userlevel.setIconSize(QtCore.QSize(100, 90))
        #self.ui.toolButton_userlevel.setToolButtonStyle(QtCore.Qt.ToolButtonTextUnderIcon)
        font = QtGui.QFont()
        font.setFamily("Arial")
        font.setPointSize(14)
        self.ui.toolButton_userlevel.setFont(font)

    def _completeconnect(self):
        self.ui.toolButton_userlevel.clicked.connect(self.showpermissiondialog)
        self.toolbutton_move.clicked.connect(self._emitmove)

    def _emitmove(self):
        self.h_checkcontrolthread.emits()


    def _setlearnstatus(self):
        if self.ui.toolbtn_learn.isChecked():
            self.widget_Paramsetting.str2paramitemfun(0, 1, "setlearnstatus", True)
        else:
            self.widget_Paramsetting.str2paramitemfun(0, 1, "setlearnstatus", False)

    def _offlinerun(self):
        super(kxmainwindow, self)._offlinerun()
        if self.ui.toolbtn_offlinerun.isChecked():  # 开始离线跑
            self.widget_Realtime.clear()

    def showpermissiondialog(self):
        self.widget_permission.setpasswordpath("d:\\")
        self.widget_permission.exec_()
        permissonlevel = self.widget_permission.getpermissionlevel()
        self.updatepermission(permissonlevel)

    def updatepermission(self, PERMISSIONLEVEL):

        self.ui.toolButton_userlevel.setStyleSheet( 'color:white;border:none;')
        if PERMISSIONLEVEL == 0:
            self.ui.toolButton_userlevel.setText("操作员")
        elif PERMISSIONLEVEL == 1:
            self.ui.toolButton_userlevel.setText("ME")
        elif PERMISSIONLEVEL == 2:
            self.ui.toolButton_userlevel.setText("IMD")
        else:
            self.ui.toolButton_userlevel.setText("管理员")

    def recmsg(self, n_stationid, n_msgtype, s_extdata=b''):
        '''
        每个界面文件初始化时都设置父窗口成员变量，并在本窗口加此方法，目的是不耦合的给子站发送消息。
        '''
        super(kxmainwindow, self).recmsg(n_stationid, n_msgtype, s_extdata)
        if n_msgtype == imc_msg.MSG_BUILD_MODEL:
            ipc_tool.kxlog("主站", logging.INFO, "开始全局拍摄建模")
            self.serial_Reconnect()
            self.h_control.setserial(self.mySeria)
            #self.h_control.buildmodel(s_extdata)
            t = threading.Thread(target=self.h_control.buildmodel, args =s_extdata)
            t.start()
        elif n_msgtype == imc_msg.MSG_BUILD_MODEL_SECOND:
            ipc_tool.kxlog("主站", logging.INFO, "开始二次建模拍摄")
            self.serial_Reconnect()
            self.h_control.setserial(self.mySeria)
            t = threading.Thread(target=self.h_control.buildmodel_second, args=s_extdata)
            t.start()


    def serial_Reconnect(self):
        """
        保障串口一定是打开状态
        :return:
        """
        if not self.mySeria.isOpen():
            #self.mySeria = serial.Serial(port=self.dict_config['hardwarecom'], baudrate=self._BAUDRATE)
            self.mySeria.reconnect()


    def callback2changecol(self):
        """
        回调告知已准备切换列，通知参数界面可进行新列图像拼接
        :return:
        """
        self.widget_Paramsetting.str2paramitemfun(0, 1, 'callback2changecol')

    def callback2showbigimg(self):
        """
        回调告知全部拍完，进行演示
        :return:
        """
        self.widget_Paramsetting.str2paramitemfun(0, 1, 'callback2showbigimg')

    def callback2judgeisfull(self):
        """
        判断当前列是否拼接完
        :return:
        """
        return self.widget_Paramsetting.str2paramitemfun(0, 1, 'callback2judgeisfull')

    def callback2changecol_second(self):
        """
        二次取图建模，回调告知已准备切换列，通知参数界面可进行新列图像拼接
        :return:
        """
        self.widget_Paramsetting.str2paramitemfun(0, 1, 'callback2changecol_second')

    def callback2showbigimg_second(self):
        """
        二次取图建模，回调告知全部拍完，进行演示
        :return:
        """
        self.widget_Paramsetting.str2paramitemfun(0, 1, 'callback2showbigimg_second')

    def callback2judgeisfull_second(self):
        """
        二次取图建模，判断当前列是否拼接完
        :return:
        """
        return self.widget_Paramsetting.str2paramitemfun(0, 1, 'callback2judgeisfull_second')


    def openCamera(self):
        #发送给子站，通知打开相机采集图像，并把采集图像返回
        self.sendmsg(0, imc_msg.MSG_JUST_OPENCAMERA_BUILDMODEL)


    def closeCamera(self):
        self.sendmsg(0, imc_msg.MSG_JUST_CLOSECAMERA_BUILDMODEL)


    def onlyopencamera(self):
        self.sendmsg(0, imc_msg.MSG_JUST_OPENCAMERA)


    def sendmsg_changecol(self):
        self.sendmsg(0, imc_msg.MSG_CHANGE_CAPTURE_COL)


    def callback2warnguangshan(self):
        """
        串口收到光栅信号，回调报警
        :return:
        """
        print("callback2warnguangshan")


    def call2back2getcaptureinfo(self):
        """
        获取检测所需移动位置参数
        :return:
        """

        return self.widget_Paramsetting.str2paramitemfun(0, 1, 'call2back2getcaptureinfo')


    def _onlinerun(self):
        super(kxmainwindow, self)._onlinerun()

        if self.ui.toolbtn_onlinerun.isChecked():  # 开始检测

            list_posinfo = self.call2back2getcaptureinfo()

            self.h_checkcontrolthread.setinfo(list_posinfo)

            self.h_control.setcheckstatus(True)

            self.h_checkcontrolthread.setControlmanger(self.h_control)

            self.h_checkcontrolthread.setstatus(True)

        else:

            self.h_checkcontrolthread.setstatus(False)

            self.h_control.setcheckstatus(False)




    def changeCameraCapturedirection(self, status=False):
        if status == False:
            self.sendmsg(0, imc_msg.MSG_CHANGE_CAMERA_INFO_REVERSE)
        else:
            self.sendmsg(0, imc_msg.MSG_RECOVER_CAMERA_INFO_REVERSE)




class CheckControlThread(threading.Thread):
    def __init__(self):
        super(CheckControlThread, self).__init__()
        self.b_runstaus = False
        self.list_info = []
        self.controlmanger = None
        self.b_emit = False


    def setstatus(self, bstatus):
        """
        设置运行状态，开始检测为True，停止检测为False
        :param bstatus:
        :return:
        """
        self.b_runstaus = bstatus


    def setinfo(self, list_info):
        """
        设置运动路径
        :param  list_info:  [list_x, list_y]
        :return:
        """
        self.list_info = list_info

    def setControlmanger(self, serials:ControlManager):

        self.controlmanger = serials

    def emits(self):
        self.b_emit = True

    def run(self):
        """
        执行检测需要的动作
        :return:
        """
        while (1):

            if self.b_runstaus and self.b_emit:

                self.b_emit = False

                #动作1


                self.controlmanger.check_control(self.list_info)

                #动作完判断
                if not self.b_runstaus: continue

                #动作2

                #..........

            else:

                time.sleep(0.5)