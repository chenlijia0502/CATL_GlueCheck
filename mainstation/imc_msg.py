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

    MSG_ENDMOVE = 255
    MSG_ENDALLMOVE = 256

    MSG_NOT_CHECK = 1004 #当前pack不检只存图


"""
   2020.02.11为了数据的传递，自定义口信号之后，其会将数据自动地放入对应的队列中，送到该界面去.
"""
    
MSG_CHECK_RESULT = 1 # 检测结果

MSG_CHECK_RESULT_FINISH = 2 #当前pack检测完成

MSG_SEND_REAL_TIME_IMAGE = 1015 #发送实时图像

MSG_START_CHECK_IS_READY = 1021#开始检测初始化完成，包括加载参数等

MSG_CAMERA_IS_READY = 1022#相机初始化完成，准备就绪

MSG_CHECK_RESULT_BAD = 1026 #检查结果有坏的情况，让主站通知硬件

MSG_LOG = 1016 #日志

MSG_A = 2022

MSG_LEARN_ONE_COMPLETED = 1009


MSG_BUILD_MODEL = 401 # 建模界面触发

MSG_BUILD_MODEL_IMG = 402 #子站发送回来的建模图像

MSG_BUILD_MODEL_SECOND = 403 #建模二次确认触发

MSG_GET_BASE_POINT_HIGH = 404 #建模获取基准高度值

MSG_SET_CHECK_MASK = 405 #设置检测屏蔽区域

#MSG_BUILD_MODEL_IMG_SECOND = 404 #子站发送回来的二次建模图像

MSG_JUST_OPENCAMERA_BUILDMODEL = 501 #只打开相机

MSG_JUST_CLOSECAMERA_BUILDMODEL = 502 #只关闭相机

MSG_CHANGE_CAMERA_INFO_REVERSE = 601 #修改相机取图方向

MSG_RECOVER_CAMERA_INFO_REVERSE = 602 #恢复相机取图方向

MSG_JUST_OPENCAMERA = 603

MSG_CHANGE_CAPTURE_COL = 604

MSG_DOT_CHECK_OPEN = 4001#
MSG_DOT_CHECK_CLOSE = 4002#
MSG_DOT_CHECK_RESULT = 4100#点检检测结果

MSG_PACK_ID = 701


MSG_SHOW_IMG = 1100#检测时候拍的实时图像，用于实时显示界面显示

MSG_CHECK_MATCHERROR = 2001 # 匹配图像发生错误

list_monitoring_module = [MSG_CHECK_RESULT, MSG_A, MSG_SHOW_IMG, MSG_CHECK_RESULT_FINISH]  # 实时显示界面
list_params_module = [MSG_SEND_REAL_TIME_IMAGE, MSG_LEARN_ONE_COMPLETED, MSG_BUILD_MODEL_IMG] #参数设置界面
list_mainwindow_module = [MSG_START_CHECK_IS_READY, MSG_CAMERA_IS_READY, MSG_CHECK_RESULT, MSG_BUILD_MODEL,
                          MSG_BUILD_MODEL_SECOND, MSG_DOT_CHECK_RESULT, MSG_GET_BASE_POINT_HIGH, MSG_SET_CHECK_MASK,
                          MSG_CHECK_MATCHERROR]#主界面



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


