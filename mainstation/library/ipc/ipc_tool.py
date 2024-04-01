# coding=utf-8
'''
Date:           2019.11.05
author:         YZH, HYH
Description:    作为公共变量被调用，使用于进程通信。
                增加传入类名，并将这里的变量作为全局变量被直接调用，在调用时传入类名方便分析是哪传入的数据

NOTE：          （1）全局变量对多进程无效，也即不同的进程调用全局变量时其实是在各自的进程内存中复制了这个变量
                （2）multiprocessing包创建的变量可以用于多进程通信，但是！必须是通过传参的方式将全局队列传入各个
                    进程的类，或者在main中初始的类的__init__中为自身的变量赋值公共队列。所以使用者要千万慎重，每
                    个新的公共队列的引入都必须注意使用方法。

'''

import multiprocessing

list_queue_received = []
def getlist_queue_received(classobj=None):
    """
    接收子站数据队列列表
    2020.02.21 这里有个值得注意的地方，list_queue_received是一个列表队列，但其索引跟子站的id没有任何关系，
    它不像list_queue_send是一一对应，比如list_queue_send[0]对应的是id为0的子站的发送队列，接收队列没有这样
    的关系，其列表的作用只是体现在当多个子站同时给主站发送数据时避免put冲突问题，用列表代替了锁的功能，至于
    怎么区分id--->在其收到的数据中有数据头，里面有站点信息
    """
    return list_queue_received

list_queue_send = []
def getlist_queue_send(classobj=None):
    """
    发送到子站数据队列列表
    """
    return list_queue_send

list_value_netconnectflag = []
def getlist_value_netconnectflag(classobj=None):
    """
    子站网络连接标志位列表，共享内存列表
    """
    return list_value_netconnectflag


def setsubstationipc(n_substationcount, classobj=None):
    """
          根据配置文件重新初始化与子站个数相关的ipc工具
    """
    global list_queue_received
    global list_queue_send
    global list_value_netconnectflag
    global list_value_cameraflag
    list_queue_received = [multiprocessing.Queue() for i in range(n_substationcount)]
    list_queue_send = [multiprocessing.Queue() for i in range(n_substationcount)]
    list_value_netconnectflag = [multiprocessing.Value('i', 0) for i in range(n_substationcount)]
    list_value_cameraflag = [multiprocessing.Value('i', 1) for i in range(n_substationcount)]


queue_hardware_rereceived = None
def getqueue_hardware_rereceived(classobj=None):
    """
    接收硬件数据队列
    """
    return queue_hardware_rereceived

queue_hardware_send = None
def getqueue_hardware_send(classobj=None):
    """
    发送到硬件数据队列
    """
    return queue_hardware_send


value_hardware_netconnectflag = None
def getvalue_hardware_netconnectflag(classobj=None):
    """
    硬件网络连接标志位，共享内存
    """
    return value_hardware_netconnectflag

def createhardwareipc(n_queuelen, classobj=None):
    """
          根据配置文件重新初始化与子站个数相关的ipc工具
    """
    global queue_hardware_rereceived
    global queue_hardware_send
    global value_hardware_netconnectflag
    queue_hardware_rereceived = multiprocessing.Queue()
    queue_hardware_send = multiprocessing.Queue(n_queuelen)
    value_hardware_netconnectflag = multiprocessing.Value('i', 0)


queue_processedData = None
def getqueue_processedData(classobj=None):
    """
    将子站接收来消息解包后塞到此队列，主要起缓冲作用，避免解包数据操作影响网络通讯和刷新界面，
    数据主要传送到mainwindow界面中
    """
    global queue_processedData
    return queue_processedData

queue_processedData_monitoring = None
def getqueue_processedData_monitoring(classobj=None):
    """
    将子站接收来消息解包后塞到此队列，主要起缓冲作用，避免解包数据操作影响网络通讯和刷新界面，
    数据主要传送到realtime界面中
    """
    global queue_processedData_monitoring
    return queue_processedData_monitoring

queue_processedData_param = None
def getqueue_processedData_param(classobj=None):
    """
    将子站接收来消息解包后塞到此队列，主要起缓冲作用，避免解包数据操作影响网络通讯和刷新界面，
    数据主要传送到param界面中
    """
    global queue_processedData_param
    return queue_processedData_param

queue_kxlog = None
def getqueue_queue_kxlog(classobj=None):
    """
    日志队列
    """
    global queue_kxlog
    return queue_kxlog

value_runflag = None
def getvalue_runflag(classobj=None):
    """
    得到运行标志，开始检测置为1，停止检测置为0.
    """
    global value_runflag
    return value_runflag

value_id = None
def getvalue_id(classobj=None):
    """
    得到图像id,方便硬件控制进程使用
    """
    global value_id
    return value_id

def createglobalparam():
    """创建全局参数"""
    global queue_processedData
    global queue_processedData_monitoring
    global queue_processedData_param
    global queue_kxlog
    global value_runflag
    global value_id
    queue_processedData = multiprocessing.Queue()
    queue_processedData_monitoring = multiprocessing.Queue()
    queue_processedData_param = multiprocessing.Queue()
    queue_kxlog = multiprocessing.Queue()
    value_runflag = multiprocessing.Value('i', 0)
    value_id = multiprocessing.Value('i', 0)




"""
    2020.02.23 要非常注意下面几个函数的使用，默认只能在mainwidget使用，原因是只有在主界面使用，其公共队列
    才是在main中创建的公共队列（看最上面公共队列的说明）
"""

import imc_msg
import struct
def sendmsg(n_stationid, n_msgtype, s_extdata=b'', classobj=None):
    """
    给子站发送消息，设置为公共函数，是为了方便各个类调用发送数据，而最后的classobj是为了判断数据来源
    在发送数据前需要调用添加数据头
    Parameters
    ----------
    n_stationid : int
        站点号
    n_msgtype : int
        数据类型（标志位）
    s_extdata : str/unicode
        发送的数据
    classobj: obj
        调用此函数的类
    """

    imc_msg.MSGHeader.n_stationid = n_stationid
    imc_msg.MSGHeader.n_msgtype = n_msgtype
    imc_msg.MSGHeader.n_extdatasize = len(s_extdata)

    list_headdata = [imc_msg.MSGHeader.n_stationtype, imc_msg.MSGHeader.n_stationid, imc_msg.MSGHeader.n_msgtype,
                     imc_msg.MSGHeader.n_subtype, imc_msg.MSGHeader.n_extdatasize,
                     imc_msg.MSGHeader.n_headerextdatasize,
                     imc_msg.MSGHeader.n_selfcheck, imc_msg.MSGHeader.n_checksum, imc_msg.MSGHeader.s_headerextdata]
    head = struct.pack(imc_msg.MSGHeader.dataheadformat, *list_headdata)
    if not isinstance(s_extdata, bytes):
        s_extdata = s_extdata.encode('gbk')#str与bytes无法直接拼接
    getlist_queue_send()[n_stationid].put((n_stationid, head + s_extdata))


def sendmsghardware(data, classobj=None):
    """
    给硬件层发送数据，PLC
    """
    getqueue_hardware_send().put((data))


def kxlog(stationname, level, data):
    """
    关于日志部分，应该使用logging模块，最多就是跟界面显示以及状态栏挂钩

    Parameters
    ----------
    stationname : str
        报错源，比如哪个站，或者是哪个进程
    level : int
        日志等级，同步logging模块
    data : str/unicode
        信息
    """
    getqueue_queue_kxlog().put((stationname, level, data))