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
from project.mainwindow.Calibrate import FindEdgeToCalibrate, ShowCalibrateWidget
from project.mainwindow.CPCTSParam import WidgetCPCTSParam
from library.common.globalfun import DriveFreeSpace, DriveTotalSize
from project.mainwindow.MesParamTree import MesParamTreeWidget

class kxmainwindow(KXBaseMainWidget):
    _BAUDRATE = 115200
    _HARDWARE_QUEUELEN = 7
    _SIG_SHOWLOCK = QtCore.pyqtSignal()
    _SIG_ERRORINFO = QtCore.pyqtSignal(str)
    _SIG_SHOWMES  = QtCore.pyqtSignal(str)
    #_SIG_AUTORUN = QtCore.pyqtSignal()
    def __init__(self, dict_config):
        super(kxmainwindow, self).__init__(dict_config)
        self.dict_config = dict_config
        self.logger = logging.getLogger('UI.%s' % self.__class__.__name__)
        self.widget_Realtime = KxBaseMonitoringWidget.create(name=dict_config["mointoringwidget_classname"], h_parent=self)
        self.widget_Paramsetting = KxBaseParameterSetting(hparent=self, dict_config=dict_config)#参数设置
        self.widget_runlog = KxBaseRunLog(self)#日志
        self.widget_permission = kxprivilege_management(list_slevel=["账号管理", "控制工具栏",  "配方选择", "参数修改", "CP/CTS参数修改"])#权限管理
        self.widget_worklist = WorkListWidget(self)

        self.mySeria = SerialManager(h_parent=self, port=dict_config['hardwarecom'], baudrate=self._BAUDRATE, nreadbuffersize=self._HARDWARE_QUEUELEN)# 波特率比较固定，没必要配置
        self.h_control = ControlManager(self)
        self.h_control.setserial(self.mySeria)
        self.h_checkcontrolthread = CheckControlThread(self, dict_config['AGV']['IP'], dict_config['AGV']['PORT'],
                                                       dict_config['AGV']['STATION1ID'],dict_config['AGV']['STATION2ID'],
                                                       dict_config['AGV']['STATION3ID'])
        self.h_checkcontrolthread.start()

        self.pushbutton_cpcts = QtWidgets.QPushButton(self)
        self.widget_cpcts = WidgetCPCTSParam()
        self.pushbutton_cpcts.setMinimumSize(QtCore.QSize(80, 90))
        self.pushbutton_cpcts.setMaximumSize(QtCore.QSize(80, 90))
        self.pushbutton_cpcts.setText("CP/CTS")
        self.ui.horizontalLayout_2.addWidget(self.pushbutton_cpcts)

        self._initstackwidget([self.ui.pbt_realtime, self.ui.pbt_paramset, self.ui.pbt_logview, self.ui.pushButton_worklist, self.pushbutton_cpcts],
                              [self.widget_Realtime, self.widget_Paramsetting, self.widget_runlog, self.widget_worklist, self.widget_cpcts])
        self._completeui()
        self._completeconnect()
        self.ui.label_2.setText("下箱体托盘检测")
        self._setstatus("00000")
        self.h_threadlock = threading.Thread(target=self._judegIsTime2Lock)
        self.h_threadlock.start()
        self.fp = None
        self.dialog_calibrate = ShowCalibrateWidget()
        # 检查硬盘
        self.ntimeout = 1000 * 60 * 60 * 12
        self.timer = QtCore.QTimer(self)
        self.timer.timeout.connect(self.__timeout2checkdiskcapacity)
        self.timer.start(self.ntimeout)

        self.widget_mes = MesParamTreeWidget()
        self._SIG_SHOWMES.connect(self.showmeswidget)
        self.widget_mes.SIG_CHUZHAN.connect(self._control_out)
        self.spackid = str(int(time.time()))
        #self._SIG_AUTORUN.connect(self.__autorun)


    def _control_out(self):
        self.h_checkcontrolthread.emitnext()


    def showmeswidget(self, spackid):
        print ('showmeswidget')
        self.widget_mes.setpackid(spackid)
        self.widget_mes.show()


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
            self.logger.info(drive + "剩余空间不足 10%，请及时清理！！！")
            return 0
        else:
            return 1


    def _judegIsTime2Lock(self):
        while 1:
            curtime = time.time()
            diff = curtime - TimeStatus.g_curtime

            if self.widget_permission.isHidden():
                if diff > 300: #表示超过这么多秒没有操作就锁屏
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
        #self.toolbutton_move.clicked.connect(self._emitmove)
        self.toolbutton_move.clicked.connect(self._shoujian)
        self.toolbutton_test.clicked.connect(self._emitmove)
        self._SIG_SHOWLOCK.connect(self._lockpermissiondialog)
        self._SIG_ERRORINFO.connect(self._showerrorinfo)

    def _test_adjust_z(self):
        ipc_tool.kxlog("主站", logging.INFO, "测试z轴调节")
        self.serial_Reconnect()
        self.h_control.setserial(self.mySeria)
        # self.h_control.buildmodel(s_extdata)
        t = threading.Thread(target=self.h_control.control_adjust_z)
        t.start()

    def _emitmove(self):
        pass
        #self.h_checkcontrolthread.emits()


    def _setlearnstatus(self):
        if self.ui.toolbtn_learn.isChecked():
            self.widget_Paramsetting.str2paramitemfun(0, 1, "setlearnstatus", True)
        else:
            self.widget_Paramsetting.str2paramitemfun(0, 1, "setlearnstatus", False)

    def _offlinerun(self):
        if self.ui.toolbtn_offlinerun.isChecked():  # 只要点击就发送
            self.callbarck2sendnextpackid(int(time.time()))
        super(kxmainwindow, self)._offlinerun()
        if self.ui.toolbtn_offlinerun.isChecked():  # 开始离线跑
            self.widget_Realtime.clear()


    def _lockpermissiondialog(self):
        """定时切出"""
        self.ui.toolButton_userlevel.setStyleSheet(LOCK_STYLESHEET)
        self._setstatus("00000")  # 锁住
        self.widget_runlog.setid("NONE")


    def showpermissiondialog(self):
        self.ui.toolButton_userlevel.setStyleSheet(LOCK_STYLESHEET)
        self.setEnabled(False)
        self.widget_permission.clear()
        self.widget_permission.show()
        self.widget_permission.exec_()
        self.setEnabled(True)
        account = self.widget_permission.getpermissionlevel()
        if account is not None:
            self.ui.toolButton_userlevel.setStyleSheet(UNLOCK_STYLESHEET)
            self._setstatus(account[1])
            self.widget_runlog.setid(account[0])
        else:
            self._setstatus("00000")#锁住


    def _setstatus(self, slevel):
        list_status = list(map(int, slevel))
        list_bstatus = list(map(bool, list_status))
        self.ui.widget_3.setEnabled(list_bstatus[1])
        if list_bstatus[2]:
            self.widget_Paramsetting.modelmanage_load_unlock()
        else:
            self.widget_Paramsetting.modelmanage_load_lock()
        if list_bstatus[3]:
            self.widget_Paramsetting.unlock()
        else:
            self.widget_Paramsetting.lock()
        if list_bstatus[4]:
            self.widget_cpcts.setEnabled(True)
        else:
            self.widget_cpcts.setEnabled(False)



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
        elif n_msgtype == imc_msg.MSG_GET_BASE_POINT_HIGH:
            ipc_tool.kxlog("主站", logging.INFO, "开始标定高度")
            self.serial_Reconnect()
            self.h_control.setserial(self.mySeria)
            t = threading.Thread(target=self.h_control.buildmodel_getsensorhigh, args=s_extdata)
            t.start()
        elif n_msgtype == imc_msg.MSG_DOT_CHECK_RESULT:
            self.reccalibrateimg(s_extdata)



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
        self.serial_Reconnect()
        self.h_control.setserial(self.mySeria)
        t = threading.Thread(target=self.h_control.control_calibrate, args = [self.dict_config['SHOUJIAN']['XPOS'],
                                                                             self.dict_config['SHOUJIAN']['YPOS1'],
                                                                             self.dict_config['SHOUJIAN']['YPOS2']])
        t.start()


    def callback2dotcheck(self):
        self.sendmsg(0, imc_msg.MSG_DOT_CHECK_OPEN)

    def callback2stopdotcheck(self):
        self.sendmsg(0, imc_msg.MSG_DOT_CHECK_CLOSE)

    def reccalibrateimg(self,tuple_data):
        import json
        dict_result = json.loads(tuple_data)
        img = self._getimage(dict_result)
        obj = FindEdgeToCalibrate()
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
        respond = QtWidgets.QMessageBox.warning(self, u"错误", info, QtWidgets.QMessageBox.Cancel)

    def callback2getrowcol(self):
        return self.widget_Paramsetting.str2paramitemfun(0, 1, 'callback2getrowcol')

    def callback2getlisthead(self):
        return self.widget_Paramsetting.str2paramitemfun(0, 1, 'callback2getlisthead')



    def callbarck2sendnextpackid(self, packid):
        self.spackid = packid
        data = json.dumps({'packid':str(packid)})
        self.sendmsg(0, imc_msg.MSG_PACK_ID, data)


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



