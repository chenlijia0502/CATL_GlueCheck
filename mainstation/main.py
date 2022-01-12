#coding:utf-8
import os
import psutil
import signal
from PyQt5 import QtGui, QtCore, QtWidgets
from library.common import readconfig
from project.mainwindow.kxmainwindow import kxmainwindow
from library.ipc import ipc_tool
from library.mainwindow.LoginInterface import LoginInterface
from library.communication.server_tcp import CreateTCPServerFactory
from library.ipc.ipc import DataProcess
from library.communication.hardware_client_udp import CreateHardwareUDPClient
from library.common.globalfun import ControlSubStation
from library.common.readconfig import MAINSTATION_CONFIG, SUBSTATION_CONFIG
import logging
import time
from multiprocessing import freeze_support
from ctypes import *
from project.other.SerialProcess import CreateSerialProcess
from App import App

"""
    基本的配置
"""

LOGGER_PATH = "D:\\log\\"
if not os.path.isdir(LOGGER_PATH):
    os.mkdir(LOGGER_PATH)
s_time = time.strftime("%Y-%m-%d")
logging.basicConfig(filename='d:\\log\\%s.log'%s_time,level=logging.INFO, format='%(levelname)s %(asctime)s %(message)s',
                    datefmt='%Y/%m/%d %H:%M:%S',filemode='a')

def killclient(port=8888):
    """
    杀死连接到本机8888端口的本机客户端,为了系统安全，只杀死当前用户创建的进程
    """
    ret = os.popen("netstat -ano | findstr " + str(port))
    str_data = ret.read()
    list_str = str_data.split('\n')
    for i in range(len(list_str)):
        listtmp = list_str[i].split(' ')
        list_i = []
        for j in range(len(listtmp)):
            if listtmp[j] != '':
                list_i.append(listtmp[j])
        if list_i != []:
            if list_i[2] == '127.0.0.1:' + str(port):
                p = psutil.Process(int(list_i[4]))
                if p.username().split('\\')[1] == psutil.users()[0].name:
                    os.kill(int(list_i[4]), signal.SIGTERM)


def setup_hardwareprocess(dict_config):
    """启动硬件通信进程"""
    #TODO 这里写的不好的一点在于不能选择硬件通信方式，后面要完善
    #hardwareudp = CreateHardwareUDPClient(dict_config['softwareaddress'], dict_config['machineaddress'],
    #                                       dict_config['n_hardware_port'], dict_config['n_hardwarequeuelen'])
    # hardwareudp.start()
    hardwareserial = CreateSerialProcess(dict_config['hardwarecom'])
    hardwareserial.start()


def setup_communicateprocess(dict_config):
    """启动与子站通信的进程"""
    tcpserver = CreateTCPServerFactory(float(dict_config['sendtcpdatalatertime']),
                                       int(dict_config['substationport']),
                                       int(dict_config['maxCameraPerStation']))
    tcpserver.start()

def setup_datatransferprocess(dict_config):
    """创建处理数据进程，处理子站发过来的数据，一方面转给主界面进程，一方面控制硬件"""
    process_datahandling = DataProcess(int(dict_config['istranslate']))
    process_datahandling.start()


def setup_allsubstation(dict_config):
    list_exepath = [dict_config[u'substation'][stationname][u'exepath'] for stationname in dict_config[u'substation']]
    class_sub = ControlSubStation()
    list_handle = class_sub.InitStation(list_exepath)
    if None in list_handle:
        ipc_tool.kxlog("main", logging.ERROR, "substation init error")
    return list_handle


if __name__ == "__main__":

    # 打包时用
    try:
        freeze_support()
    except Exception as e:
        pass

    #dll = cdll.LoadLibrary("res\\UI_key.dll") #加密狗加密


    app = App([])

    if  os.path.isfile(MAINSTATION_CONFIG):
        #弹出一个框，那个框里可以选择配置
        dialog_login = LoginInterface(MAINSTATION_CONFIG, SUBSTATION_CONFIG)
        dialog_login.show()
        dialog_login.exec_()
        if dialog_login.getstatus() is True:
            dict_config = readconfig.readconfig(MAINSTATION_CONFIG)
            ipc_tool.setsubstationipc(len(dict_config['substation']))
            ipc_tool.createhardwareipc(-1)
            ipc_tool.createglobalparam()

            killclient(int(dict_config['substationport']))

            #setup_hardwareprocess(dict_config)

            setup_communicateprocess(dict_config)

            setup_datatransferprocess(dict_config)

            w = kxmainwindow(dict_config)
            list_handle = setup_allsubstation(dict_config)
            w.sethandle(list_handle)
            w.showFullScreen()
            #w.showpermissiondialog()
            import sys
            sys.exit(app.exec_())
    else:
        print("config path does not exist !" )
        #TODO 这里应该弹出一个报错框， 20200203 后面应该加上一个自动生成基础版本的)