class HARDWAREBASEMSG:
    """
    下位机通信
    """

    # 电机相关

    MSG_MOTOR_X_ZERO = [0x01, 0x02, 0x01, 0x02, 0x00, 0x00, 0x00]# 电机移动回到传感器，并自我标定

    MSG_MOTOR_Y_ZERO = [0x01, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00]

    MSG_REBACKMOTOR_X = [0x01, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00] # 返回程序记录的原点位置，是漂的

    MSG_REBACKMOTOR_Y = [0x01, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00]

    MSG_STARTMOTOR_X = [0x01, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00]

    MSG_STARTMOTOR_Y = [0x01, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00]

    MSG_MOTOR_X_BASEMOVE = [0x01, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00]

    MSG_MOTOR_Y_BASEMOVE = [0x01, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00]

    MSG_MOTOR_X_ARRIVE = [0x01, 0x02, 0x01, 0xff, 0x00, 0x00, 0x00]

    MSG_MOTOR_Y_ARRIVE = [0x01, 0x02, 0x02, 0xff, 0x00, 0x00, 0x00]

    MSG_X_XIANWEI = [0x01, 0x02, 0x01, 0xDD, 0x00, 0x00, 0x00]

    MSG_Y_XIANWEI = [0x01, 0x02, 0x02, 0xDD, 0x00, 0x00, 0x00]

    MSG_GET_Z_POS = [0x01, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00]

    MSG_Z_RESPOND_HEAD = [0x03, 0x04]

    MSG_MOTOR_Z_BASEMOVE = [0x01, 0x04, 0x03, 0x00, 0x00, 0x00, 0x00]

    MSG_REBACKMOTOR_Z = [0x01, 0x02, 0x03, 0x02, 0x00, 0x00, 0x00]

    MSG_MOTOR_Z_MOVE = [0x01, 0x02, 0x03, 0x01, 0x00, 0x00, 0x00]#向上动

    MSG_MOTOR_Z_MOVE_DOWN = [0x01, 0x02, 0x03, 0x01, 0x00, 0x00, 0x00]#向下动

    MSG_MOTOR_Z_XIANWEI = [0x01, 0x02, 0x03, 0xDD, 0x00, 0x00, 0x00]

    MSG_MOTOR_Z_ARRIVE = [0x01, 0x02, 0x03, 0xff, 0x00, 0x00, 0x00]

    # 光源

    MSG_OPEN_LIGHT1 = [0x01, 0x03, 0x05, 0x01, 0x00, 0x00, 0x00]

    MSG_OPEN_LIGHT2 = [0x01, 0x03, 0x05, 0x02, 0x00, 0x00, 0x00]

    MSG_OPEN_ALL_LIGHT = [0x01, 0x03, 0x05, 0x03, 0x00, 0x00, 0x00]

    MSG_CLOSE_ALL_LIGHT = [0x01, 0x03, 0x05, 0x00, 0x00, 0x00, 0x00]


    # 气缸

    MSG_JIAJIN = [0x01, 0x03, 0x03, 0x01, 0x00, 0x00, 0x00]

    MSG_SONGKAI = [0x01, 0x03, 0x03, 0x02, 0x00, 0x00, 0x00]

    MSG_SHENG = [0x01, 0x03, 0x02, 0x01, 0x00, 0x00, 0x00]

    MSG_JIANG = [0x01, 0x03, 0x02, 0x02, 0x00, 0x00, 0x00]

    MSG_DUIWEI_JIANG = [0x01, 0x03, 0x04, 0x02, 0x00, 0x00, 0x00]

    MSG_DUIWEI_SHENG = [0x01, 0x03, 0x04, 0x01, 0x00, 0x00, 0x00]

    MSG_LEFT_DINGSHENG_DAOWEI = [0x01, 0x02, 0x01, 0x05, 0x00, 0x00, 0x00]

    MSG_RIGHT_DINGSHENG_DAOWEI = [0x01, 0x02, 0x01, 0x08, 0x00, 0x00, 0x00]

    MSG_LEFT_DINGSHENG_XIAJIANG_DAOWEI = [0x01, 0x02, 0x01, 0x06, 0x00, 0x00, 0x00]

    MSG_RIGHT_DINGSHENG_XIAJIANG_DAOWEI = [0x01, 0x02, 0x01, 0x07, 0x00, 0x00, 0x00]

    MSG_LEFT_JIAJIN_DAOWEI = [0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00]

    MSG_RIGHT_JIAJIN_DAOWEI = [0x01, 0x02, 0x01, 0x03, 0x00, 0x00, 0x00]

    MSG_LEFT_JIAJIN_SONGKAI_DAOWEI = [0x01, 0x02, 0x01, 0x02, 0x00, 0x00, 0x00]

    MSG_RIGHT_JIAJIN_SONGKAI_DAOWEI = [0x01, 0x02, 0x01, 0x04, 0x00, 0x00, 0x00]

    MSG_LEFT_DUIWEI_JIANG_DAOWEI = [0x01, 0x02, 0x01, 0x10, 0x00, 0x00, 0x00]

    MSG_RIGHT_DUIWEI_JIANG_DAOWEI = [0x01, 0x02, 0x01, 0x12, 0x00, 0x00, 0x00]

    MSG_LEFT_DUIWEI_SHENG_DAOWEI = [0x01, 0x02, 0x01, 0x09, 0x00, 0x00, 0x00]

    MSG_RIGHT_DUIWEI_SHENG_DAOWEI = [0x01, 0x02, 0x01, 0x11, 0x00, 0x00, 0x00]


    #门控

    MSG_MENKONG_JIAJIN = [0x01, 0x03, 0x07, 0x01, 0x00, 0x00, 0x00]

    MSG_MENKONG_SONGKAI = [0x01, 0x03, 0x07, 0x02, 0x00, 0x00, 0x00]


    #光栅

    MSG_TURNON_GUANGSHAN = [0x01, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00]

    MSG_TURNOFF_GUANGSHAN1 = [0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00] # 关闭小车进站光栅

    MSG_TURNOFF_GUANGSHAN2 = [0x01, 0x03, 0x00, 0x02, 0x00, 0x00, 0x00]# 关闭小车出站光栅



    # 其余

    MSG_GUANGSHAN = ['01020136000000', '01020137000000', '01020138000000', '01020139000000'] #光栅信号，当接收到这个要做出反应

    MSG_JITING = ['01020141000000', '01020142000000', '01020143000000', '01020144000000']

    MSG_MENKONG = ['01020145000000', '01020146000000', '01020147000000', '01020148000000', '01020149000000', '0102014a000000']

    MSG_REC_START = [0x01, 0x02, 0x01, 0x33, 0x01, 0x00, 0x00]

    MSG_CONTROL_ALARM = [0x01, 0x03, 0x06, 0x03, 0x01, 0x00, 0x00]

    # 切换状态
    MSG_START_CHECK = [0x01, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00]
    #MSG_START_CHECK = [0x01, 0x03, 0x06, 0x00, 0x01, 0x00, 0x00]

    MSG_END_CHECK = [0x01, 0x06, 0x02, 0x00, 0x00, 0x00, 0x00]



class AGVMSG:

    MSG_BASE_GET_STATION_STATUS = [0x4A, 0x54, 0x58, 0x02, 0xE1, 0x00, 0x00, 0x00]# 获取站点状态，第六位为设置位，代表第几个站点的状态

    MSG_BASE_CONTROL_STATION_STATUS = [0x4A, 0x54, 0x58, 0x03, 0xE3, 0x00, 0x02, 0xA2, 0x00]# 设置站点状态，第六位为设置站点位，第七位为状态位

    MSG_BASE_GET_PACK_ID = [0x4A, 0x54, 0x58, 0x02, 0xE7, 0x00, 0x00, 0x00]# 获取小车pack id，第六位为小车编号

    AGVSTATUS_ONSTATION = 3#AGV小车停站状态

    AGVSTATUS_FREE = 1#站点空闲状态

    AGVSTATUS_NONE = 0#站点未知状态