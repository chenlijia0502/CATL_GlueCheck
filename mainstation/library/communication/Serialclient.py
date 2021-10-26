#coding: utf-8
import serial
import time
import multiprocessing
import imc_msg
import struct
import threading
_IsMetersInfo    = (chr(0x0a), chr(0x00), chr(0x00), chr(0x00), chr(0x00))
DATALEN = len(_IsMetersInfo)

BEAT_TIMES = 0

"""
    date:   20190919
    author: GL
    fun:    串口通信，实际效果需要验证
"""

class CreateHardwareUDPClient(multiprocessing.Process):
    def __init__(self, timeout, bps, port, queue_sendtohardware, value_netconnectflag,
                 queue_processedData, n_hardwarequeuelen):
        super(CreateHardwareUDPClient, self).__init__()
        self.queue_processedData = queue_processedData#
        self.queue_sendtohardware = queue_sendtohardware#发送数据
        self.hardwareport = 11  # 主站接收端端口，固定
        self.BOOL=True
        self.port=port
        self.timeout=timeout
        self.bps=bps
        self.value_runflag = value_netconnectflag
        self.n_hardwarequeuelen=n_hardwarequeuelen
    def sender(self,ser):
        while self.BOOL:
            while (not self.queue_sendtohardware.empty()):
                curData = self.queue_sendtohardware.get()
                ser.write(curData)
                time.sleep(0.005)
    def Heartbeat_detection(self,ser):
        #心跳检测，检测网络是连接状态,一般是循环10次之后检测一下
        global  BEAT_TIMES
        BEAT_TIMES += 1
        if BEAT_TIMES >= 10:
            BEAT_TIMES = 0
            try:
                data = self.sendone(ser)
                #下面这句话是为了防止数据冲突，反正接收到硬件的消息就往主界面送
                self.queue_processedData.put((self.hardwareport, imc_msg.MSGHardware.MSG_HARDWARE, data))
            except:
                self.value_runflag.value = 0
                return False
            self.value_runflag.value = 1
        return True
    def sendone(self,ser):
        #发送 1 测试是否回复
        time.sleep(0.1)
        str_test = struct.pack('6B', 0xdd, self.hardwareport, 0, 0, 0,0)
        ser.write(str_test)
        data  =ser.read(ser.in_waiting)
        return data

    def recv(self,ser):
        data=ser.read(ser.in_waiting)
        return data

    def Dopenport(self):
        ret=False
        try:
            ser=serial.Serial(self.port,self.bps,timeout=self.timeout)
            if ser.is_open:
                ret=True
                threading.Thread(target=self.recv,args=(ser,)).start()
            return ser, ret
        except Exception as e:
            print (e)
            self.Dopenport()
    def DCloseport(self,ser):
        self.BOOL=False
        ser.close()
    def run(self):
        try:
            a=self.Dopenport()
        except WindowsError:
            a = self.Dopenport()
        while True:
            try:
                if self.Heartbeat_detection(a[0]):
                    data = self.recv(a[0])
                    if data != '':
                        self.queue_processedData.put((self.hardwareport, imc_msg.MSGHardware.MSG_HARDWARE, data))
            except Exception as ex:
                pass
            time.sleep(0.01)
