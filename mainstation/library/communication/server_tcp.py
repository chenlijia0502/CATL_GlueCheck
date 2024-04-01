#coding=utf-8
'''
Created on 2017年07月05号

@author: huber yao
'''

import struct
import time
import threading
import multiprocessing

import random
#import chardet
from twisted.internet import protocol
from twisted.internet import reactor

import imc_msg
from library.ipc import ipc_tool


def convertIPAndPortToInt(ipPortTuple, existedList):
    ip, port = ipPortTuple
    numList = ip.split('.')
    ipNum = sum([int(number) for number in numList])
    while (ipNum + port + random.randint(1, 100)) in existedList:
        ipNum = sum([int(number) for number in numList])
    return ipNum

def generateHandShakeData():#握手数据包
    imc_msg.MSGHeader.n_stationid = 0
    imc_msg.MSGHeader.n_msgtype = imc_msg.GlobalMsgSend.MSG_HANDSHAKE_SEND
    imc_msg.MSGHeader.n_extdatasize = 0
    
    list_headdata = [imc_msg.MSGHeader.n_stationtype, imc_msg.MSGHeader.n_stationid, imc_msg.MSGHeader.n_msgtype,
                     imc_msg.MSGHeader.n_subtype, imc_msg.MSGHeader.n_extdatasize, imc_msg.MSGHeader.n_headerextdatasize,
                     imc_msg.MSGHeader.n_selfcheck, imc_msg.MSGHeader.n_checksum, imc_msg.MSGHeader.s_headerextdata]
    head = struct.pack(imc_msg.MSGHeader.dataheadformat, *list_headdata)
    return head


class CreateTCPServerFactory(multiprocessing.Process):
    '''
          生成TCP服务器工厂，一个端口号对应一个工厂，一个工厂对应一个协议，一个协议对应一个客户端，所以多客户端多协议

          2020.02.21
          服务器这有点复杂，可以认为利用了twisted的协议类、工厂类的函数进行的重写，完成了一系列的连接。
    '''

    def __init__(self, f_latertime, portid, maxCameraPerStation):
        '''
           初始化函数，几个形参是本模块对外接口，几个列表长度一致，长度值代表有几个TCP客户端
        '''

        super(CreateTCPServerFactory, self).__init__()
        self.f_latertime = f_latertime
        self.serverportid = portid  # 端口号
        self.list_queue_received = ipc_tool.getlist_queue_received(self)  # 接收数据队列列表，模块接口
        self.list_queue_send = ipc_tool.getlist_queue_send(self)  # 发送数据队列列表，模块接口
        self.queue_processedData = ipc_tool.getqueue_processedData(self)
        self.list_value_netconnectflag = ipc_tool.getlist_value_netconnectflag()  # 连接标志位列表，模块接口
        self.queue_kxlog = ipc_tool.getqueue_queue_kxlog()
        self.maxCameraPerStation = maxCameraPerStation

    def senddata(self):
        '''
        根据发送时所提供的接收方stationId可以区分要发给谁，所以发送队列只需要一个（循环调用自身）
        '''
        for i in range(len(self.list_queue_send)):
            while not self.list_queue_send[i].empty():
                self.serverFactory.send(self.list_queue_send[i].get())
        reactor.callLater(self.f_latertime, self.senddata)

    def run(self):
        '''
            根据端口号列表长度确定客户端个数，一个服务器工厂绑定一个端口号
        '''
        self.serverFactory = _ServerFactory(self.list_queue_received, self.queue_processedData,
                                            self.list_value_netconnectflag, self.queue_kxlog,
                                            self.maxCameraPerStation)
        reactor.listenTCP(self.serverportid, self.serverFactory)
        reactor.callLater(self.f_latertime, self.senddata)  # 延时f_latertime然后调用senddata
        reactor.run(installSignalHandlers=False)


