#coding=utf-8
'''
Created on 2017年07月06号

date:           2019.11.05
author:         huber yao, HYH
Description:

drawback：       要排查基础版有哪些不用的

'''
import logging

class GlobalMsgSend:
    '''
         给子站发送消息类型集合
    '''
    MSG_START_CHECK = 1001  #开始检测 
    MSG_STOP_CHECK = 1002  #停止检测
    MSG_SAVE_IMAGE = 1003  #保存采集图像
    MSG_SAVE_BAD_IMAGE = 1033  #保存采集图像
    MSG_ALL_IMG = 1005  #多张测试

    MSG_START_CAMERA = 1012 #开始采集
    MSG_STOP_CAMERA = 1013  #停止采集
    MSG_CHANGE_EXPOURE_TIME = 1014 #调整曝光时间
    MSG_CHANGE_ADJUST_MOTOR_STATUS = 1044#改变切换调焦状态，也即自动还是手动
    MSG_HANDSHAKE_SEND = 3001 #子站心跳

    MSG_CLOSECAMERA = 1090#关闭海康相机




"""
   2020.02.11为了数据的传递，自定义口信号之后，其会将数据自动地放入对应的队列中，送到该界面去.
"""
    
MSG_CHECK_RESULT = 1 # 检测结果

MSG_SEND_REAL_TIME_IMAGE = 1015 #发送实时图像

MSG_START_CHECK_IS_READY = 1021#开始检测初始化完成，包括加载参数等
MSG_CAMERA_IS_READY = 1022#相机初始化完成，准备就绪
MSG_CHECK_RESULT_BAD = 1026 #检查结果有坏的情况，让主站通知硬件

MSG_LOG = 1016 #日志


MSG_A = 2022
MSG_LEARN_ONE_COMPLETED = 1009

list_monitoring_module = [MSG_CHECK_RESULT, MSG_A]  # 实时显示界面
list_params_module = [MSG_SEND_REAL_TIME_IMAGE, MSG_LEARN_ONE_COMPLETED] #参数设置界面
list_mainwindow_module = [MSG_START_CHECK_IS_READY, MSG_CAMERA_IS_READY, MSG_CHECK_RESULT]#主界面



class MSGHardware:
    MSG_HARDWARE = 8888
    # MSG_LABELF_FAIL = 6 #贴标失败
    # MSG_CHANGE_VOLUME = 7 #换卷
    # MSG_HARDWARE_TEST_START = 8 #硬件发信号让软件开始检测的流程，目的是为了测试
    #


class MSGHeader:
    '''
    TCP消息头
    '''    
    dataheadformat = b'=6i2H228s' # "="是默认字节对齐方式，H是unsigned short，两个字节
        
    n_stationtype = 0
    n_stationid = 0 #站点ID
    n_msgtype = 0 #消息标志位
    n_subtype = 0 #
    n_extdatasize = 0 #
    n_headerextdatasize = 0
    n_selfcheck = 0
    n_checksum = 0
    s_headerextdata = b'1'*228# 头文件剩余文件
      
class MSGSENDER:
    '''
    消息发送人定义
    '''
    MaxSubStationNum = 100
    HardwareTCPServer = 101
    DataProcess = 102
    SysParams = 103
    UdpMessage = 104
    NetConfig = 105

    
class LOGLEVEL:
    """
    日志错误等级
    """
    # GENERAL = 0
    # WARN = 1
    # ERR = 2
    GENERAL = logging.INFO
    WARN = logging.WARN
    ERR = logging.ERROR

    
class RUNFLAG:
    '''
    运行状态标志
    '''
    STOP = 0 #停止状态是0
    ONLINERUN = 1 #在线运行状态是1
    OFFLINERUN = 2 #离线运行状态是2