class CheckControlThread(threading.Thread):
    _MAX_AGV_BUFFERLEN = 100  # 赌小车答复不会一次性答复这么长
    def __init__(self, hparent, ip, port, nid1, nid2, nid3):
        super(CheckControlThread, self).__init__()
        self.h_parent = hparent
        self.b_runstaus = False
        self.b_isfirstrun = True# 是否为第一次触发，第一次触发需要人为按下开始按钮
        self.list_info = []
        self.controlmanger = None
        self.b_emit = False
        self.tcp_agvclient = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.tcp_agvclient.connect((ip, int(port)))
        self.tcp_agvclient.settimeout(2)
        self.nid1 = int(nid1)
        self.nid2 = int(nid2)
        self.nid3 = int(nid3)
        self.s_packid = str(int(time.time()))
        self.b_nextstatus = False #进入下一个站点



    def setstatus(self, bstatus):
        """
        设置运行状态，开始检测为True，停止检测为False
        :param bstatus:
        :return:
        """
        self.b_runstaus = bstatus
        if bstatus:
            self.b_isfirstrun = True


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

    def emitnext(self):
        self.b_nextstatus = True


    def str_to_hex(self, data):
        list_hex = []
        for i in range(0, len(data), 2):
            list_hex.append(int(data[i:i + 2], 16))
        return list_hex


    def list_hex_to_str(self, list_data):
        sword = ''

        for data in list_data:

            sword += chr(data)

        return sword


    def _get_status_and_packid(self, nagvstationid):
        """
        获取站点状态以及pack号
        :param nagvstationid: 站点号
        :return: 1为当前站点有车，0为没有
        """

        MSG_AGV_GETINFO = imc_msg.AGVMSG.MSG_BASE_GET_STATION_STATUS

        MSG_AGV_GETINFO[5] = nagvstationid

        self.tcp_agvclient.send(bytes(MSG_AGV_GETINFO))

        print('发送获取站点状态， 1： ', MSG_AGV_GETINFO)

        sreaddata = self.tcp_agvclient.recv(self._MAX_AGV_BUFFERLEN).hex()

        print('发送获取站点状态， 2： ', sreaddata)


        list_readdata = self.str_to_hex(sreaddata)

        if len(list_readdata) > 8 and list_readdata[8] == imc_msg.AGVMSG.AGVSTATUS_ONSTATION:

            nagvid = list_readdata[7]

            MSG_GET_PACKID = imc_msg.AGVMSG.MSG_BASE_GET_PACK_ID

            MSG_GET_PACKID[5] = nagvid

            while self.b_runstaus:

                self.tcp_agvclient.send(bytes(MSG_GET_PACKID))

                print('发送获取站点状态， 3： ', MSG_GET_PACKID)

                sreaddata = self.tcp_agvclient.recv(self._MAX_AGV_BUFFERLEN).hex()

                print('发送获取站点状态， 4： ', MSG_GET_PACKID)


                list_readdata = self.str_to_hex(sreaddata)

                if len(list_readdata) > 5 and list_readdata[4] == 0xB1:

                    nidlen = list_readdata[3]

                    list_packid = list_readdata[6:6 + nidlen - 2]

                    self.s_packid = self.list_hex_to_str(list_packid)

                    print('发送获取站点状态， 5： ', list_readdata, self.s_packid)

                    return 1

                time.sleep(1)

        else:

            return 0


    def waitforagv(self):
        """
        等待agv小车的到来
        :return:
        """
        #1. 清楚接收缓存
        try:
            try:
                self.tcp_agvclient.recv(1000)
            except Exception as e:
                pass#清除缓存

            #2. 获取中间站点小车状态
            if self._get_status_and_packid(self.nid2):

                return# 站点2内有车直接返回，并开始

            else:
                # 3. 获取进站口站点状态
                while not self._get_status_and_packid(self.nid1) and self.b_runstaus:

                    time.sleep(1)

                if not self.b_runstaus: return

                # 4. 关闭光栅放入小车
                self.controlmanger.control_guangshan(0)

                MSG_CONTROL_AGV = imc_msg.AGVMSG.MSG_BASE_CONTROL_STATION_STATUS

                MSG_CONTROL_AGV[5] = self.nid1

                self.tcp_agvclient.send(bytes(MSG_CONTROL_AGV))

                print ('放入小车: ', MSG_CONTROL_AGV)

                # 5. 检测小车到位中间站点
                while self.b_runstaus:

                    MSG_AGV_GETINFO = imc_msg.AGVMSG.MSG_BASE_GET_STATION_STATUS

                    MSG_AGV_GETINFO[5] = self.nid2

                    self.tcp_agvclient.send(bytes(MSG_AGV_GETINFO))

                    print('请求站点20状态: ', MSG_CONTROL_AGV)

                    sreaddata = self.tcp_agvclient.recv(self._MAX_AGV_BUFFERLEN).hex()

                    print('获取站点20状态: ', sreaddata)

                    list_readdata = self.str_to_hex(sreaddata)

                    if len(list_readdata) > 8 and list_readdata[8] == imc_msg.AGVMSG.AGVSTATUS_ONSTATION:

                        break

                # 6. 打开光栅
                self.controlmanger.control_guangshan(1)
        except Exception as e:
            print('等待AGV错误', e)



    def sendagv2next(self):

        while (not self.b_nextstatus) and self.b_runstaus:#等待外部将b_nextstatus置为True

            time.sleep(1)

        self.b_nextstatus = False

        if not self.b_runstaus: return

        while self.b_runstaus :

            MSG_AGV_GETINFO = imc_msg.AGVMSG.MSG_BASE_GET_STATION_STATUS

            MSG_AGV_GETINFO[5] = self.nid3

            self.tcp_agvclient.send(bytes(MSG_AGV_GETINFO))

            print('发送获取出站状态， 1： ', MSG_AGV_GETINFO)

            sreaddata = self.tcp_agvclient.recv(self._MAX_AGV_BUFFERLEN).hex()

            print('发送获取出站状态， 2： ', sreaddata)

            list_readdata = self.str_to_hex(sreaddata)

            if len(list_readdata) > 8 and (list_readdata[8] == imc_msg.AGVMSG.AGVSTATUS_FREE or
                                           list_readdata[8] == imc_msg.AGVMSG.AGVSTATUS_NONE) :

                break

        #关闭光栅
        self.controlmanger.control_guangshan(0)

        #送出小车
        MSG_CONTROL_AGV = imc_msg.AGVMSG.MSG_BASE_CONTROL_STATION_STATUS

        MSG_CONTROL_AGV[5] = self.nid2

        self.tcp_agvclient.send(bytes(MSG_CONTROL_AGV))

        #监听送出小车状态

        while self.b_runstaus:

            MSG_AGV_GETINFO = imc_msg.AGVMSG.MSG_BASE_GET_STATION_STATUS

            MSG_AGV_GETINFO[5] = self.nid3

            self.tcp_agvclient.send(bytes(MSG_AGV_GETINFO))

            print('发送获取出站状态， 1： ', MSG_AGV_GETINFO)

            sreaddata = self.tcp_agvclient.recv(self._MAX_AGV_BUFFERLEN).hex()

            print('发送获取出站状态， 2： ', sreaddata)

            list_readdata = self.str_to_hex(sreaddata)

            if len(list_readdata) > 8 and (list_readdata[8] == imc_msg.AGVMSG.AGVSTATUS_ONSTATION):

                break

            time.sleep(1)

        print('车已到达下一站点')
        #开启光栅
        self.controlmanger.control_guangshan(1)

        #开启下个状态
        self.b_emit = True



    def run(self):
        """
        执行检测需要的动作
        :return:
        """
        while (1):

            if self.b_runstaus and self.b_emit:
            #if self.b_runstaus:

                self.b_emit = False

                #复位开始按钮按下，只对开始检测后第一次有用
                self.controlmanger.waitforstart(self.b_isfirstrun)

                if not self.b_runstaus: continue

                #初始化所有气缸状态
                self.controlmanger.MakeEveryposFuwei()

                if not self.b_runstaus: continue

                #动作1, 监听设备上是否有小车
                self.waitforagv()

                if not self.b_runstaus: continue

                #发送packdi
                self.h_parent.callbarck2sendnextpackid(self.s_packid)

                #控制主流程
                self.controlmanger.check_control(self.list_info)

                self.b_isfirstrun = False

                if not self.b_runstaus: continue

                #小车出站
                #self.sendagv2next()

                #触发mes上传，同时送出packid
                #self.h_parent._SIG_SHOWMES.emit(self.s_packid)

            else:

                time.sleep(0.5)