class _ServerProtocol(protocol.Protocol):
    """
         协议类，处理最底层的连接，断开，接收，发送
    """

    def __init__(self, revQueueIndex):
        """
        这里的stationIndex记录的既不是站点号也不是相机号，由于这里有多个站点，每个站点又有多个相机（最多有4个），
        所以这里把属于同一个站点的几个子站号记录在连续的几个数中，如站点1有3个相机，
        它占据0,1,2三个数，站点2有两个相机，它占据4,5两个数，这个数整除4的商就是站点号，对4取摸就是相机号。
        """
        self.revQueueIndex = revQueueIndex
        self.stationIndex = None

    def connectionMade(self):  # 重写
        """
        TCP握手成功触发此方法，连接成功后连接标志位置1
        """
        self.stationIndex = None
        self.send(generateHandShakeData())

    def setStationIndex(self, stationId):
        """
        给工厂的协议字典设置，键值为  id：协议，字典存在的目的是发送时可以索引
        Parameters
        ----------
        stationId : 该协议对应的站点号

        Returns
        -------

        """
        if stationId in self.factory.clientDict:
            s_msg = u'子站站点号配置重复,请重新配置站点号并重启程序！'
            self.factory.queue_kxlog.put(("serve tcp", imc_msg.LOGLEVEL.WARN, s_msg))
        else:
            self.stationIndex = stationId
            self.factory.clientDict[stationId] = self
            # realStationId, realCameraId = stationId // self.factory.maxCameraPerStation, stationId % self.factory.maxCameraPerStation
            # if realStationId < len(self.factory.list_valueNetconnectflag) and realCameraId < len(
            #         self.factory.list_valueNetconnectflag[realStationId]):
            self.factory.list_valueNetconnectflag[stationId].value = 1

    def dataReceived(self, data):  # 重写
        """
            接收到客户端数据后调用此方法，将数据塞给工厂的接受数据队列。
            而第一句话是为了保证只进入一次，但是data的解包方式的写法在这里算是不规范了，
            因为如果数据头（imc_msg.MSGHeader）格式改变，那么子站id就不是现在这个位置
        """
        if self.stationIndex is None:
            self.setStationIndex(struct.unpack('i', data[struct.calcsize('i'):struct.calcsize('2i')])[0])
        self.factory.list_queueRev[self.revQueueIndex].put(data)

    def send(self, data):
        """
                     将数据写到网络上，传给客户端
        """
        self.transport.write(data)

    def connectionLost(self, reason):  # 重写
        """
                    连接断开后触发此方法，连接断开后连接标志位置0
        """
        if self.stationIndex in self.factory.clientDict:
            del self.factory.clientDict[self.stationIndex]
        self.factory.list_queueRevFlag[self.revQueueIndex] = False
        # if self.stationIndex is not None:
        #     realStationId, realCameraId = self.stationIndex // self.factory.maxCameraPerStation, self.stationIndex % self.factory.maxCameraPerStation
        #     if realStationId < len(self.factory.list_valueNetconnectflag) and realCameraId < len(
        #             self.factory.list_valueNetconnectflag[realStationId]):
        if self.stationIndex is not None:
            self.factory.list_valueNetconnectflag[self.stationIndex].value = 0
            self.stationIndex = None


class _ServerFactory(protocol.ServerFactory):
    """
          20190923 工厂类，一个工厂可生产若干协议，一个协议对应一个客户端

    """

    def __init__(self, queue_revlist, queue_processedData, value_netconnectflaglist, queue_kxlog, maxCameraPerStation):
        """
                   工厂成员变量初始化，同时开启一线程不断将发送队列数据通过协议写到网络上，传给客户端
        """
        self.list_queueRev = queue_revlist
        self.list_queueRevFlag = [False] * len(self.list_queueRev)
        self.queue_processedData = queue_processedData
        self.list_valueNetconnectflag = value_netconnectflaglist
        self.queue_kxlog = queue_kxlog
        self.maxCameraPerStation = maxCameraPerStation
        self.clientDict = {}# 键值为  子站id:对于的protocol类

    def nextAvailableQueue(self):
        """
        获取还未“填入”的索引位置，这个索引只是接收队列的索引，跟子站id没有关系
        """
        if self.list_queueRevFlag.count(False) > 0:
            index = self.list_queueRevFlag.index(False)
            self.list_queueRevFlag[index] = True
            return index
        else:
            return None

    def buildProtocol(self, addr):  # 重写
        """
        子站启动时调用，一个子站调用一次(多协议才是twisted最重要的部分)，创建一个协议对应连接的客户端
        值得注意的事这里的索引跟子站的id没有任何关系，这个索引只是决定了接收队列的顺序
        :param addr:
        :return:
        """
        nextIndex = self.nextAvailableQueue()
        if nextIndex is not None:
            p = _ServerProtocol(nextIndex)
            p.factory = self
            return p
        else:
            s_msg = '子站个数超出了配置文件中指定的相机个数%d，请确认子站个数是否配置正确！' % len(self.list_queueRev)
            self.queue_kxlog.put(("serve tcp", imc_msg.LOGLEVEL.ERR, s_msg))

    def send(self, data):
        """
                调用协议的send方法， 将数据写到网络上，传给客户端
        """
        stationId, buf = data
        if stationId == -1:
            for client in list(self.clientDict.values()):
                client.send(buf)
        elif stationId in self.clientDict:
            self.clientDict[stationId].send(buf)




