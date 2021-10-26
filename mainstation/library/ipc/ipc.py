#coding=utf-8

import struct
import multiprocessing
import imc_msg
from library.common.readconfig import readconfig
from library.ipc import ipc_tool



class DataProcess(multiprocessing.Process):
    """
    接收各子站接收数据队列中数据，如果接收来的数据是结果数据，解包结果数据为字典，
    一方面分析后可通过硬件发送数据队列控制硬件，一方面塞给处理数据队列刷新结果显示界面。
    其它数据，直接加头字符串塞到处理数据队列刷新各界面.

    2020.02.04
    总结来说，其作用就是将主站外的数据送至其应该出现的位置，在这里不要放太多逻辑判断的东西，
    直接将结果送至目的地即可.


    """

    DATAHEADFORMAT = imc_msg.MSGHeader.dataheadformat
    NLENOFDATAHEADFORMAT = struct.calcsize(DATAHEADFORMAT)
    
    def __init__(self, n_istranslate):
        '''
                    初始化
        '''
        multiprocessing.Process.__init__(self)
        
        self.list_queue_received = ipc_tool.getlist_queue_received(self)  #接收子站数据队列列表
        self.queue_processedData = ipc_tool.getqueue_processedData(self)  #将子站接收来消息解包后塞到此队列，主要起缓冲作用，避免解包数据操作影响网络通讯和刷新界面
        self.queue_processedData_monitoring = ipc_tool.getqueue_processedData_monitoring(self)
        self.queue_processedData_param = ipc_tool.getqueue_processedData_param(self)
        self.value_runflag = ipc_tool.getvalue_runflag(self)
        self.value_id = ipc_tool.getvalue_id(self)
        self.queue_kxlog = ipc_tool.getqueue_queue_kxlog()
        self.n_istranslate = n_istranslate

        self.dict_id2stationresultlist = {} #{图像1: [结果1, 结果2, ..], 图像2: [结果1, 结果2, ..], ..}
        self.dict_id2stationresultdatalist = {}
        self.dict_sysparams = readconfig()
        self.n_stationnum = len(self.list_queue_received)  #工位数
        self.list_buf = []
        for pos in range(self.n_stationnum):
            self.list_buf.append( b'' )
    
    def sendonerunlog(self, pos, databuf):
        """
        Parameters
        ----------
        pos: 站点号
        databuf：数据，2020年开始databuf格式为 缺陷等级('i') + 日志长度('i') + 日志信息(n * 's')

        """
        loglevel,nloglen = struct.unpack("2i",databuf[:struct.calcsize("2i")])
        #字符串后面有一段空白，然后解码
        # exceptioninf = struct.unpack("%ds"%nloglen, databuf[struct.calcsize("2i"):])[0]
        exceptioninf = databuf[struct.calcsize("2i"):]
        exceptioninf = exceptioninf.split(str.encode('\x00'))[0]
        try:
            exceptioninf = exceptioninf.decode('gbk')  # , 'ignore')
        except UnicodeDecodeError as e:
            strtmp = 'UnicodeDecodeError::' + str(e)
            strtmp += 'exceptioninf:' + str(len(exceptioninf)) + str(exceptioninf)
            exceptioninf = strtmp
        self.queue_kxlog.put((str(pos), loglevel, exceptioninf))

    # def _dealresultmsg(self, pos, databuf):
    #     ''''
    #     处理结果消息，
    #     1.解包数据，判断指定卡是否收集够所有工位消息，
    #     2.维护一个无限长队列。如果卡Id各工位消息收集够，直接删掉这一项。
    #     3.收集够后，一方面将未解包各工位数据组成列表，发给主界面，主界面发给结果显示界面。
    #                                               一方面根据各工位解包数据，控制硬件
    #     '''
    #     h_checkresult = CheckResultInfo()
    #     h_checkresult.ParseFromString(databuf) #解包结果数据到字典
    #     #1, 维护无限长队列
    #     if h_checkresult.nId not in self.dict_id2stationresultlist.keys():
    #         self.dict_id2stationresultlist[h_checkresult.nId] = [None,] * self.n_stationnum
    #         self.dict_id2stationresultdatalist[h_checkresult.nId] = [None,] * self.n_stationnum
    #     #2.1, 判断数据中站号是否不对
    #     if h_checkresult.nStationId >= self.n_stationnum:
    #         s_time = time.strftime( '%X', time.localtime( time.time() ) )
    #         if self.n_istranslate:
    #             s_msg = u'收到站id异常，为：'+str(h_checkresult.nStationId)
    #         else:
    #             s_msg = u'Receive the station ID as exception, the nStationId is:'+str(h_checkresult.nStationId)
    #         self.queue_processedData.put((imc_msg.MSGSENDER.DataProcess, imc_msg.MSG_LOG,
    #                                       imc_msg.LOGLEVEL.ERR, s_time, s_msg))
    #         return
    #     # 2.1, 判断数据中卡号结果是否已经有了
    #     if self.dict_id2stationresultlist[h_checkresult.nId][h_checkresult.nStationId] is not None :
    #         s_time = time.strftime( '%X', time.localtime( time.time() ) )
    #         if self.n_istranslate:
    #             s_msg = u'收到子站消息异常，当前卡id和站id对应结果已存在，当前卡id为：'+str(h_checkresult.nId)+' 当前站id为：'+str(h_checkresult.nStationId)
    #         else:
    #             s_msg = u'The sub station received message is abnormal, the ID card and the corresponding results of ID station already exists, the ID card::'\
    #             +str(h_checkresult.nId)+' The current stationid is: '+str(h_checkresult.nStationId)+str(h_checkresult.nStationId)
    #         self.queue_kxlog.put(("ipc", imc_msg.LOGLEVEL.ERR, s_msg))
    #         return
    #     # 3. 为数据赋值
    #     self.dict_id2stationresultlist[h_checkresult.nId][h_checkresult.nStationId] = h_checkresult.nQualityLevel
    #     self.dict_id2stationresultdatalist[h_checkresult.nId][h_checkresult.nStationId] = databuf
    #     list_curidstationresult = self.dict_id2stationresultlist[h_checkresult.nId]
    #     list_curidstationresultdata = self.dict_id2stationresultdatalist[h_checkresult.nId]
    #
    #     # 4. 各个站的队列对齐之后取出结果
    #     if self._judgeisfull(list_curidstationresult):
    #         # 2018.08.29 下面这些修改一下，到时在这里解出结果是否是坏品，一般N个相机N个判断，
    #         # 根据项目需要确定这个N个相机的结果是 与关系 还是 或关系
    #
    #         self.value_id.value = h_checkresult.nId
    #         self.queue_processedData.put((list_curidstationresult.index(max(list_curidstationresult)),
    #                                       imc_msg.MSG_CHECK_RESULT, list_curidstationresultdata))
    #
    #         #处理结果一并传送，0是随便写
    #         self._PutDataToMainwindow(0, imc_msg.MSG_CHECK_RESULT, list_curidstationresultdata)
    #         self._PutDataToMonitoring(0, imc_msg.MSG_CHECK_RESULT, list_curidstationresultdata)
    #
    #         # 下面这段代码对结果进行判废，可取是否需要
    #         # if max(list_curidstationresult) >= GlobalQuality.BAD:
    #         #     self.queue_processedData.put((list_curidstationresult.index(max(list_curidstationresult)),
    #         #                               imc_msg.MSG_CHECK_RESULT_BAD, list_curidstationresult,
    #         #                                   h_checkresult.nId))
    #         del self.dict_id2stationresultlist[h_checkresult.nId]
    #         del self.dict_id2stationresultdatalist[h_checkresult.nId]

    def _judgeisfull(self, list_curidstationresult):
        if None in list_curidstationresult:
            return False
        else:
            return True

    def _dealresultmsg_sensitivity(self, stationid, databuf):
        """
        处理灵敏度界面结果消息，但现在基本用不到，所以暂不做保留
        """
        pass

    def _PutDataToMonitoring(self, n_stationid, n_msgtype, tuple_data):
        self.queue_processedData_monitoring.put((n_stationid, n_msgtype, tuple_data))

    def _PutDataToMainwindow(self, n_stationid, n_msgtype, tuple_data):
        self.queue_processedData.put((n_stationid, n_msgtype, tuple_data))

    def _PutDataToParamWidget(self, n_stationid, n_msgtype, tuple_data):
        self.queue_processedData_param.put((n_stationid, n_msgtype, tuple_data))


    def run(self): 
        """
        将各个站的数据进行初次解包，注意，解包是解包TCP的数据头，而不是那些类似于用os或protobuf封装
        起来的数据，也即压到各个公共队列里的databuf是子站切切实实传过来的数据，并不包含数据头的那些
        """
        while True:
            if self.value_runflag.value == imc_msg.RUNFLAG.STOP:
                self.dict_id2stationresultlist = {}
                self.dict_id2stationresultdatalist = {}
            for pos in range(self.n_stationnum):
                if not self.list_queue_received[pos].empty():
                    while(not self.list_queue_received[pos].empty()):
                        s_temp = self.list_queue_received[pos].get()
                        self.list_buf[pos] += s_temp

                #判断数据长度是否大于数据头长度        
                if len(self.list_buf[pos])  >= self.NLENOFDATAHEADFORMAT:
                    curheaddata = self.list_buf[pos][:self.NLENOFDATAHEADFORMAT]
                    (n_stationtype, n_stationid, n_msgtype, n_subtype, n_extdatasize, n_headerextdatasize,
                       n_selfcheck, n_checksum, s_headerextdata) = struct.unpack(self.DATAHEADFORMAT, curheaddata)
                    #判断是否有丢失的数据
                    if self.NLENOFDATAHEADFORMAT + n_extdatasize > len(self.list_buf[pos]):
                        continue

                    #如果数据足够，取出数据分析，并将指针后移    
                    if self.NLENOFDATAHEADFORMAT + n_extdatasize <= len(self.list_buf[pos]):

                        #这里的databuf就是在调用子站的sendmsg时最后写入的数据
                        dataBuf = self.list_buf[pos][self.NLENOFDATAHEADFORMAT : self.NLENOFDATAHEADFORMAT + n_extdatasize]
                        self.list_buf[pos] = self.list_buf[pos][(self.NLENOFDATAHEADFORMAT+n_extdatasize):]
                        #分析数据，并将分析结果转手给主界面、日志界面、参数界面等
                        if n_msgtype == imc_msg.MSG_LOG:
                            self.sendonerunlog(n_stationid, dataBuf)

                        elif n_msgtype == imc_msg.MSG_CHECK_RESULT:
                            if self.value_runflag.value != imc_msg.RUNFLAG.STOP:
                                self._PutDataToMainwindow(n_stationid, n_msgtype, dataBuf)
                                self._PutDataToMonitoring(n_stationid, n_msgtype, dataBuf)
                            continue#打破

                        elif n_msgtype in imc_msg.list_mainwindow_module:
                            self._PutDataToMainwindow(n_stationid, n_msgtype, dataBuf)

                        elif n_msgtype in imc_msg.list_params_module:
                            self._PutDataToParamWidget(n_stationid, n_msgtype, dataBuf)

                        elif n_msgtype in imc_msg.list_monitoring_module:
                            self._PutDataToMonitoring(n_stationid, n_msgtype, dataBuf)




  
