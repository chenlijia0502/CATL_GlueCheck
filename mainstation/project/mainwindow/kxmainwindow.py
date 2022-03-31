import time
import threading
import logging
import socket
from library.mainwindow.BaseMainWindow import KXBaseMainWidget
from library.common.BaseRunLog import KxBaseRunLog
from library.parametersetting.BaseParameterSetting import KxBaseParameterSetting
from library.common.usermanager.Permission_Management import kxprivilege_management
from project.monitoring import * #这行import保证实时界面能够挂载
from project.param import * #这行保证参数设置界面能挂载
from project.other.WorkList import WorkListWidget
from library.ipc import ipc_tool
from project.mainwindow.ControlManager import ControlManager
from project.mainwindow.SerialManager import SerialManager
from App import TimeStatus
from project.other.globalparam import *
from project.mainwindow.Calibrate import FindEdgeToCalibrateNew, ShowCalibrateWidget
from project.mainwindow.CPCTSParam import WidgetCPCTSParam
from library.common.globalfun import DriveFreeSpace, DriveTotalSize
#from project.mainwindow.MesParamTree import MesParamTreeWidget
from project.mes.MesParamWidget import CMesParamWidget
from library.common.globalparam import LogInfo
from project.other.WidgetMaskCheckArea import CWidgetMaskCheckArea
from project.mainwindow.UploadDialog import CUploadDialog
from project.mainwindow.CheckControlThread import CheckControlThread
from project.mainwindow.InputDialog import CInputDialog
from project.mainwindow.RecordDebugTimes import CRecordDebugTimes, DebugStatus
from project.other.WaitDialogWithText import WaitDialogWithText
from project.other.DebugWidget import CDebugWidget