"""
    下面对应的是多个子站对应一个站点，每个子站都有一个netid，但是他们共用一份配置，但这种写法跟一对一的思路不符合，
    所以就此分家，有可能会形成两个分支
"""

# class CreateTCPServerFactory(multiprocessing.Process):
#     '''
#           生成TCP服务器工厂，一个端口号对应一个工厂，一个工厂对应一个协议，一个协议对应一个客户端，所以多客户端多协议
#
#           2020.02.05 后续做的优化在于改进了N个相机对应一个站，这样的好处在于当几个相机是同一配置时，
#           建模方便许多。但缺点是跟一对一建模的思路有些违背
#     '''
#     def __init__(self, f_latertime, portid, maxCameraPerStation):
#         '''
#            初始化函数，几个形参是本模块对外接口，几个列表长度一致，长度值代表有几个TCP客户端
#         '''
#         super(CreateTCPServerFactory, self).__init__()
#         self.f_latertime = f_latertime
#         self.serverportid = portid   #端口号
#         self.list_queue_received = ipc_tool.getlist_queue_received(self) #接收数据队列列表，模块接口
#         self.list_queue_send = ipc_tool.getlist_queue_send(self)  #发送数据队列列表，模块接口
#         self.queue_processedData = ipc_tool.getqueue_processedData(self)
#         self.list_value_netconnectflag = ipc_tool.getlist_value_netconnectflag()  #连接标志位列表，模块接口
#         self.maxCameraPerStation = maxCameraPerStation
#
#     def senddata(self):
#         '''
#         根据发送时所提供的接收方stationId可以区分要发给谁，所以发送队列只需要一个（循环调用自身）
#         '''
#         if len(self.list_queue_send) > 0:
#             while(not self.list_queue_send[0].empty()):
#                 self.serverFactory.send(self.list_queue_send[0].get())
#         reactor.callLater(self.f_latertime, self.senddata)
#
#     def run(self):
#         '''
#             根据端口号列表长度确定客户端个数，一个服务器工厂绑定一个端口号
#         '''
#         self.serverFactory = _ServerFactory(self.list_queue_received, self.queue_processedData, self.list_value_netconnectflag, self.maxCameraPerStation)
#         reactor.listenTCP(self.serverportid, self.serverFactory)
#         reactor.callLater(self.f_latertime, self.senddata)#延时f_latertime然后调用senddata
#         reactor.run(installSignalHandlers = False)
#
#
#
#
#
# class _ServerProtocol(protocol.Protocol):
#     '''
#          协议类，处理最底层的连接，断开，接收，发送
#     '''
#     def __init__(self, revQueueIndex):
#         '''
#         这里的stationIndex记录的既不是站点号也不是相机号，由于这里有多个站点，每个站点又有多个相机（最多有4个），
#         所以这里把属于同一个站点的几个子站号记录在连续的几个数中，如站点1有3个相机，
#         它占据0,1,2三个数，站点2有两个相机，它占据4,5两个数，这个数整除4的商就是站点号，对4取摸就是相机号。
#         '''
#         self.revQueueIndex = revQueueIndex
#         self.stationIndex = None
#
#     def connectionMade(self):#重写
#         '''
#         TCP握手成功触发此方法，连接成功后连接标志位置1
#         '''
#         self.stationIndex = None
#         self.send(generateHandShakeData())
#
#     def setStationIndex(self, stationId):
#         if stationId in self.factory.clientDict:
#             s_time = time.strftime( '%X', time.localtime( time.time() ) )
#             s_msg = u'子站站点号配置重复,请重新配置站点号并重启程序！'
#             ipc_tool.kxlog("serve tcp", imc_msg.LOGLEVEL.WARN, s_msg)
# #             self.transport.loseConnection()
#         else:
#             self.stationIndex = stationId
#             self.factory.clientDict[stationId] = self
#             realStationId, realCameraId = stationId // self.factory.maxCameraPerStation, stationId % self.factory.maxCameraPerStation
#             if realStationId < len(self.factory.list_valueNetconnectflag) and realCameraId < len(self.factory.list_valueNetconnectflag[realStationId]):
#                 self.factory.list_valueNetconnectflag[realStationId][realCameraId].value = 1
#
#     def dataReceived(self, data):#重写
#         '''
#                    接收到客户端数据后调用此方法，将数据塞给工厂的接受数据队列
#         '''
#         # print ('datareceive')
#         if self.stationIndex is None:
#             self.setStationIndex(struct.unpack('i', data[struct.calcsize('i'):struct.calcsize('2i')])[0])
#         self.factory.list_queueRev[self.revQueueIndex].put(data)
#
#     def send(self, data):
#         '''
#                      将数据写到网络上，传给客户端
#         '''
#         self.transport.write(data)
#
#     def connectionLost(self, reason):#重写
#         '''
#                     连接断开后触发此方法，连接断开后连接标志位置1
#         '''
#         if self.stationIndex in self.factory.clientDict:
#             del self.factory.clientDict[self.stationIndex]
#         self.factory.list_queueRevFlag[self.revQueueIndex] = False
#         if self.stationIndex is not None:
#             realStationId, realCameraId = self.stationIndex // self.factory.maxCameraPerStation, self.stationIndex % self.factory.maxCameraPerStation
#             if realStationId < len(self.factory.list_valueNetconnectflag) and realCameraId < len(self.factory.list_valueNetconnectflag[realStationId]):
#                 self.factory.list_valueNetconnectflag[realStationId][realCameraId].value = 0
#
#         self.stationIndex = None
#
#
# class _ServerFactory(protocol.ServerFactory):
#     '''
#           20190923 工厂类，一个工厂可生产若干协议，一个协议对应一个客户端
#
#     '''
#     protocol = _ServerProtocol #确定工厂生产的协议类型
#
#     def __init__(self, queue_revlist, queue_processedData, value_netconnectflaglist, maxCameraPerStation):
#         '''
#                    工厂成员变量初始化，同时开启一线程不断将发送队列数据通过协议写到网络上，传给客户端
#         '''
#         self.list_queueRev = queue_revlist
#         self.list_queueRevFlag = [False] * len(self.list_queueRev)
#         self.queue_processedData = queue_processedData
#         self.list_valueNetconnectflag = value_netconnectflaglist
#         self.maxCameraPerStation = maxCameraPerStation
#         self.clientDict = {}
#
#     def nextAvailableQueue(self):
#         if self.list_queueRevFlag.count(False) > 0:
#             index = self.list_queueRevFlag.index(False)
#             self.list_queueRevFlag[index] = True
#             return index
#         else:
#             return None
#
#     def buildProtocol(self, addr):#重写
#         """
#         子站启动时调用，一个子站调用一次(多协议才是twisted最重要的部分)
#         :param addr:
#         :return:
#         """
#         # print ('build')
#         nextIndex = self.nextAvailableQueue()
#         print (nextIndex)
#         if nextIndex is not None:
#             p = self.protocol(nextIndex)
#             p.factory = self
#             return p
#         else:
#             s_time = time.strftime('%X', time.localtime(time.time()))
#             s_msg = '子站个数超出了配置文件中指定的相机个数%d，请确认子站个数是否配置正确！'%len(self.list_queueRev)
#             ipc_tool.kxlog("serve tcp", imc_msg.LOGLEVEL.ERR, s_msg)
#
#     def send(self, data):
#         '''
#                    调用协议的send方法， 将数据写到网络上，传给客户端
#         '''
#         stationId, buf = data
#         if stationId == -1:
#             for client in list(self.clientDict.values()):
#                 client.send(buf)
#         elif stationId in self.clientDict:
#             self.clientDict[stationId].send(buf)
        

