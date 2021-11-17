import time
import threading
import logging
import serial
from library.mainwindow.BaseMainWindow import KXBaseMainWidget
from PyQt5 import QtGui,QtCore,QtWidgets
from PyQt5.QtWidgets import QWidget
from library.common.BaseRunLog import KxBaseRunLog
from library.monitoring.BaseMonitoringWidget import  KxBaseMonitoringWidget
from library.common.Permission_Management import kxprivilege_management
from project.monitoring import * #这行import保证实时界面能够挂载
from project.param import * #这行保证参数设置界面能挂载
from project.param.ChipParametersetting import ChipParameterSetting
from project.other.WorkList import WorkListWidget
from project.mainwindow.DotCheckResultWidget import DotCheckResultWidget
from library.ipc import ipc_tool
from project.mainwindow.ControlManager import ControlManager


class kxmainwindow(KXBaseMainWidget):
    _BAUDRATE = 115200
    def __init__(self, dict_config):
        super(kxmainwindow, self).__init__(dict_config)
        self.widget_Realtime = KxBaseMonitoringWidget.create(name=dict_config["mointoringwidget_classname"], h_parent=self)
        self.widget_Paramsetting = ChipParameterSetting(hparent=self, dict_config=dict_config)#参数设置
        self.widget_runlog = KxBaseRunLog(self)#日志
        self.widget_permission = kxprivilege_management()#权限管理
        self.widge_worklist = WorkListWidget(self)

        self.mySeria = serial.Serial(port=dict_config['hardwarecom'], baudrate=self._BAUDRATE)# 波特率比较固定，没必要配置
        self.h_control = ControlManager(self)

        self._initstackwidget([self.ui.pbt_realtime, self.ui.pbt_paramset, self.ui.pbt_logview, self.ui.pushButton_worklist],
                              [self.widget_Realtime, self.widget_Paramsetting, self.widget_runlog, self.widge_worklist])
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
        # self.toolbutton_move.clicked.connect(self._ready2dotcheck)
        # self.toolbutton_test.clicked.connect(self._ready2show_machine_move)


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
        # if n_msgtype == imc_msg.MSG_DOT_CHECK_RESULT:
        #     self.dialog = DotCheckResultWidget()
        #     self.dialog.setvalue(0.103, 0.987)
        #     self.dialog.show()
        #     self.dialog.exec_()
        if n_msgtype == imc_msg.MSG_BUILD_MODEL:
            ipc_tool.kxlog("主站", logging.INFO, "开始全局拍摄建模")
            self.serial_Reconnect()
            self.h_control.setserial(self.mySeria)
            self.h_control.buildmodel(s_extdata)


    def serial_Reconnect(self):
        """
        保障串口一定是打开状态
        :return:
        """
        if not self.mySeria.isOpen():
            self.mySeria = serial.Serial(port=self.dict_config['hardwarecom'], baudrate=self._BAUDRATE)

    def _ready2dotcheck(self):
        """启动点检或master"""
        #QtWidgets.QMessageBox.warning(self, "提示", "正在进行点检，请勿点击", QtWidgets.QMessageBox.Ok)
        #让相机去某个固定位置拍照，然后相机进行识别甄选
        t = threading.Thread(target=self._control_calibrate)
        t.start()

    def _ready2show_machine_move(self):
        """启动设备"""
        #QtWidgets.QMessageBox.warning(self, "提示", "设备正在测试运行", QtWidgets.QMessageBox.Ok)
        #让相机去某个固定位置拍照，然后相机进行识别甄选
        t = threading.Thread(target=self._controlmachine)
        t.start()


    def openCamera(self):
        pass#发送给子站，通知打开相机采集图像，并把采集图像返回

    def closeCamera(self):
        pass
