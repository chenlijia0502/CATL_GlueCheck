#coding: utf-8

import sys
import threading
import time
import struct
from PyQt5 import QtGui,QtCore,QtWidgets

sys.path.append('../')
# from abc import ABCMeta, abstractmethod #虚函数方法
from UI.ui_mainstation import Ui_MainStation
import imc_msg
from library.mainwindow.KxLineEditDialog import KxLineEditDialog
from UI.ui_mainwindow_lunzhuan import Ui_MainStation_lunzhuan
from globalparam import ChineseWord
from library.common.globalfun import *
"""
date:           2019.09.17
author:         HYH
Description:    

"""

class KXBaseMainWidget(QtWidgets.QWidget):
    """
    主窗口，几个界面都是挂在这。并且功能区的逻辑功能也是在这挂住基本的实现
    """
    sig_mainwindows = QtCore.pyqtSignal(int, int, object)#主界面相关数据
    sig_netconnectchange = QtCore.pyqtSignal(int, bool)#网络连接情况
    sig_hardware_netconnectchange = QtCore.pyqtSignal(bool)#硬件连接情况
    sig_hardware_receive = QtCore.pyqtSignal(tuple)#接收到的硬件信息
    def __init__(self, dict_config):
        """
        Parameters
        ----------
        dict_config 配置字典
        """
        super(KXBaseMainWidget, self).__init__()
        #公共队列及重要参数
        self.value_runflag = ipc_tool.getvalue_runflag()# 是否处于在线检测，主要是用于多进程的判断
        self.n_substationcount = int(len(dict_config['substation'])) #子站数
        self.n_isengineermode = int(dict_config['isengineermode'])#设置是否工程师模式
        self.dict_config = dict_config
        if int(dict_config['developer']['Ui_stytle']) == 0:
            self.ui = Ui_MainStation()
            self.ui.setupUi(self)
        else:
            self.ui = Ui_MainStation_lunzhuan()
            self.ui.setupUi(self)
        self.s_offlinerunpath = ''
        self.s_saveallimagepath = ''
        self.s_savebadimagepath = ''
        #定时器，子站断线重启用
        self.timer_substation = QtCore.QTimer()
        self.timer_substation.timeout.connect(self._restartsubstation_timer)
        #状态栏相关
        self.list_netconnnectlabel = [] #子站连接情况图标类列表
        self.list_camerastatuslabel = [] #相机连接情况图标类列表
        self.label_hardwarenetconnnect = None #硬件连接情况图标类
        self._createstatusbar()

        #参数
        self.list_isready_camera = [] #相机是否准备好，通过子站的反馈
        for i in range(self.n_substationcount):
            self.list_isready_camera.append(False)

        #关键的界面
        self._initconnect()# 启动信号槽连接
        self._init_msgthread()
        self.ui.toolbtn_onlinerun.setEnabled(False)
        self.setWindowFlags(QtCore.Qt.FramelessWindowHint)#全屏


    def _receive_hardware_msg(self, tuple_data):
        """
        :fun:                   接收到硬件消息
        :param tuple_data:
        :return:
        """
        pass

    def _help(self):
        """
        这个函数弹出操作手册以及建模文档
        """
        # helpdialog = KxHelp(self)
        # helpdialog.exec_()




    def _initstackwidget(self, list_widgetpbt, list_widgetobj):
        """
        初始化statckwidget，目的是使按钮跟显示某个界面挂钩，也即点击某个按钮使挂钩界面显示

        Parameters
        ----------
        list_widgetpbt : list
            按钮列表
        list_widgetobj : list
            挂钩类列表
        """
        if not isinstance(list_widgetpbt[0], QtWidgets.QPushButton):
            return -1
        if not isinstance(list_widgetobj[0], QtWidgets.QWidget):
            return -1
        for nindex in range(len(list_widgetpbt)):
            self._pushbutton_connect_stackedwidget(list_widgetpbt[nindex], list_widgetobj[nindex])
            list_widgetpbt[nindex].clicked.connect(self._slot_showspecif_widget)

    def _pushbutton_connect_stackedwidget(self, pushbutton, widget):
        """
        这个函数将按钮跟放在statckedwidget上的widget挂起钩，做法是赋予其两个属性

        Parameters
        ----------
        pushbutton : QPushbutton
            按钮对象
        widget : QWidget
            widget对象，也即点击按钮时要显示的界面
        """
        pushbutton.correspondWidget = widget
        widget.correspondBtn = pushbutton
        self.ui.stackedWidget.addWidget(widget)

    def _slot_showspecif_widget(self):
        """
        信号槽，判断是哪个按钮点击，并作出显示当前界面的动作
        """
        currentWidget = self.ui.stackedWidget.currentWidget()
        if hasattr(self.sender(), 'correspondWidget') and hasattr(currentWidget, 'correspondBtn'):
            self.ui.stackedWidget.setCurrentWidget(self.sender().correspondWidget)
            currentWidget.correspondBtn.setChecked(False)
            self.sender().setChecked(True)

    """
         function: 系统结构相关，包括状态栏、连接信号、显示窗口的初始化
     """

    def _createstatusbar(self):
        """
        创建状态条，相当于UI补充，根据配置的个数增加相机、硬件、子站图标
        """

        self.QPixmap_disconnect = QtGui.QPixmap('res\\子站断开.png')
        self.QPixmap_connect = QtGui.QPixmap('res\\子站连接.png')
        self.QPixmap_discamera = QtGui.QPixmap('res\\相机断开.png')
        self.QPixmap_camera = QtGui.QPixmap('res\\相机连接.png')
        self.QPixmap_dishardware = QtGui.QPixmap('res\\下位机断开.png')
        self.QPixmap_hardware = QtGui.QPixmap('res\\下位机连接.jpg')

        ## old method
        # self.horizontalLayout_labwidget = QtWidgets.QHBoxLayout(self.ui.labelwidget)
        # self.horizontalLayout_labwidget.setSpacing(0)
        # self.horizontalLayout_labwidget.setContentsMargins(0, 0, 0, 0)
        # for pos in range(self.n_substationcount):  # 子站
        #     self.list_netconnnectlabel.append(QtWidgets.QLabel(self.ui.labelwidget))
        #     self.list_netconnnectlabel[pos].setAlignment(QtCore.Qt.AlignCenter)
        #     self.list_netconnnectlabel[pos].setFixedWidth(70)
        #     self.list_netconnnectlabel[pos].setPixmap(self.QPixmap_disconnect)
        #     self.horizontalLayout_labwidget.addWidget(self.list_netconnnectlabel[pos])
        #
        # for pos in range(self.n_substationcount):  # 相机
        #     self.list_camerastatuslabel.append(QtWidgets.QLabel(self.ui.labelwidget))
        #     self.list_camerastatuslabel[pos].setAlignment(QtCore.Qt.AlignCenter)
        #     self.list_camerastatuslabel[pos].setFixedWidth(70)
        #     self.list_camerastatuslabel[pos].setPixmap(self.QPixmap_discamera)
        #     self.horizontalLayout_labwidget.addWidget(self.list_camerastatuslabel[pos])
        #
        # self.label_hardwarenetconnnect = QtWidgets.QLabel(self.ui.labelwidget)  # 硬件网络连接
        # self.label_hardwarenetconnnect.setAlignment(QtCore.Qt.AlignCenter)
        # self.label_hardwarenetconnnect.setFixedWidth(70)
        # self.label_hardwarenetconnnect.setPixmap(self.QPixmap_dishardware)
        # self.horizontalLayout_labwidget.addWidget(self.label_hardwarenetconnnect)
        #
        # spacerItem = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        # self.horizontalLayout_labwidget.addItem(spacerItem)
        # self.label_statusinf = QtWidgets.QLabel(self.ui.labelwidget)  # 状态信息
        # self.horizontalLayout_labwidget.addWidget(self.label_statusinf)
        # nstrechall = len(self.list_netconnnectlabel) + len(self.list_camerastatuslabel) + 1 + 1
        # self.horizontalLayout_labwidget.setStretch(nstrechall, 1)

        # new method
        self.statusBar = QtWidgets.QStatusBar(self)

        self.horizontalLayout_labwidget = QtWidgets.QHBoxLayout(self.ui.labelwidget)
        self.horizontalLayout_labwidget.setSpacing(0)
        self.horizontalLayout_labwidget.setContentsMargins(0, 0, 0, 0)
        for pos in range(self.n_substationcount):  # 子站
            self.list_netconnnectlabel.append(QtWidgets.QLabel(self.ui.labelwidget))
            self.list_netconnnectlabel[pos].setAlignment(QtCore.Qt.AlignCenter)
            self.list_netconnnectlabel[pos].setFixedWidth(70)
            self.list_netconnnectlabel[pos].setPixmap(self.QPixmap_disconnect)
            self.statusBar.addWidget(self.list_netconnnectlabel[pos], 1)

        for pos in range(self.n_substationcount):  # 相机
            self.list_camerastatuslabel.append(QtWidgets.QLabel(self.ui.labelwidget))
            self.list_camerastatuslabel[pos].setAlignment(QtCore.Qt.AlignCenter)
            self.list_camerastatuslabel[pos].setFixedWidth(70)
            self.list_camerastatuslabel[pos].setPixmap(self.QPixmap_discamera)
            self.statusBar.addWidget(self.list_camerastatuslabel[pos])

        self.label_hardwarenetconnnect = QtWidgets.QLabel(self.ui.labelwidget)  # 硬件网络连接
        self.label_hardwarenetconnnect.setAlignment(QtCore.Qt.AlignCenter)
        self.label_hardwarenetconnnect.setFixedWidth(70)
        self.label_hardwarenetconnnect.setPixmap(self.QPixmap_dishardware)
        self.statusBar.addWidget(self.label_hardwarenetconnnect)

        self.label_statusinf = QtWidgets.QLabel(self.ui.labelwidget)  # 状态信息
        self.horizontalLayout_labwidget.addWidget(self.statusBar)
        self.horizontalLayout_labwidget.addWidget(self.label_statusinf)
        self.horizontalLayout_labwidget.setStretch(1, 10)


    def _initconnect(self):
        """
        在这里连接几个基本的功能，比如开始、停止、模拟、存图等功能
        """
        self._init_commuicate_connection()
        self.ui.ptb_help.clicked.connect(self._help)
        self.ui.pbt_quit.clicked.connect(self.close)
        self.ui.toolbtn_onlinerun.clicked.connect(self._onlinerun)
        self.ui.toolbtn_offlinerun.clicked.connect(self._offlinerun)
        self.ui.toolbtn_savebadimage.clicked.connect(self._saveimg)
        self.ui.pbt_minimize.clicked.connect(self.showMinimized)


    def _init_commuicate_connection(self):
        """
        连接通信信号，通信信号一般是与线程中的函数相挂载，比如硬件进程中心跳无回复，则讲命令发送到
        线程中，线程捕捉到异常，触发信号
        """
        self.sig_mainwindows.connect(self.recmsg)
        self.sig_netconnectchange.connect(self._receive_netconnectchange_msg)
        self.sig_hardware_netconnectchange.connect(self._receive_hardware_netconnectchange_msg)
        self.sig_hardware_receive.connect(self._receive_hardware_msg)


    def _receive_netconnectchange_msg(self, n_stationid, bool_status):
        """
        各子站网络连接状态改变槽函数(线程中一直判断，并触发该函数)

        Parameters
        ----------
        n_pos : int
            站号
        bool_status : int
            状态
        """
        #print ('子站连接情况，', bool_status)
        if bool_status:
            self.timer_substation.stop()
            self.list_netconnnectlabel[n_stationid].setPixmap(self.QPixmap_connect)

        else:
            if not self.timer_substation.isActive() and not int(self.dict_config['isengineermode']):
                self.timer_substation.start(5*1000)#5s 重启一次子站

            self.list_netconnnectlabel[n_stationid].setPixmap(self.QPixmap_disconnect)
            self._camerareadystatuschange(n_stationid, False)

        # 非离线检中且相机初始化成功状态
        if not self.ui.toolbtn_offlinerun.isChecked() and not False in self.list_isready_camera:
            self.ui.toolbtn_onlinerun.setEnabled(True)
        else:
            self.ui.toolbtn_onlinerun.setEnabled(False)


    def _receive_hardware_netconnectchange_msg(self, bool_status):
        """
        ::                   硬件网络连接状态改变槽函数,其实不必再使用
                                一个self.b_hardwarestatus，直接用self.value_hardware_netconnectflag
        :param bool_status:     状态
        :return:
        """
        if bool_status:
            self.label_hardwarenetconnnect.setPixmap(self.QPixmap_hardware)
        else:
            self.label_hardwarenetconnnect.setPixmap(self.QPixmap_dishardware)

    def _camerareadystatuschange(self, n_pos, bool_status):
        '''
        相机准备就绪后调用
        '''
        if bool_status:
            self.list_camerastatuslabel[n_pos].setPixmap(self.QPixmap_camera)
            self.list_isready_camera[n_pos] = True
            self.ui.toolbtn_onlinerun.setEnabled(False)
        else:
            self.list_camerastatuslabel[n_pos].setPixmap(self.QPixmap_discamera)
            self.list_isready_camera[n_pos] = False
            self.ui.toolbtn_onlinerun.setEnabled(True)

    def _allqueue_clear(self):
        """
        清除公共队列内容，在开始检测时被调用
        """
        for i in range(len(ipc_tool.getlist_queue_send())):
            self._queue_clear(ipc_tool.getlist_queue_send()[i])
        self._queue_clear(ipc_tool.getqueue_processedData_param())
        self._queue_clear(ipc_tool.getqueue_processedData_monitoring())
        self._queue_clear(ipc_tool.getqueue_processedData())


    def _queue_clear(self, queue):
        """
        清除 queue 队列中的内容
        """
        if queue is not None:
            while (not queue.empty()):
                queue.get()

    def _getcurmodelposition(self):
        """
        得到当前模板路径,比如当开始检测时，子站需要知道当前模板位置，所以由这里提供
        """
        if self.widget_Paramsetting.getcurmodelposition() is not None:
            return self.widget_Paramsetting.getcurmodelposition()
        else:
            return []

    def _onlinecheck_ready(self):
        """
        开始检测前的准备
        """
        for pos in range(self.n_substationcount):  # 开启子站
            data = struct.pack('i', 0)
            self.sendmsg(pos, imc_msg.GlobalMsgSend.MSG_STOP_CAMERA, data)
            paramsdir = self._getcurmodelposition()[pos]
            if paramsdir is None or paramsdir == []:
                self.changestausword(u"当前无模板", n_ErrorLevel=logging.WARN)
                return 0
            if not isinstance(paramsdir, bytes):
                paramsdir = paramsdir.encode('gbk')
            data = struct.pack('i', len(paramsdir)) + struct.pack('=%ds' % len(paramsdir), paramsdir)
            self.sendmsg(pos, imc_msg.GlobalMsgSend.MSG_START_CHECK, data)
        return 1

    def _offlinecheck_ready(self):
        """
        模拟检测准备
        """
        pass

    def _onlinerun(self):
        """
        在线跑槽函数：清除队列，给子站发送的命令，切换检测状态
        """
        if self.ui.toolbtn_onlinerun.isChecked():  # 开始检测
            self._allqueue_clear()
            if not self._onlinecheck_ready():
                self.ui.toolbtn_onlinerun.setChecked(False)
                return
            self.ui.toolbtn_offlinerun.setEnabled(False)
            self.value_runflag.value = imc_msg.RUNFLAG.ONLINERUN
            self.changestausword(ChineseWord.STARTCHECK, n_ErrorLevel=imc_msg.LOGLEVEL.GENERAL)
        else:  # 停止检测
            for pos in range(self.n_substationcount):
                self.sendmsg(pos, imc_msg.GlobalMsgSend.MSG_STOP_CHECK)
            self.ui.toolbtn_offlinerun.setEnabled(True)
            self.value_runflag.value = imc_msg.RUNFLAG.STOP
            self.changestausword(ChineseWord.STOPCHECK, n_ErrorLevel=imc_msg.LOGLEVEL.GENERAL)


    def _offlinerun(self):
        """离线跑槽函数"""
        if self.ui.toolbtn_offlinerun.isChecked():  # 开始离线跑
            offlinerundialog = KxLineEditDialog(self)
            offlinerundialog.setWindowTitle(self.tr('Offline running path'))
            offlinerundialog.setlabeltext(self.tr(u'Sub path'))
            offlinerundialog.settext(self.s_offlinerunpath)
            list_picturepath = []
            if offlinerundialog.exec_() == QtWidgets.QDialog.Accepted:
                self._offlinecheck_ready()
                self.s_offlinerunpath = offlinerundialog.gettext()
                for pos in range(self.n_substationcount):
                    filePath = self.s_offlinerunpath + '\\'
                    list_picturepath.append(filePath)
            else:
                self.ui.toolbtn_offlinerun.setChecked(False)
                return

            self._allqueue_clear()
            for pos in range(self.n_substationcount):
                data = struct.pack('i', 0)
                self.sendmsg(pos, imc_msg.GlobalMsgSend.MSG_STOP_CAMERA, data)
                paramsdir = self._getcurmodelposition()
                if paramsdir is None or paramsdir == []:
                    self.changestausword(u"当前无模板", n_ErrorLevel=logging.WARN)
                    self.ui.toolbtn_offlinerun.setChecked(False)
                    return
                paramsdir = paramsdir[pos]
                picturedir = list_picturepath[pos]

                if not isinstance(paramsdir, bytes):
                    paramsdir = paramsdir.encode('gbk')
                if not isinstance(picturedir, bytes):
                    picturedir = picturedir.encode('gbk')
                data = struct.pack('i', len(paramsdir)) + struct.pack('=%ds' % len(paramsdir), paramsdir) \
                       + struct.pack('i', len(picturedir)) + struct.pack('=%ds' % len(picturedir), picturedir)
                self.sendmsg(pos, imc_msg.GlobalMsgSend.MSG_ALL_IMG, data)
            self.value_runflag.value = imc_msg.RUNFLAG.OFFLINERUN
            self.changestausword(ChineseWord.STARTOFFLINECHECK, n_ErrorLevel=imc_msg.LOGLEVEL.GENERAL)
        else:
            for pos in range(self.n_substationcount):
                self.sendmsg(pos, imc_msg.GlobalMsgSend.MSG_STOP_CHECK)
            if not (False in self.list_isready_camera):
                self.ui.toolbtn_onlinerun.setEnabled(True)
            self.value_runflag.value = imc_msg.RUNFLAG.STOP
            self.changestausword(ChineseWord.STOPOFFLINECHECK, n_ErrorLevel=imc_msg.LOGLEVEL.GENERAL)

    def _stopcheck(self):
        """
        关软件前调用，给子站发送停止检测命令;2019.08.06 没有用
        """
        pass
        # if not self.ui.toolbtn_offlinerun.isChecked():
        #     self._offlinerun()
        # if self.bool_onlinerunstatus:
        #     self._onlinerun()

    def _getcurprocessname(self):
        """得到当前进程名，目的是关闭前杀掉当前进程"""
        cpid = os.getpid()
        curprocess = psutil.Process(cpid)
        return curprocess.name()

    def closeEvent(self, event):
        """
        主界面关闭触发此函数，杀死进程，并记录日志
        """
        import win32process
        warnwindow = QtWidgets.QMessageBox()
        respond = warnwindow.warning(self, u"警告", u"确定关闭程序？",
                                     QtWidgets.QMessageBox.Ok | QtWidgets.QMessageBox.Cancel)
        # respond = QtGui.QMessageBox
        if respond == QtWidgets.QMessageBox.Cancel:
            event.ignore()
        else:
            ipc_tool.kxlog("MAIN", logging.INFO, "————————————————关闭程序————————————————————")
            self._stopcheck()
            for pos in range(len(self.list_handle)):
                self.sendmsg(0, imc_msg.GlobalMsgSend.MSG_CLOSECAMERA)
                time.sleep(2)
                if self.list_handle[pos] is not None:
                    if self.list_handle[pos] != 0:
                        try:
                            win32process.TerminateProcess(self.list_handle[pos][0], 0)
                        except Exception as e:
                            self.changestausword(u"有子站提前关闭", n_ErrorLevel=imc_msg.LOGLEVEL.ERR)
            # self.widget_runlog.savelastlog()
            #ipc_tool.kxlog("main", logging.INFO, u"关闭程序")
            os.system('taskkill /f /im ' + self._getcurprocessname())  # 将当前进程强制关闭（可以啊姚老板）

    def _init_msgthread(self):
        """
        启动主界面刷新线程，其余进程将数据推送到队列中，由该线程判断这些数据是否需要刷新，
        并通过信号槽刷新UI(传入自身指针，回调)
        """
        self.thread_refreshinterface = _RefreshInterface(self)
        self.thread_refreshinterface.start()

    def _saveallimage(self):
        """存图槽函数,注意发消息时没用+\0方式,全部图片默认存在d:\\all\\ + 输入位置 """
        saveallimagedialog = KxLineEditDialog(self)
        saveallimagedialog.setWindowTitle(u'保存所有图片')
        saveallimagedialog.setlabeltext(u'输入保存路径')
        saveallimagedialog.settext(self.s_saveallimagepath)

        list_saveallimagepath = []
        s_saveallimagepath = ""
        if saveallimagedialog.exec_() == QtWidgets.QDialog.Accepted:
            self.s_saveallimagepath = str(saveallimagedialog.gettext())
            if self.n_isengineermode:
                if not os.path.isdir("d:\\all\\"):
                    os.mkdir("d:\\all\\")
                s_saveallimagepath = "d:\\all\\" + self.s_saveallimagepath
                if not os.path.isdir(s_saveallimagepath):
                    os.mkdir(s_saveallimagepath)
            else:
                s_saveallimagepath = self.s_saveallimagepath
            for pos in range(self.n_substationcount):
                filePath = s_saveallimagepath + '\\' + str(pos + 1)
                list_saveallimagepath.append(filePath)
        else:
            return

        for pos in range(self.n_substationcount):
            saveallimagepath = list_saveallimagepath[pos]
            if not isinstance(saveallimagepath, bytes):
                saveallimagepath = saveallimagepath.encode('gbk')
            data = struct.pack('i', len(saveallimagepath)) + struct.pack('=%ds' % len(saveallimagepath),
                                                                         saveallimagepath)
            self.sendmsg(pos, imc_msg.GlobalMsgSend.MSG_SAVE_IMAGE, data)

        self.changestausword(u'保存图片，图片路径是：' + s_saveallimagepath, n_ErrorLevel=imc_msg.LOGLEVEL.GENERAL)

    def _savebadimage(self):
        """存坏图槽函数,注意发消息时没用+\0方式，默认存在d:\\bad\\ + 输入位置"""
        saveallimagedialog = KxLineEditDialog(self)
        saveallimagedialog.setWindowTitle(u'保存坏图')
        saveallimagedialog.setlabeltext(u'输入保存路径')
        saveallimagedialog.settext(self.s_savebadimagepath)

        list_savebadimagepath = []
        if saveallimagedialog.exec_() == QtWidgets.QDialog.Accepted:
            self.s_savebadimagepath = str(saveallimagedialog.gettext())
            if self.n_isengineermode:
                if not os.path.isdir("d:\\bad\\"):
                    os.mkdir("d:\\bad\\")
                s_savebadimagepath = "d:\\bad\\" + self.s_savebadimagepath
                if not os.path.isdir(s_savebadimagepath):
                    os.mkdir(s_savebadimagepath)
            else:
                s_savebadimagepath = self.s_savebadimagepath
            for pos in range(self.n_substationcount):
                filePath = s_savebadimagepath + '\\' + str(pos + 1)
                list_savebadimagepath.append(filePath)
        else:
            return

        for pos in range(self.n_substationcount):
            savebadimagepath = list_savebadimagepath[pos]
            if not isinstance(savebadimagepath, bytes):
                savebadimagepath = savebadimagepath.encode('gbk')
            data = struct.pack('i', len(savebadimagepath)) + struct.pack('=%ds' % len(savebadimagepath),
                                                                         savebadimagepath)
            self.sendmsg(pos, imc_msg.GlobalMsgSend.MSG_SAVE_BAD_IMAGE, data)
        self.changestausword(u'保存坏图，图片路径是：' + s_savebadimagepath, n_ErrorLevel=imc_msg.LOGLEVEL.GENERAL)

    def _saveimg(self):
        # dialog = QtGui.QDialog(self)
        # # layout = QtGui.QHBoxLayout(dialog)
        # box = QtGui.QDialogButtonBox(dialog)
        # # layout.addWidget(box)
        msg_box = QtWidgets.QMessageBox
        result = msg_box.question(self, u'保存类型', u'是否只保存坏图？', msg_box.Yes | msg_box.No | msg_box.Cancel, msg_box.Cancel)
        if result == msg_box.Yes:
            self._savebadimage()
        elif result == msg_box.No:
            self._saveallimage()
        else:
            pass



    """
        *** 重要且经常会用到的系统函数 ***
    """

    def recmsg(self, n_stationid, n_msgtype, s_extdata=b''):
        '''
        每个界面文件初始化时都设置父窗口成员变量，并在本窗口加此方法，目的是不耦合的给子站发送消息。
        '''
        if n_msgtype == imc_msg.MSG_CAMERA_IS_READY:
            nstatus = struct.unpack("i", s_extdata)[0]
            self._camerareadystatuschange(n_stationid, nstatus)
        elif n_msgtype == imc_msg.MSG_START_CHECK_IS_READY:
            nstatus = struct.unpack("i", s_extdata[:struct.calcsize('i')])[0]
            if not nstatus:
                if self.ui.toolbtn_onlinerun.isChecked():
                    self.ui.toolbtn_onlinerun.setChecked(False)
                    self._onlinerun()
                elif self.ui.toolbtn_offlinerun.isChecked():
                    self.ui.toolbtn_offlinerun.setChecked(False)
                    self._offlinerun()


    def sethandle(self, list_handle):
        """初始化时启用各个子站，将各个子站权柄交于这里，方便关闭的时候调用关闭"""
        self.list_handle = list_handle

    def DataPipeline(self, npipelinetype, stationid, list_data):
        """
        数据管道，不同界面之间存在交互需求，定义唯一的交互函数，并用标志位来判断每次交互的类型

        Parameters
        ----------
        npipelinetype : int
            标志位
        stationid : int
            站点号
        list_data : list
            数据
        """
        # if npipelinetype in DataPipeLineType.LIST_GLOBALPARAM_TYPE:
        #     return self.widget_Paramsetting.DataPipeline(npipelinetype, stationid, list_data)

    def changestausword(self, s_word, n_ErrorLevel=logging.INFO, s_station="mainstation"):
        """
        写日志，状态栏提示；n_ErrorLevel不为None时写入日志（写日志理论不应在这，应改为logging模块的写法）

        Parameters
        ----------
        s_word : str/unicode
            报错内容
        n_ErrorLevel : int
            错误等级，类型由imc_msg.LOGLEVEL组成
        nstation : int
            站号
        """
        ipc_tool.kxlog(s_station, n_ErrorLevel, s_word)


    def sendmsg(self, n_stationid, n_msgtype, s_extdata=b''):
        """
        给子站发送消息
        """
        ipc_tool.sendmsg(n_stationid, n_msgtype, s_extdata, self)

    def CallbackTip(self, s_word, color=None):
        """
        提供于日志界面回调的函数，用于显示状态栏

        Parameters
        ----------
        s_word : str
            报错内容
        color : list
            颜色值,RGB，0~255
        """
        self.label_statusinf.setText(s_word)
        if color is not None:
            if isinstance(color, str):
                self.label_statusinf.setStyleSheet(color)
            else:
                s_color = "background-color: rgb(%d, %d, %d);"%(color[0], color[1], color[2])
                self.label_statusinf.setStyleSheet(s_color + '\n' +  "font: 10 10pt \"宋体\";")
            self.label_statustimer = QtCore.QTimer()
            self.label_statustimer.timeout.connect(self._resetcolor)
            self.label_statustimer.start(5000)
        else:
            self.label_statusinf.setStyleSheet("")



    def _resetcolor(self):
        self.label_statusinf.setStyleSheet("")
        self.label_statusinf.clear()
        self.label_statustimer.stop()

        # -------------------------  TODO:  待修改区 -------------------------------------------------------------------#

    """
        date:
        author:
        description:        ----------------   权限设置部分   -----------------
                            需要根据实际情况调整

    """

    def showpermissiondialog(self):
        self.widget_permission.setpasswordpath(self.s_paramdir)
        self.widget_permission.exec_()
        permissonlevel = self.widget_permission.getpermissionlevel()
        self.updatepermission(permissonlevel)

    def updatepermission(self, PERMISSIONLEVEL):
        pass
        # #if self.permissionlevel != PERMISSIONLEVEL:
        # self.permissionlevel = PERMISSIONLEVEL
        # if PERMISSIONLEVEL == PermissionLevel.PRODUCER:
        #     self.ui.widget_3.setEnabled(False)
        #     self.ui.widget_11.setEnabled(False)
        #     self.ui.pbt_paramset.setEnabled(False)
        #     self.ui.label_PermissionLevel.setText( u"生产员")
        # elif PERMISSIONLEVEL == PermissionLevel.MANAGER:
        #     self.ui.widget_3.setEnabled(True)
        #     self.ui.widget_11.setEnabled(True)
        #     self.ui.pbt_paramset.setEnabled(True)
        #     self.ui.label_PermissionLevel.setText( u"管理员")
        # else:
        #     pass



    def sendmsghardware(self, data):
        """给硬件发送数据"""
        ipc_tool.sendmsghardware(data)

    def _restartsubstation_timer(self):
        """
        通过定时器启动子站
        :return:
        """
        # 先关闭所有相机
        for i in range(len(self.list_handle)):
            self.sendmsg(i, imc_msg.GlobalMsgSend.MSG_CLOSECAMERA)
        time.sleep(0.5)
        killclient(int(self.dict_config['substationport']))
        list_handle = setup_allsubstation(self.dict_config)
        if None not in list_handle:
            self.sethandle(list_handle)
            ipc_tool.kxlog("主站", logging.INFO, "重启程序成功")
        else:
            ipc_tool.kxlog("主站", logging.ERROR, "重启程序失败")



