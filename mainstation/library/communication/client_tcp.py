#coding=utf-8
'''
Created on 2017年07月05号

@author: huber yao
'''
import struct
import time
import threading
# from project
import multiprocessing
from twisted.internet import protocol
from twisted.internet import reactor

class CreateTCPClientFactory(multiprocessing.Process):
    '''
          生成TCP客户端工厂，一个端口号对应一个工厂，一个工厂对应一个协议，一个协议对应一个客户端
    '''
    def __init__(self, f_latertime, list_serverip, list_portid, list_queue_received, list_queue_send, list_value_netconnectflag):
        super(CreateTCPClientFactory,self).__init__()
        '''
                   初始化函数，几个形参是本模块对外接口，几个列表长度一致，长度值代表有几个TCP客户端
        '''
        self.f_latertime = f_latertime 
        self.list_serverip = list_serverip   #端口号列表，模块接口
        self.list_portid = list_portid   #端口号列表，模块接口
        self.list_queue_received = list_queue_received #接收数据队列列表，模块接口
        self.list_queue_send = list_queue_send  #发送数据队列列表，模块接口
        self.list_value_netconnectflag = list_value_netconnectflag  #连接标志位列表，模块接口
        
        self._ClientFactoryList = []
        self.list_curdatalist = []
        for i in range(len(self.list_portid)):
            self.list_curdatalist.append([])
    
    def senddata(self):
        '''
                    不断读取各工厂发送队列数据，通过工厂send方法调用协议send方法将数据写到网络上，传给客户端,间隔时间为外部传入
        '''
        for i in range(len(self._ClientFactoryList)):
            if self._ClientFactoryList[i] is not None:
                self.list_curdatalist[i] = []
                while(not self.list_queue_send[i].empty()):               
                    self.list_curdatalist[i].append(self.list_queue_send[i].get())  
                if len(self.list_curdatalist[i]) > 0:
                    curdata = ''.join(self.list_curdatalist[i])        
                    self._ClientFactoryList[i].send(str(curdata))
        reactor.callLater(self.f_latertime, self.senddata)
        
    def run(self):  
        '''
                    根据端口号列表长度确定客户端个数，一个服务器工厂绑定一个端口号
        '''
        self._ClientFactoryList = []
        for i in range(len(self.list_portid)):
            if self.list_portid[i] is not None:
                
                self._ClientFactoryList.append(_ClientFactory(self.list_queue_received[i], self.list_queue_send[i], self.list_value_netconnectflag[i]))
                reactor.connectTCP(self.list_serverip[i], self.list_portid[i], self._ClientFactoryList[i]) 
            else:
                self._ClientFactoryList.append(None)
        reactor.callLater(self.f_latertime, self.senddata)
        reactor.run(installSignalHandlers = False) 
        
class _ClientProtocol(protocol.Protocol):
    '''
         协议类，处理最底层的连接，断开，接收，发送
    '''   
    def connectionMade(self):
        '''
        TCP握手成功触发此方法，连接成功后连接标志位置1
        '''
        self.factory.protocoler = self
        self.factory.value_netconnectflag.value = 1
        
        
    def dataReceived(self, data):
        '''
                   接收到客户端数据后调用此方法，将数据塞给工厂的接受数据队列
        '''
        self.factory.queue_rev.put(data)
        
    def send(self,data):   
        '''
                     将数据写到网络上，传给客户端
        '''
        self.transport.write(data)
        
    def connectionLost(self, reason):
        '''
                    连接断开后触发此方法，连接断开后连接标志位置1
        '''
        self.factory.protocoler = None
        self.factory.value_netconnectflag.value = 0
        self.transport.loseConnection()
        

class _ClientFactory(protocol.ClientFactory):
    '''
          工厂类，一个工厂可生产若干协议，本程序为了识别客户端方便，只生产一个协议，一个协议对应一个客户端，所以本程序一个客户端对应一个端口号
    '''
    protocol = _ClientProtocol #确定工厂生产的协议类型
    def __init__(self, queue_rev, queue_send, value_netconnectflag):
        '''
                   工厂成员变量初始化，同时开启一线程不断将发送队列数据通过协议写到网络上，传给客户端
        '''
        self.queue_rev = queue_rev
        self.queue_send = queue_send
        self.value_netconnectflag = value_netconnectflag
        self.protocoler = None
        
    
    def clientConnectionFailed(self, connector, reason):
        connector.connect()
 
    def clientConnectionLost(self, connector, reason):
        connector.connect()
        
    def send(self,data):
        '''
                   调用协议的send方法， 将数据写到网络上，传给客户端
        '''
        if self.protocoler is not None:
            self.protocoler.send(data)

if __name__ == '__main__':

    f_latertime = 0.1
    list_serverip = ['127.0.0.1',]
    list_portid = [3009,]
    list_queue_received = [multiprocessing.Queue() for i in range(1)]
    list_queue_send = [multiprocessing.Queue() for i in range(1)]
    list_value_netconnectflag = [multiprocessing.Value('i', 0) for i in range(1)]
    exm = CreateTCPClientFactory(f_latertime, list_serverip, list_portid, list_queue_received, list_queue_send, list_value_netconnectflag)
    exm.start()
    while(1):
        pass
    
    
    