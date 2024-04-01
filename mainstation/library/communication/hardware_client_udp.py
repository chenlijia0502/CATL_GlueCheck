#coding:utf-8
import multiprocessing
import socket
from socket import socket
from _socket import AF_INET, SOCK_STREAM, SOCK_DGRAM
import imc_msg
import threading, time, datetime
import struct
from library.ipc import ipc_tool

_IsMetersInfo        = (chr(0x0a), chr(0x00), chr(0x00), chr(0x00), chr(0x00))
DATALEN = len(_IsMetersInfo)

BEAT_TIMES = 0





class CreateHardwareUDPClient(multiprocessing.Process):
    """
        date:   20190919
        author: HYH
        fun:    硬件UDP通信（经过极片分切项目验证）
    """
    def __init__(self, SoftwareAddress, MachineAddress, port, n_hardwarequeuelen):
        super(CreateHardwareUDPClient, self).__init__()
        self.queue_processedData = ipc_tool.getqueue_processedData()#
        self.queue_sendtohardware = ipc_tool.getqueue_hardware_send()#发送数据
        self.value_runflag = ipc_tool.getvalue_hardware_netconnectflag()#连接标志
        self.queue_kxlog = ipc_tool.getqueue_queue_kxlog()#日志
        self.hardwareport = 11  # 主站接收端端口，固定
        self.SoftwareAddress = (SoftwareAddress, int(self.hardwareport))#本机地址，硬件同事只能固定回复某个IP
        self.MachineAddress = (MachineAddress, int(port))#硬件IP地址
        self.n_hardwarequeuelen = int(n_hardwarequeuelen)#接收数据的长度（一般按字节算）


    def Restart(self):
        self.value_runflag.value = 1
        try:
            self.clientSocket = socket(AF_INET, SOCK_DGRAM)
            # self.clientSocket.setsockopt(socket.SOL_UDP, socket.SO_REUSEADDR, 1)
            self.clientSocket.bind(self.SoftwareAddress)
            # 第一次测试是否连上（发送之后是否等得到回复结果）
            self.sendone()
        except Exception as ex:
            self.value_runflag.value = 0
        self.thread_read = threading.Thread(target=self.Sender, name='SendThread')
        self.thread_read.setDaemon(True)
        self.thread_read.start()

    def Sender(self):
        while True:
            while (not self.queue_sendtohardware.empty()):
                curData = self.queue_sendtohardware.get()
                self.clientSocket.sendto(curData, self.MachineAddress)
                time.sleep(0.005)#硬件缓冲

    def recvData(self):
        data = self.clientSocket.recv(self.n_hardwarequeuelen)
        return data

    def Heartbeat_detection(self):
        #心跳检测，检测网络是连接状态,一般是循环10次之后检测一下
        global  BEAT_TIMES
        BEAT_TIMES += 1
        if BEAT_TIMES >= 10:
            BEAT_TIMES = 0
            try:
                data = self.sendone()
                #下面这句话是为了防止数据冲突，反正接收到硬件的消息就往主界面送
                self.queue_processedData.put((self.hardwareport, imc_msg.MSGHardware.MSG_HARDWARE, data))
            except:
                self.value_runflag.value = 0
                return False
            self.value_runflag.value = 1
        return True

    def sendone(self):
        #发送 1 测试是否回复
        self.clientSocket.settimeout(1)
        str_test = struct.pack('6B', 0xdd, self.hardwareport, 0, 0, 0,0)
        self.clientSocket.sendto(str_test, self.MachineAddress)
        data  = self.clientSocket.recv(self.n_hardwarequeuelen)
        return data

    def run(self):
        self.Restart()
        while True:
            try:
                if self.Heartbeat_detection():
                    data = self.recvData()
                    if data != '':
                        self.queue_processedData.put((self.hardwareport, imc_msg.MSGHardware.MSG_HARDWARE, data))
            except Exception as ex:
                pass
            time.sleep(0.01)





if __name__ == '__main__':

    # f_latertime = 0.1
    # list_serverip = ['127.0.0.1',]
    # list_portid = [3009,]
    # queue_processedData = multiprocessing.Queue()
    # queue_sendtohardware = multiprocessing.Queue()
    # SoftwareAddress = '192.168.1.176'
    # MachineAddress = '192.168.1.10'
    # port = 7#接收跟发送统一用一个port
    # value_netconnectflag = multiprocessing.Value('i', 0)
    # exm = CreateHardwareUDPClient(SoftwareAddress, MachineAddress, port, queue_sendtohardware, value_netconnectflag,
    #              queue_processedData, 6)
    # exm.start()
    # queue_sendtohardware.put('1')

    import socket

    HostPort = ('192.168.0.10', 7)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # 创建UDP套接字
    sock.bind(('192.168.0.20', 11))  # 服务器端绑定端口
    # while True:
    # data = '12'
    # sock.sendto(data.encode(),('192.168.0.10', 7))
    datas = struct.pack('6B', 0xdd, 11, 0, 0, 0, 0)
    sock.sendto(datas, ('192.168.0.10', 7))
    data, addr = sock.recvfrom(6)
    print ('-------')
    print(data)


    # while True:
        # user_input = input('>>>:')
        # if user_input == 'quit': break
        # sock.sendto(user_input.encode(), HostPort)  # 指定地址端口发送数据，数据必须encode
        # data = sock.recv(6)
        # print (data)
    # sock.close()