class kxmainwindow(KXBaseMainWidget):
    _BAUDRATE = 115200
    _HARDWARE_QUEUELEN = 7
    _SIG_SHOWLOCK = QtCore.pyqtSignal()
    _SIG_ERRORINFO = QtCore.pyqtSignal(str)
    _SIG_NEXTPACK  = QtCore.pyqtSignal()
    _SIG_ISMASKSELECT = QtCore.pyqtSignal()
    _SIG_PACKID = QtCore.pyqtSignal(str)
    #_SIG_AUTORUN = QtCore.pyqtSignal()
    def __init__(self, dict_config):
        super(kxmainwindow, self).__init__(dict_config)
        self.dict_config = dict_config
        self.widget_Realtime = KxBaseMonitoringWidget.create(name=dict_config["mointoringwidget_classname"], h_parent=self)
        self.widget_Paramsetting = KxBaseParameterSetting(hparent=self, dict_config=dict_config)#参数设置
        self.widget_runlog = KxBaseRunLog(self)#日志
        self.widget_permission = kxprivilege_management(list_slevel=["账号管理", "控制工具栏",  "配方选择", "参数修改",
                                                                     "CP/CTS参数修改", "MES", "屏蔽检测区域"])#权限管理
        self.widget_worklist = WorkListWidget(self)


        self.mySeria = SerialManager(h_parent=self, port=dict_config['hardwarecom'], baudrate=self._BAUDRATE, nreadbuffersize=self._HARDWARE_QUEUELEN)# 波特率比较固定，没必要配置
        self.h_control = ControlManager(self)
        self.h_control.setserial(self.mySeria)
        self.h_checkcontrolthread = CheckControlThread(self, dict_config['AGV']['IP'], dict_config['AGV']['PORT'],
                                                       dict_config['AGV']['STATION1ID'],dict_config['AGV']['STATION2ID'],
                                                       dict_config['AGV']['STATION3ID'])
        self.h_checkcontrolthread.start()

        font = QtGui.QFont()
        font.setFamily("Arial")
        font.setPointSize(16)

        self.pushbutton_cpcts = QtWidgets.QPushButton(self)
        self.widget_cpcts = WidgetCPCTSParam()
        self.pushbutton_cpcts.setMinimumSize(QtCore.QSize(80, 90))
        self.pushbutton_cpcts.setMaximumSize(QtCore.QSize(80, 90))
        self.pushbutton_cpcts.setFont(font)
        self.pushbutton_cpcts.setText("CP/CTS")
        self.ui.horizontalLayout_2.addWidget(self.pushbutton_cpcts)

        self.pushbutton_mes = QtWidgets.QPushButton(self)
        self.widget_mes = CMesParamWidget()
        self.pushbutton_mes.setMinimumSize(QtCore.QSize(80, 90))
        self.pushbutton_mes.setMaximumSize(QtCore.QSize(80, 90))
        self.pushbutton_mes.setFont(font)
        self.pushbutton_mes.setText("MES")
        self.ui.horizontalLayout_2.addWidget(self.pushbutton_mes)

        self.pushbutton_mask = QtWidgets.QPushButton(self)
        list_stautus = self.widget_Paramsetting.str2paramitemfun(0, 1, "getcheckareastatus")
        self.widget_maskcheckarea = CWidgetMaskCheckArea(self, list_stautus)
        self.pushbutton_mask.setMinimumSize(QtCore.QSize(80, 90))
        self.pushbutton_mask.setMaximumSize(QtCore.QSize(80, 90))
        self.pushbutton_mask.setFont(font)
        self.pushbutton_mask.setText("屏蔽检\n测区域")
        self.ui.horizontalLayout_2.addWidget(self.pushbutton_mask)


        self.pushbutton_debug = QtWidgets.QPushButton(self)
        self.widget_debug = CDebugWidget(self)
        self.pushbutton_debug.setMinimumSize(QtCore.QSize(80, 90))
        self.pushbutton_debug.setMaximumSize(QtCore.QSize(80, 90))
        self.pushbutton_debug.setFont(font)
        self.pushbutton_debug.setText("工具\n助手")
        self.ui.horizontalLayout_2.addWidget(self.pushbutton_debug)


        self._initstackwidget([self.ui.pbt_realtime, self.ui.pbt_paramset, self.ui.pbt_logview, self.ui.pushButton_worklist,
                               self.pushbutton_cpcts, self.pushbutton_mes, self.pushbutton_mask, self.pushbutton_debug],
                              [self.widget_Realtime, self.widget_Paramsetting, self.widget_runlog, self.widget_worklist,
                               self.widget_cpcts, self.widget_mes, self.widget_maskcheckarea, self.widget_debug])
        self._completeui()
        self._completeconnect()
        self.ui.label_2.setText("下箱体托盘检测")
        self.blockstatus = True
        self._setstatus("0000000")
        self.h_threadlock = threading.Thread(target=self._judegIsTime2Lock)
        self.h_threadlock.start()
        self.fp = None
        self.dialog_calibrate = ShowCalibrateWidget()
        # 检查硬盘
        self.ntimeout = 1000 * 60 * 60 * 12
        self.timer = QtCore.QTimer(self)
        self.timer.timeout.connect(self.__timeout2checkdiskcapacity)
        self.timer.start(self.ntimeout)

        self._SIG_NEXTPACK.connect(self.widget_Realtime.clear)
        self.spackid = str(int(time.time()))

        self._SIG_ISMASKSELECT.connect(self._slot_ensureismaskselected)
        self._SIG_PACKID.connect(self.callback2sendnextpackid)

        self.loggertimer = QtCore.QTimer(self)
        self.s_today = time.strftime("%Y-%m-%d")
        self.loggertimer.timeout.connect(self._refreshloggerpath)
        self.loggertimer.start(1000*60*5)

        #调试模式
        self.modeobj = CRecordDebugTimes()
        ipc_tool.kxlog("MAIN", logging.INFO, "————————————————启动程序————————————————————")


    def _refreshloggerpath(self):
        """
        定时更新日志路径，确保日志按天更新。
        AGV、 上下位机通信、 控制流程日志
        """
        if self.s_today != time.strftime("%Y-%m-%d"):
            self.s_today = time.strftime("%Y-%m-%d")
            self.h_control.updatelog()
            self.mySeria.updatelog()
            self.h_checkcontrolthread.updatelog()


    def __timeout2checkdiskcapacity(self):
        """
        判断两个盘符剩余空间大小，如果剩余空间不足会提示，并且改变计时器定时判断的时间间隔
        :return:
        """
        drive1 = "D:\\"
        drive2 = "F:\\"
        nresult1 = self._judgeisdriveout(drive1)
        nresult2 = self._judgeisdriveout(drive2)
        if nresult1 == 1 and nresult2 == 1:
            self.timer.start(self.ntimeout)
        else:
            self.timer.start(1000 * 60 * 30)


    def _judgeisdriveout(self, drive):
        fall = DriveTotalSize(drive)
        ffree = DriveFreeSpace(drive)
        if fall == -1 and ffree == -1:
            return 1
        if (ffree / fall) < 0.1:
            QtWidgets.QMessageBox.warning(self, u"错误", "！！！" + drive + "剩余空间不足 10%，请及时清理！！！",
                                          QtWidgets.QMessageBox.Cancel)
            ipc_tool.kxlog("kxmainwindow", logging.WARN, drive + "剩余空间不足 10%，请及时清理！！！")
            return 0
        else:
            return 1


    def _judegIsTime2Lock(self):
        while 1:
            curtime = time.time()
            diff = curtime - TimeStatus.g_curtime

            if self.widget_permission.isHidden() and not self.blockstatus:
                if diff > 300: #表示超过这么多秒没有操作就锁屏
                    self.blockstatus = True
                    self._SIG_SHOWLOCK.emit()
            else:
                TimeStatus.g_curtime = time.time()

            time.sleep(1)



    def  _completeui(self):
        self.toolbutton_move = QtWidgets.QToolButton(self)
        self.toolbutton_move.setMinimumSize(QtCore.QSize(100, 100))
        self.toolbutton_move.setMaximumSize(QtCore.QSize(100, 100))
        self.toolbutton_move.setIcon(QtGui.QIcon('res/设备点检.png'))
        self.toolbutton_move.setStyleSheet('color:white;border:none;')
        self.toolbutton_move.setIconSize(QtCore.QSize(100, 90))
        self.toolbutton_move.setToolButtonStyle(QtCore.Qt.ToolButtonTextUnderIcon)
        #self.toolbutton_move.setText("首件检测")
        self.ui.verticalLayout_2.addWidget(self.toolbutton_move)

        self.toolbutton_test = QtWidgets.QToolButton(self)
        self.toolbutton_test.setMinimumSize(QtCore.QSize(100, 100))
        self.toolbutton_test.setMaximumSize(QtCore.QSize(100, 100))
        self.toolbutton_test.setIcon(QtGui.QIcon('res/设备自启测试.png'))
        self.toolbutton_test.setStyleSheet('color:white;border:none;')
        self.toolbutton_test.setIconSize(QtCore.QSize(100, 90))
        self.toolbutton_test.setToolButtonStyle(QtCore.Qt.ToolButtonTextUnderIcon)
        self.toolbutton_test.setCheckable(True)
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
        self.toolbutton_move.clicked.connect(self._shoujian)
        self.toolbutton_test.clicked.connect(self._ROOT)
        self._SIG_SHOWLOCK.connect(self._lockpermissiondialog)
        self._SIG_ERRORINFO.connect(self._showerrorinfo)


    # def _test_adjust_z(self):
    #     ipc_tool.kxlog("主站", logging.INFO, "测试z轴调节")
    #     self.serial_Reconnect()
    #     self.h_control.setserial(self.mySeria)
    #     # self.h_control.buildmodel(s_extdata)
    #     t = threading.Thread(target=self.h_control.control_adjust_z)
    #     t.start()



    def _setlearnstatus(self):
        if self.ui.toolbtn_learn.isChecked():
            self.widget_Paramsetting.str2paramitemfun(0, 1, "setlearnstatus", True)
        else:
            self.widget_Paramsetting.str2paramitemfun(0, 1, "setlearnstatus", False)

    def _offlinerun(self):
        if self.ui.toolbtn_offlinerun.isChecked():  # 只要点击就发送
            self.callback2sendnextpackid(str(int(time.time())))
        super(kxmainwindow, self)._offlinerun()
        if self.ui.toolbtn_offlinerun.isChecked():  # 开始离线跑
            self.widget_Realtime.clear()


    def _lockpermissiondialog(self):
        """定时切出"""
        self.ui.toolButton_userlevel.setStyleSheet(LOCK_STYLESHEET)
        self._setstatus("0000000")  # 锁住
        self.widget_runlog.setid("NONE")
        ipc_tool.kxlog("权限", logging.INFO, "用户自动退出")


    def showpermissiondialog(self):
        ipc_tool.kxlog("权限", logging.INFO, "用户手动退出")
        self.ui.toolButton_userlevel.setStyleSheet(LOCK_STYLESHEET)
        self.setEnabled(False)
        self.widget_permission.clear()
        self.widget_permission.show()
        self.widget_permission.exec_()
        self.blockstatus = True

        self.setEnabled(True)
        account = self.widget_permission.getpermissionlevel()
        if account is not None:
            self.blockstatus = False
            ipc_tool.kxlog("权限", logging.INFO, "用户 " + account[0] + " 刷卡登录")
            self.ui.toolButton_userlevel.setStyleSheet(UNLOCK_STYLESHEET)
            self._setstatus(account[2])
            self.widget_runlog.setid(account[0])
        else:
            self._setstatus("0000000")#锁住
            self.widget_runlog.setid("None")



    def _setstatus(self, slevel):
        list_status = list(map(int, slevel))
        list_bstatus = list(map(bool, list_status))
        self._settoolbuttonEnable(list_bstatus[1])
        if list_bstatus[2]:
            self.widget_Paramsetting.modelmanage_load_unlock()
        else:
            self.widget_Paramsetting.modelmanage_load_lock()
        if list_bstatus[3]:
            self.widget_Paramsetting.unlock()
        else:
            self.widget_Paramsetting.lock()

        self.widget_cpcts.setEnabled(bool(list_bstatus[4]))

        self.widget_mes.setEnabled(bool(list_bstatus[5]))

        self.widget_maskcheckarea.setEnabled(bool(list_bstatus[6]))


    def _settoolbuttonEnable(self, bstatus):
        self.ui.toolbtn_offlinerun.setEnabled(bstatus)
        self.ui.toolbtn_savebadimage.setEnabled(bstatus)
        self.toolbutton_move.setEnabled(bstatus)
        self.toolbutton_test.setEnabled(bstatus)


    def recmsg(self, n_stationid, n_msgtype, s_extdata=b''):
        '''
        每个界面文件初始化时都设置父窗口成员变量，并在本窗口加此方法，目的是不耦合的给子站发送消息。
        '''
        super(kxmainwindow, self).recmsg(n_stationid, n_msgtype, s_extdata)
        if n_msgtype == imc_msg.MSG_BUILD_MODEL:
            ipc_tool.kxlog("主站", logging.INFO, "开始全局拍摄建模")
            self.serial_Reconnect()
            self.h_control.setserial(self.mySeria)
            self.h_control.setcheckstatus(True)
            t = threading.Thread(target=self.h_control.buildmodel, args =s_extdata)
            t.start()
        elif n_msgtype == imc_msg.MSG_BUILD_MODEL_SECOND:
            ipc_tool.kxlog("主站", logging.INFO, "开始二次建模拍摄")
            self.serial_Reconnect()
            self.h_control.setserial(self.mySeria)
            self.h_control.setcheckstatus(True)
            t = threading.Thread(target=self.h_control.buildmodel_second, args=s_extdata)
            t.start()
        elif n_msgtype == imc_msg.MSG_GET_BASE_POINT_HIGH:
            ipc_tool.kxlog("主站", logging.INFO, "开始标定高度")
            self.serial_Reconnect()
            self.h_control.setserial(self.mySeria)
            self.h_control.setcheckstatus(True)
            t = threading.Thread(target=self.h_control.buildmodel_getsensorhigh, args=s_extdata)
            t.start()
        elif n_msgtype == imc_msg.MSG_DOT_CHECK_RESULT:
            self._closewaitdialog()
            self.reccalibrateimg(s_extdata)
        elif n_msgtype == imc_msg.MSG_SET_CHECK_MASK:
            self.widget_maskcheckarea.setcheckarea(s_extdata)
        elif n_msgtype == imc_msg.MSG_CHECK_MATCHERROR:
            if self.ui.toolbtn_onlinerun.isChecked():
                self._showerrorinfo("！！！匹配图像发生错误，疑似相机异常，请重新开始检测！！！")



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

            self.widget_Realtime.clear()

            list_posinfo = self.call2back2getcaptureinfo()

            self.h_checkcontrolthread.setinfo(list_posinfo)

            self.h_control.setcheckstatus(True)

            self.h_checkcontrolthread.setControlmanger(self.h_control)

            self.h_checkcontrolthread.setstatus(True)

            self.h_checkcontrolthread.emits()

        else:

            self.h_checkcontrolthread.setstatus(False)

            self.h_control.setcheckstatus(False)



    def changeCameraCapturedirection(self, status=False):
        if status == False:
            self.sendmsg(0, imc_msg.MSG_CHANGE_CAMERA_INFO_REVERSE)
        else:
            self.sendmsg(0, imc_msg.MSG_RECOVER_CAMERA_INFO_REVERSE)


    def _shoujian(self):
        ipc_tool.kxlog("主站", logging.INFO, "开始首件检测")
        self.threadWaitDialog = WaitDialogWithText('正在做首件，请勿点击...')
        self.threadWaitDialog.clear()
        self.threadWaitDialog.setProcessBarRange(0, 100)
        self.threadWaitDialog.show()
        QtCore.QCoreApplication.processEvents(QtCore.QEventLoop.ExcludeUserInputEvents)
        self.serial_Reconnect()
        self.h_control.setserial(self.mySeria)
        self.h_control.setcheckstatus(True)
        t = threading.Thread(target=self.h_control.control_calibrate, args = [int(self.dict_config['SHOUJIAN']['XPOS']),
                                                                             int(self.dict_config['SHOUJIAN']['YPOS1']),
                                                                             int(self.dict_config['SHOUJIAN']['YPOS2'])])
        t.start()
        # TODO : 暂时使用首件做来回跑测试
        # t = threading.Thread(target=self.h_control.testcamera)
        # t.start()


    def callback2dotcheck(self):
        self.sendmsg(0, imc_msg.MSG_DOT_CHECK_OPEN)


    def callback2stopdotcheck(self):
        self.sendmsg(0, imc_msg.MSG_DOT_CHECK_CLOSE)
        self.h_control.setcheckstatus(False)


    def reccalibrateimg(self,tuple_data):
        import json
        dict_result = json.loads(tuple_data)
        img = self._getimage(dict_result)
        obj = FindEdgeToCalibrateNew()
        solveimg, list_w, list_h, list_gray  = obj.solveimg(img)
        cv2.imwrite("d:\\test.bmp", img)

        s_word = "识别到的格子宽： " + str(list_w) + "\n\n识别到的格子高： " + str(list_h) + "\n\n识别到的标准色板灰度： " + str(list_gray)
        self.dialog_calibrate.setimg(solveimg)
        self.dialog_calibrate.settext(s_word)
        self.dialog_calibrate.show()
        self.dialog_calibrate.exec_()


    def _getimage(self, dict_result):
        try:
            readimagepath = dict_result['imagepath']
            startoffset = dict_result['startoffset']
            offsetlen = dict_result['imageoffsetlen']
        except AttributeError:
            return None
        if self.fp is None:
            try:
                self.fp = open(readimagepath, "rb")
            except IOError:
                return None
        self.fp.seek(startoffset)
        data = self.fp.read(offsetlen)
        Img = KxImageBuf()
        Img.unpack(data)
        arrImg = Img.Kximage2npArr()
        return arrImg


    def callback2showerror(self, info):
        self._SIG_ERRORINFO.emit(info)


    def _showerrorinfo(self, info):
        """
        除了要显示错误，还必须停止检测
        """
        self.ui.toolbtn_onlinerun.setChecked(False)
        self._onlinerun()
        self.h_control.ALARM()
        ipc_tool.kxlog("main", logging.ERROR, info)
        respond = QtWidgets.QMessageBox.warning(self, u"错误", info, QtWidgets.QMessageBox.Cancel)


    def callback2getrowcol(self):
        return self.widget_Paramsetting.str2paramitemfun(0, 1, 'callback2getrowcol')

    def callback2getlisthead(self):
        return self.widget_Paramsetting.str2paramitemfun(0, 1, 'callback2getlisthead')



    def callback2sendnextpackid(self, packid):
        """
        控制线程发送packid过来，并且触发界面清零
        """
        self.spackid = packid
        data = json.dumps({'packid':str(packid)})
        self.sendmsg(0, imc_msg.MSG_PACK_ID, data)
        self._SIG_NEXTPACK.emit()
        self.widget_Realtime.setsfc(self.spackid)
        self.widget_mes.setchuzhansfc(self.spackid)


    def callback2autorun(self):
        """
        自动循环检
        :return:
        """
        print('开始循环检')
        self.widget_Realtime.clear()

        self.h_checkcontrolthread.emits()
        #测试下状态
        #self._SIG_SHOWMES.emit(self.spackid)



    def __autorun(self):
        self.ui.toolbtn_offlinerun.setChecked(False)
        self._offlinerun()
        self.ui.toolbtn_offlinerun.setChecked(True)
        self._offlinerun()


    def callback2set_highsensor_point(self, list_param):
        self.widget_Paramsetting.str2paramitemfun(0, 1, 'callback2set_highsensor_point', list_param)


    def callback2uploaddata(self, bresult):
        """
        一个pack检测完成后会调用这个函数，完成两个动作
        1. 放行小车
        2. 进行mes数据的上传； bresult 如果为1 ，则检测正常，直接上传数据； 如果为0则弹框确认是正常放行还是
        """
        if not self.ui.toolbtn_onlinerun.isChecked():
            return #只有在线检测状态才执行如下

        self.h_checkcontrolthread.emitnext()#检测完成都放小车走

        if self.modeobj.debugmode == DebugStatus.STATUS1:
            if not bresult: # 调试模式以及结果为1都不进入下面循环
                self.dialog_upload = CUploadDialog()
                self.dialog_upload.show()
                self.dialog_upload.exec_()

                nstatus, user = self.dialog_upload.getstatusAnduser()
                if nstatus:
                    self.widget_mes.senddata(self.spackid, 1)  #放行
                    ipc_tool.kxlog("检测", logging.WARNING, "卡号 " + user + "对 PACKID：" + self.spackid + "进行放行操作")
                else:
                    self.widget_mes.senddata(self.spackid, 0)  # 不放行，叛废
                    ipc_tool.kxlog("检测", logging.WARNING, "卡号 " + user + "对 PACKID：" + self.spackid + "进行判废操作")

            else:
                self.widget_mes.senddata(self.spackid, 1)

        elif self.modeobj.debugmode == DebugStatus.STATUS3:

            self.widget_mes.senddata(self.spackid, 1)  #

            ipc_tool.kxlog("检测", logging.WARNING, "系统自动对 PACKID：" + self.spackid + "进行放行操作")

        elif self.modeobj.debugmode == DebugStatus.STATUS4:# 取图模式只检一次

            self.widget_mes.senddata(self.spackid, 1)  #

            ipc_tool.kxlog("检测", logging.WARNING, "取图模式，自动对 PACKID：" + self.spackid + "进行放行操作")

            self.toolbutton_test.setChecked(False)

            self._ROOT()

            self._sendnotcheck(1)

        else:#调试模式下增加记录
            self.modeobj.IncreaseDebugTimes()

            self.ui.label_rootnum.setText(str(self.modeobj.Getdebugtimes()))

        self.h_checkcontrolthread.emits()# 判断结果后触发下次逻辑





    def callback2changecheckstatus(self, list_data):

        self.widget_Paramsetting.str2paramitemfun(0, 1, 'callback2changecheckstatus', list_data)


    def callback2ensure_all_checkarea_selected(self):
        """
        控制线程回调，触发信号判断是否有屏蔽框，有屏蔽框则弹框提示，让人进行选择
        """
        self._SIG_ISMASKSELECT.emit()


    def _slot_ensureismaskselected(self):
        """
        判断是否存在屏蔽检测区域的情况，有则弹框等待确认。如果不继续检测则停止检测.
        注意：不管如何都会触发线程的函数，目的是不让线程卡死
        """
        if self.modeobj.debugmode == DebugStatus.STATUS1:
            list_data = self.widget_maskcheckarea.getcheckarea()

            if sum(list_data) > 0:

                respond = QtWidgets.QMessageBox.warning(self, u"警告", u"存在屏蔽检测区域的情况，是否继续检测？",
                                             QtWidgets.QMessageBox.Ok | QtWidgets.QMessageBox.Cancel)
                if respond == QtWidgets.QMessageBox.Cancel:

                    self.ui.toolbtn_onlinerun.setChecked(False)

                    self._onlinerun()

        self.h_checkcontrolthread.emitselected()


    def _ROOT(self):
        """
        程序进入调试模式
        """
        if self.toolbutton_test.isChecked():

            self.inputdialog = CInputDialog()
            self.inputdialog.show()
            self.inputdialog.exec_()

            if self.inputdialog.gettext() == "zs20210401":
                ipc_tool.kxlog("main", logging.WARNING, "进入调试模式")
                self.modeobj.debugmode = DebugStatus.STATUS2
                self.ui.label_rootnum.setText(str(self.modeobj.Getdebugtimes()))
                self.h_checkcontrolthread.setrootmode(True)
                self.toolbutton_test.setIcon(QtGui.QIcon(''))
                font = QtGui.QFont()
                font.setPointSizeF(30)
                font.setBold(True)
                self.toolbutton_test.setFont(font)
                self.toolbutton_test.setText("调试\n模式")
                self.toolbutton_test.setStyleSheet("""color: rgb(255, 0, 0)""")
            elif self.inputdialog.gettext() == "zs20210401auto":
                ipc_tool.kxlog("main", logging.WARNING, "进入自动放行模式")
                self.modeobj.debugmode = DebugStatus.STATUS3
                self.h_checkcontrolthread.setrootmode(False)
                self.toolbutton_test.setIcon(QtGui.QIcon(''))
                font = QtGui.QFont()
                font.setPointSizeF(30)
                font.setBold(True)
                self.toolbutton_test.setFont(font)
                self.toolbutton_test.setText("放行\n模式")
                self.toolbutton_test.setStyleSheet("""color: rgb(0, 255, 0)""")
            elif self.inputdialog.gettext() == "zs20210401pass":
                ipc_tool.kxlog("main", logging.WARNING, "进入取图模式，当前pack不检，只存图以及上传正确数据")
                self.modeobj.debugmode = DebugStatus.STATUS4
                self._sendnotcheck(0)
                self.h_checkcontrolthread.setrootmode(False)
                self.toolbutton_test.setIcon(QtGui.QIcon(''))
                font = QtGui.QFont()
                font.setPointSizeF(30)
                font.setBold(True)
                self.toolbutton_test.setFont(font)
                self.toolbutton_test.setText("取图\n模式")
                self.toolbutton_test.setStyleSheet("""color: rgb(0, 0, 255)""")
            else:
                self.modeobj.debugmode = DebugStatus.STATUS1
                self.h_checkcontrolthread.setrootmode(False)
                self.ui.label_rootnum.setText("")
                self.toolbutton_test.setChecked(False)
                self.toolbutton_test.setText("")
                self.toolbutton_test.setIcon(QtGui.QIcon('res/设备自启测试.png'))
        else:
            ipc_tool.kxlog("main", logging.INFO, "退出调试模式")
            self._sendnotcheck(1)
            self.modeobj.debugmode = DebugStatus.STATUS1
            self.ui.label_rootnum.setText("")
            self.h_checkcontrolthread.setrootmode(False)
            self.toolbutton_test.setChecked(False)
            self.toolbutton_test.setText("")
            self.toolbutton_test.setIcon(QtGui.QIcon('res/设备自启测试.png'))


    def _sendnotcheck(self, nischeck):
        """
        发送给子站，用于取图模式时不检测
        :param nischeck: 0 为不检，1为继续检测
        :return:
        """
        data = json.dumps({'ischeck':nischeck})
        self.sendmsg(0, imc_msg.GlobalMsgSend.MSG_NOT_CHECK, data)


    def callback2lockwindow(self):
        """
        当检测到并无托盘的时候，回调锁住界面
        """
        if self.widget_permission.isHidden() and not self.blockstatus:
            self.blockstatus = True
            self._SIG_SHOWLOCK.emit()
            ipc_tool.kxlog("权限", logging.INFO, "检测过程未发现托盘，用户被强制退出锁住权限")


    def getspackid(self):
        """提供其它界面调用"""
        return self.spackid

    def _closewaitdialog(self):
        QtCore.QCoreApplication.processEvents(QtCore.QEventLoop.AllEvents)

        self.threadWaitDialog.setProcessBarVal(100)

        self.threadWaitDialog.close()


    def sendagvmsg(self, msgtype):
        """
        发送agv小车信息，msgtype = 0为放小车进站，其余为出站
        """
        self.h_checkcontrolthread.sendagvmsg(msgtype)