#QtCore.QThread
class _RefreshInterface(threading.Thread):
    """
    刷新界面线程，不断读取处理数据队列数据，根据标志位，通过信号槽来调用各界面函数
    """
    def __init__(self, mainwindow):
        '''
        初始化，传入处理数据队列
        '''
        super(_RefreshInterface, self).__init__()
        self.queue_processedData = ipc_tool.getqueue_processedData()
        self.mainwindow = mainwindow
        self.list_value_netconnectflag = ipc_tool.getlist_value_netconnectflag()
        self.value_hardware_netconnectflag = ipc_tool.getvalue_hardware_netconnectflag()

    def _refresh_connectstatus(self):
        """
        刷新连接状态：子站连接状态，硬件连接状态
        """
        for pos in range(len(self.list_value_netconnectflag)):
            if self.list_value_netconnectflag[pos].value == 1:
                self.mainwindow.sig_netconnectchange.emit(pos, True)
            else:
                self.mainwindow.sig_netconnectchange.emit(pos, False)

        if self.value_hardware_netconnectflag is not None:
            if self.value_hardware_netconnectflag.value == 1:
                self.mainwindow.sig_hardware_netconnectchange.emit(True)
            else:
                self.mainwindow.sig_hardware_netconnectchange.emit(False)

    def run(self):
        """
        线程开始后触发此函数,RefreshInterface线程不直接调用主界面线程函数，发送信号调用，避免一些不必要的警告,
        开始不断根据共享内存标志位，改变状态栏对应位置颜色，表明各子站与硬件是否连接成功
        """
        time.sleep(0.1)  # 为了不出现程序启动时争着写异常列表，导致异常丢失
        while (1):
            if self.queue_processedData.empty():
                time.sleep(0.001)
            while (not self.queue_processedData.empty()):
                curtuple = self.queue_processedData.get()#

                if curtuple[1] in imc_msg.list_mainwindow_module:
                    self.mainwindow.sig_mainwindows.emit(curtuple[0], curtuple[1], curtuple[2])

                if curtuple[1] == imc_msg.MSGHardware.MSG_HARDWARE:
                    self.mainwindow.sig_hardware_receive.emit(curtuple)

            self._refresh_connectstatus()


