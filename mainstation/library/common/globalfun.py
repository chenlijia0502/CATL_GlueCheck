#coding:utf-8
import os
import win32api
import win32process
import logging
from library.ipc import ipc_tool
import psutil
import signal
def setup_allsubstation(dict_config):
    list_exepath = [dict_config[u'substation'][stationname][u'exepath'] for stationname in dict_config[u'substation']]
    class_sub = ControlSubStation()
    list_handle = class_sub.InitStation(list_exepath)
    if None in list_handle:
        ipc_tool.kxlog("main", logging.ERROR, "substation init error")
        #return None
    return list_handle

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

class ControlSubStation(object):
    """控制同个主机的子站启动，并返回handle，用以杀死关闭"""
    def __init__(self):
        pass

    def InitStation(self, list_exepath):
        s_curpath = os.getcwd()
        list_handle = []
        for pos in range(len(list_exepath)):
            if os.path.isfile(list_exepath[pos]):
                if os.path.splitext(list_exepath[pos])[1] == '.exe':
                    os.chdir(os.path.split(list_exepath[pos])[0])
                    handle = win32process.CreateProcess(list_exepath[pos], '', None, None, 0,
                                                        win32process.CREATE_NEW_CONSOLE, None, None,
                                                        win32process.STARTUPINFO())
                else:
                    handle = None
            else:
                handle = None
            list_handle.append(handle)
        os.chdir(s_curpath)
        return list_handle

    def CloseHandle(self, list_handle):
        for pos in range(len(list_handle)):
            if list_handle[pos] is not None:
                if list_handle[pos] != 0:
                    try:
                        win32process.TerminateProcess(list_handle[pos][0], 0)
                    except Exception as e:
                        pass

def ischildof(obj, cls):
    """判断obj是否是cls的继承类"""
    try:
        for i in obj.__bases__:#直接继承
            if i is cls or isinstance(i, cls):
                return True
        for i in obj.__bases__:#多重继承
            if ischildof(i, cls):
                return True
    except AttributeError:
        return ischildof(obj.__class__, cls)
    return False




import win32com.client as com

def DriveTotalSize(drive):
  """
  输入盘符路径，如"d:\\"，返回盘符总大小，单位GB
  :param drive:
  :return:
  """
  try:
    fso = com.Dispatch("Scripting.FileSystemObject")
    drv = fso.GetDrive(drive)
    return drv.TotalSize/2**30
  except:
    return -1

def DriveFreeSpace(drive):
  """
  输入盘符路径，如"d:\\"，返回盘符剩余空间，单位GB
  :param drive:
  :return:
  """
  try:
    fso = com.Dispatch("Scripting.FileSystemObject")
    drv = fso.GetDrive(drive)
    return drv.FreeSpace/2**30
  except:
    return -1
workstations = ['dolphins']