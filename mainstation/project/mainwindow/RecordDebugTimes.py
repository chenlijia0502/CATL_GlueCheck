import os
import logging
from library.common.globalparam import SYSTEMPATH
from library.ipc import ipc_tool

class DebugStatus:
    STATUS1 = 0# 非调试模式
    STATUS2 = 1#  调试模式
    STATUS3 = 2#  系统自动放行模式，全判OK
    STATUS4 = 3#  软件不检模式，但会取图存图。目的是考虑程序可能检测过程中发生崩溃，但不想放弃该组数据。注意只有一次，检完一次之后会自动复位

class CRecordDebugTimes(object):
    """
    记录调试模式，并且记录调试模式下运行次数
    """
    FILE_PATH = SYSTEMPATH.BASE_DIR + "\\debug.txt"
    def __init__(self):
        super(CRecordDebugTimes, self).__init__()
        self.debugmode = DebugStatus.STATUS1
        self.ndebugtimes = 0
        self.__initpathfile()


    def __initpathfile(self):
        """
        如果路径存在则初始化self.ndebugtimes的值，不存在则创建路径并初始化
        :return:
        """
        try:
            list_dir = self.FILE_PATH.split("\\")
            if not os.path.isfile(self.FILE_PATH):# 创建文件夹
                if len(list_dir) > 2:
                    del list_dir[-1]
                    newpath = "\\".join(list_dir)
                    if not os.path.isdir(newpath):
                         os.makedirs(newpath)
                    self._savefile()
            else:
                with open(self.FILE_PATH, "r") as f:  # 设置文件对象
                    strdata = self._decrypt(f.read())  # 可以是随便对文件的操作
                    self.ndebugtimes = int(strdata)
        except Exception as e:
            self.ndebugtimes = 0
            self._savefile()



    def _encrypt(self,data):
        try:
            key = 'ZS_TECH'
            data = data.encode('utf-8')
            dataLen = len(data)
            keyLen = len(key)
            key = dataLen // keyLen * key + key[:dataLen % keyLen]
            result = bytearray()
            for i in range(len(key)):
                result.append(ord(key[i]) ^ data[i])
            return result.decode('utf-8')
        except Exception as e:
            ipc_tool.kxlog("CRecordDebugTimes encrypt", logging.ERROR, "加密调试次数密钥发生错误")


    def _decrypt(self,data):
        key = 'ZS_TECH'
        data = data.encode('utf-8')
        dataLen = len(data)
        keyLen = len(key)
        key = dataLen // keyLen * key + key[:dataLen % keyLen]

        result = bytearray()
        for j in range(len(key)):
            result.append(data[j] ^ ord(key[j]))
        return result.decode('utf-8')


    def _savefile(self):
        with open(self.FILE_PATH, "w") as f:  # 设置文件对象
            f.write(self._encrypt(str(self.ndebugtimes)))  # 可以是随便对文件的操作


    def IncreaseDebugTimes(self):
        self.ndebugtimes += 1
        self._savefile()

    def Getdebugtimes(self):
        return self.ndebugtimes