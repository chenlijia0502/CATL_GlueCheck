import multiprocessing
import serial
from library.ipc import ipc_tool
import threading
import time
import imc_msg
import struct





class CreateSerialProcess(multiprocessing.Process):
    _RECMSGLEN = 7
    _BAURATE = 115200
    def __init__(self, scom):
        super(CreateSerialProcess, self).__init__()
        self.queue_sendtohardware = ipc_tool.getqueue_hardware_send()#发送数据
        self.queue_hardware_receive = ipc_tool.getqueue_hardware_rereceived()
        self.com = scom
        self.verticalTime = 0   #需要计算
        self.horizontalTime = 0 #次数一定
        self.roiPos = ()
        self.list_queue_send = ipc_tool.getlist_queue_send(self)


    def run(self):
        self.mySeria = serial.Serial(port=self.com, baudrate=self._BAURATE)# 波特率比较固定，没必要配置

        thread_send = threading.Thread(target=self.serialThread)
        thread_send.start()
        thread_receive = threading.Thread(target=self.serialRecThread)
        thread_receive.start()

    def serialThread(self):
        self.flag = 0
        while True:
            if self.mySeria.isOpen():
                if self.queue_sendtohardware.empty() == False:
                    info = self.queue_sendtohardware.get()
                    self.mySeria.write(info)
                else:
                    time.sleep(0.2)

                    # self.verticalTime = info[0]
                    # self.allorRoi = info[1]
                    # #MSG_RUN = [1, 4, 2, 134, 160, 0, 1]
                    #
                    # if self.allorRoi == 0:#拍整张图, 传入参数是纵向扫描的次数
                    #     self.verticalTime = 2
                    #     for i in range(0, self.verticalTime):
                    #         if i / 2 == 0 or i == 0:#偶数次，正方向走
                    #             self.inputInstruction(MSG_RUN, MSG_STARTMOTOR)
                    #             self.sendmsg(0, imc_msg.GlobalMsgSend.MSG_ENDMOVE, '123')
                    #         else:#奇数次，反方向0
                    #             self.inputInstruction(MSG_RUNBACKY, MSG_STARTMOTOR)
                    #             self.sendmsg(0, imc_msg.GlobalMsgSend.MSG_ENDMOVE, '123')
                    #         if i != self.verticalTime - 1: #最后一次不向y轴移动
                    #             self.inputInstruction(MSG_RUNX, MSG_STARTMOTORX)
                    #             #self.sendmsg(0, imc_msg.GlobalMsgSend.MSG_ENDMOVE, '123')
                    #     if self.verticalTime > 0:
                    #         self.returnStartPoint(MSG_RETURN)
                    # else:#拍ROI区域，传入起始的X轴坐标，和终止的X轴坐标
                    #     #1,将位移的脉冲数修改成指令，先往X轴移动到roi的顶部
                    #     self.inputInstruction(MSG_RUN, MSG_STARTMOTOR)
                    # self.sendmsg(0, imc_msg.GlobalMsgSend.MSG_ENDALLMOVE, '123')#整个流程结束，开始上传拼接的图

    def serialRecThread(self):
        while True:
            if self.mySeria.isOpen():
                data = self.mySeria.read(self._RECMSGLEN)
                print ("com3 receive data: ",self.str_to_hex(data))
                self.queue_hardware_receive.put(data)
            else:
                time.sleep(0.2)



    def str_to_hex(self, data):
        sdata = str(data)[2:-1]
        list_data = sdata.split('\\x')
        list_hex = []
        for strdata in list_data:
            if strdata != '':
                list_hex.append(int(strdata, 16))
        return list_hex
        # print (str(data)[2:-1])
        # print (len(data))
        # list_result = struct.unpack("7h", data)
        # print (list_result)
        # return list_result


    def length2instruction(self, length, xOrY):
        a = hex(length & 0xFFFFFFFF)
        Difference = 10 - len(a)
        str = ''
        kk = a
        if Difference != 0:
            for x in range(0, Difference):
                str += '0'
            kk = a[0:2] + str + a[2:]
        num1 = int(kk[2:4], 16)
        num2 = int(kk[4:6], 16)
        num3 = int(kk[6:8], 16)
        num4 = int(kk[8:10], 16)
        if xOrY == 0:
            num5 = 0x01
        else:
            num5 = 0x02
        self.command = [0x01, 0x04, num5, num3, num4, num1, num2]


    def inputInstruction(self, instruction, startmotor):
        self.mySeria.write(instruction)
        while True:
            step1 = self.mySeria.inWaiting()
            if step1 > 6:
                a1 = self.mySeria.read(step1)
                #print('step1', step1, a1)
                time.sleep(2)
                self.mySeria.write(startmotor)
                while True:
                    step2 = self.mySeria.inWaiting()
                    if step2 > 5:
                        a1 = self.mySeria.read(step2)
                        print('step2', step2, a1)
                        while True:
                            step3 = self.mySeria.inWaiting()
                            if step3 > 5:
                                a1 = self.mySeria.read(step3)
                                print('step3', step2, a1)
                                time.sleep(2)
                                return

    def returnStartPoint(self, backInstruction):
        self.mySeria.write(backInstruction)
        while True:
            a = self.mySeria.inWaiting()
            if a > 11:
                step1 = self.mySeria.read(a)
                print('return step1', step1)
                while True:
                    b = self.mySeria.inWaiting()
                    if b > 5:
                        step2 = self.mySeria.read(b)
                        print('return step2', step2)
                        while True:
                            c = self.mySeria.inWaiting()
                            if c > 5:
                                step3 = self.mySeria.read(c)
                                print('return step3', step3)
                                # while True:
                                #     d = self.mySeria.inWaiting()
                                #     if d > 5:
                                #         step4 = self.mySeria.read(d)
                                #         print('return step4', step4)
                                #         return

    def byte2int(self, byte):
        vd = [hex(x) for x in byte]
        cd = tuple(vd)
        result = tuple((x[2:] for x in cd))
        result = result[0:3]
        self.tuple = tuple(int(x) for x in result)

    def sendmsg(self, n_stationid, n_msgtype, s_extdata=b'', classobj=None):
        imc_msg.MSGHeader.n_stationid = n_stationid
        imc_msg.MSGHeader.n_msgtype = n_msgtype
        imc_msg.MSGHeader.n_extdatasize = len(s_extdata)

        list_headdata = [imc_msg.MSGHeader.n_stationtype, imc_msg.MSGHeader.n_stationid, imc_msg.MSGHeader.n_msgtype,
                         imc_msg.MSGHeader.n_subtype, imc_msg.MSGHeader.n_extdatasize,
                         imc_msg.MSGHeader.n_headerextdatasize,
                         imc_msg.MSGHeader.n_selfcheck, imc_msg.MSGHeader.n_checksum, imc_msg.MSGHeader.s_headerextdata]
        head = struct.pack(imc_msg.MSGHeader.dataheadformat, *list_headdata)
        if not isinstance(s_extdata, bytes):
            s_extdata = s_extdata.encode('gbk')  # str与bytes无法直接拼接
        self.list_queue_send[n_stationid].put((n_stationid, head + s_extdata))
