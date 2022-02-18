import threading
import socket
import time
import  logging
from project.mainwindow.ControlManager import ControlManager
from library.common.globalparam import LogInfo
import imc_msg
from library.ipc import ipc_tool


class CheckControlThread(threading.Thread):
    _MAX_AGV_BUFFERLEN = 100  # 赌小车答复不会一次性答复这么长
    def __init__(self, hparent, ip, port, nid1, nid2, nid3):
        super(CheckControlThread, self).__init__()
        self.h_parent = hparent
        self.b_runstaus = False
        self.b_isfirstrun = True# 是否为第一次触发，第一次触发需要人为按下开始按钮
        self.list_info = []
        self.controlmanger = None
        self.b_emit = False
        self.b_allselected = False
        self.b_rootmode = False#设置调试模式，调试模式不需要与小车交互
        try:
            self.tcp_agvclient = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.tcp_agvclient.connect((ip, int(port)))
            self.tcp_agvclient.settimeout(2)
        except Exception as e:
            ipc_tool.kxlog("CheckControlThread", logging.ERROR, "AGV 启动失败")
        self.nid1 = int(nid1)
        self.nid2 = int(nid2)
        self.nid3 = int(nid3)
        self.s_packid = str(int(time.time()))
        self.b_nextstatus = False #进入下一个站点
        self.__initlog()


    def setrootmode(self, status):
        """
        设置调试模式
        """
        self.b_rootmode = status

    def __initlog(self):
        self.s_time = time.strftime("%Y-%m-%d")
        self.logger = logging.getLogger(__name__)
        self.logger.setLevel(level=logging.INFO)
        self.handler = logging.FileHandler(LogInfo.PATH_SAVE_LOG + 'AGV_%s.log' %  self.s_time)
        self.handler.setLevel(logging.INFO)
        self.formatter = logging.Formatter('%(levelname)s %(asctime)s %(message)s')
        self.handler.setFormatter(self.formatter)
        self.logger.addHandler(self.handler)



    def setstatus(self, bstatus):
        """
        设置运行状态，开始检测为True，停止检测为False
        :param bstatus:
        :return:
        """
        self.b_runstaus = bstatus
        if bstatus:
            self.b_isfirstrun = True

            self.b_nextstatus = False


    def setinfo(self, list_info):
        """
        设置运动路径
        :param  list_info:  [list_x, list_y]
        :return:
        """
        self.list_info = list_info


    def setControlmanger(self, serials:ControlManager):

        self.controlmanger = serials


    def emits(self):
        self.b_emit = True

    def emitnext(self):
        self.b_nextstatus = True


    def str_to_hex(self, data):
        list_hex = []
        for i in range(0, len(data), 2):
            list_hex.append(int(data[i:i + 2], 16))
        return list_hex


    def list_hex_to_str(self, list_data):
        sword = ''

        for data in list_data:

            sword += chr(data)

        return sword


    def _get_status_and_packid(self, nagvstationid):
        """
        获取站点状态以及pack号
        :param nagvstationid: 站点号
        :return: 1为当前站点有车，0为没有
        """
        try:

            self.logger.log(logging.INFO, "获取 %d 站点状态以及pack号"%nagvstationid)

            MSG_AGV_GETINFO = imc_msg.AGVMSG.MSG_BASE_GET_STATION_STATUS

            MSG_AGV_GETINFO[5] = nagvstationid

            self.tcp_agvclient.send(bytes(MSG_AGV_GETINFO))

            self.logger.log(logging.INFO, "send: " + str(MSG_AGV_GETINFO))

            sreaddata = self.tcp_agvclient.recv(self._MAX_AGV_BUFFERLEN).hex()

            self.logger.log(logging.INFO, "recv: " + str(sreaddata))

            list_readdata = self.str_to_hex(sreaddata)

            if len(list_readdata) > 8 and list_readdata[8] == imc_msg.AGVMSG.AGVSTATUS_ONSTATION:

                nagvid = list_readdata[7]

                self.logger.log(logging.INFO, "小车 ID: %d"%nagvid)

                MSG_GET_PACKID = imc_msg.AGVMSG.MSG_BASE_GET_PACK_ID

                MSG_GET_PACKID[5] = nagvid

                while self.b_runstaus:

                    self.tcp_agvclient.send(bytes(MSG_GET_PACKID))

                    self.logger.log(logging.INFO, "send: " + str(MSG_GET_PACKID))

                    sreaddata = self.tcp_agvclient.recv(self._MAX_AGV_BUFFERLEN).hex()

                    self.logger.log(logging.INFO, "recv: " + sreaddata)

                    list_readdata = self.str_to_hex(sreaddata)

                    if len(list_readdata) > 5 and list_readdata[4] == 0xB1:

                        nidlen = list_readdata[3]

                        list_packid = list_readdata[6:6 + nidlen - 2]

                        self.s_packid = self.list_hex_to_str(list_packid)

                        self.logger.log(logging.INFO, "小车 ID:  " + self.s_packid)

                        return 1

                    time.sleep(1)

            else:

                self.logger.log(logging.ERROR, "获取小车站点号出现失败的情况")

                return 0

        except Exception as e:

            self.logger.log(logging.ERROR, "AGV错误  _get_status_and_packid %s"%str(e))

            self.h_parent.callback2showerror("AGV 交互错误，请查收AGV日志查看具体原因")

            return 0
                #print('等待AGV错误', e)


    def waitforagv(self):
        """
        等待agv小车的到来
        :return:
        """
        #1. 清楚接收缓存
        try:
            try:
                self.tcp_agvclient.recv(1000)
            except Exception as e:
                pass#清除缓存

            #2. 获取中间站点小车状态

            self.logger.log(logging.INFO, "------- waitforagv -----------")

            self.logger.log(logging.INFO, "判断设备内部工位是否有小车")

            if self._get_status_and_packid(self.nid2):

                self.logger.log(logging.INFO, "设备内部存在小车，直接开始检测")

                return 1# 站点2内有车直接返回，并开始

            else:
                self.logger.log(logging.INFO, "设备内部并无小车，等待进站工位是否有小车")

                # 3. 获取进站口站点状态
                while not self._get_status_and_packid(self.nid1) and self.b_runstaus:

                    time.sleep(2)

                if not self.b_runstaus: return 1

                self.logger.log(logging.INFO, "得到小车信息， 关闭光栅")
                # 4. 关闭光栅放入小车
                self.controlmanger.control_guangshan(1)

                MSG_CONTROL_AGV = imc_msg.AGVMSG.MSG_BASE_CONTROL_STATION_STATUS

                MSG_CONTROL_AGV[5] = self.nid1

                self.tcp_agvclient.send(bytes(MSG_CONTROL_AGV))

                self.logger.log(logging.INFO, "send: " + str(MSG_CONTROL_AGV))

                self.logger.log(logging.INFO, "等待小车进入中间工位 %d"%self.nid2)
                # 5. 检测小车到位中间站点
                while self.b_runstaus:

                    MSG_AGV_GETINFO = imc_msg.AGVMSG.MSG_BASE_GET_STATION_STATUS

                    MSG_AGV_GETINFO[5] = self.nid2

                    self.tcp_agvclient.send(bytes(MSG_AGV_GETINFO))

                    self.logger.log(logging.INFO, "send: " + str(MSG_AGV_GETINFO))

                    sreaddata = self.tcp_agvclient.recv(self._MAX_AGV_BUFFERLEN).hex()

                    self.logger.log(logging.INFO, "rec: " + sreaddata)

                    list_readdata = self.str_to_hex(sreaddata)

                    if len(list_readdata) > 8 and list_readdata[8] == imc_msg.AGVMSG.AGVSTATUS_ONSTATION:

                        break

                    time.sleep(1)

                # 6. 打开光栅
                self.controlmanger.control_guangshan(0)

                return 1
        except Exception as e:

            self.logger.log(logging.ERROR, 'AGV错误 waitforagv %s'%str(e))

            self.h_parent.callback2showerror("AGV 交互错误，请查收AGV日志查看具体原因")
            return 0



    def sendagv2next(self):
        try:
            self.logger.log(logging.INFO, "sendagv2next， 等待结果，准备将小车送出站")

            #2022.1.21 选择直接将车放出
            while (not self.b_nextstatus) and self.b_runstaus:#等待外部将b_nextstatus置为True

                time.sleep(1)

            self.b_nextstatus = False

            if not self.b_runstaus: return

            self.logger.log(logging.INFO, "检测结果已出，等待站点 %d 没有小车状态"%self.nid3)

            while self.b_runstaus:

                MSG_AGV_GETINFO = imc_msg.AGVMSG.MSG_BASE_GET_STATION_STATUS

                MSG_AGV_GETINFO[5] = self.nid3

                self.tcp_agvclient.send(bytes(MSG_AGV_GETINFO))

                self.logger.log(logging.INFO, "send: " + str(MSG_AGV_GETINFO))

                sreaddata = self.tcp_agvclient.recv(self._MAX_AGV_BUFFERLEN).hex()

                self.logger.log(logging.INFO, "recv: " + sreaddata)

                list_readdata = self.str_to_hex(sreaddata)

                if len(list_readdata) > 8 and (list_readdata[8] == imc_msg.AGVMSG.AGVSTATUS_FREE or
                                               list_readdata[8] == imc_msg.AGVMSG.AGVSTATUS_NONE):

                    break

                time.sleep(5)

            self.logger.log(logging.INFO, "出站口没有小车，关闭出站光栅，将小车送出")

            #关闭光栅
            self.controlmanger.control_guangshan(2)

            #送出小车
            self.logger.log(logging.INFO, "将小车送出 %d 号站点"%self.nid2)

            MSG_CONTROL_AGV = imc_msg.AGVMSG.MSG_BASE_CONTROL_STATION_STATUS

            MSG_CONTROL_AGV[5] = self.nid2

            self.tcp_agvclient.send(bytes(MSG_CONTROL_AGV))

            self.logger.log(logging.INFO, "send: " + str(MSG_CONTROL_AGV))

            #监听送出小车状态
            self.logger.log(logging.INFO, "监听小车是否到达 %d 号站点"%self.nid3)

            while self.b_runstaus:

                MSG_AGV_GETINFO = imc_msg.AGVMSG.MSG_BASE_GET_STATION_STATUS

                MSG_AGV_GETINFO[5] = self.nid3

                self.tcp_agvclient.send(bytes(MSG_AGV_GETINFO))

                self.logger.log(logging.INFO, "send: " + str(MSG_AGV_GETINFO))

                sreaddata = self.tcp_agvclient.recv(self._MAX_AGV_BUFFERLEN).hex()

                self.logger.log(logging.INFO, "recv: " + sreaddata)

                list_readdata = self.str_to_hex(sreaddata)

                if len(list_readdata) > 8 and (list_readdata[8] == imc_msg.AGVMSG.AGVSTATUS_ONSTATION):

                    break

                time.sleep(5)

            self.logger.log(logging.INFO, "------------ 小车已到达 %d 号站点"%self.nid3 + ", 结束本次循环 -------------")


            #开启光栅
            self.controlmanger.control_guangshan(0)

        except Exception as e:

            self.logger.log(logging.ERROR, 'AGV错误 sendagv2next %s'%str(e))

            self.h_parent.callback2showerror("AGV 交互错误，请查收AGV日志查看具体原因")

            return 0


    def emitselected(self):
        self.b_allselected = True


    def waitfor_checkmaskallselect(self):
        """
        等待确认是否所有检测区域都勾选
        """
        self.logger.log(logging.INFO, "waitfor_checkmaskallselect， 进入等待检测是否有屏蔽框状态")

        self.b_allselected = False

        self.h_parent.callback2ensure_all_checkarea_selected()

        while (not self.b_allselected) and self.b_runstaus:

            time.sleep(1)


    def run(self):
        """
        执行检测需要的动作
        :return:
        """
        while (1):

            if self.b_runstaus and self.b_emit:
            #if self.b_runstaus:
                self.b_emit = False

                ipc_tool.kxlog("checkcontrolthread", logging.INFO, "第一步，等待按下开始按钮，只对开始检测后第一次有用")
                self.controlmanger.waitforstart(self.b_isfirstrun)

                self.b_isfirstrun = False

                if not self.b_runstaus: continue

                ipc_tool.kxlog("checkcontrolthread", logging.INFO, "第二步，确认主界面检测区域是否全部开启，如未全部开启的话需要弹窗并等待确认")
                self.waitfor_checkmaskallselect()

                ipc_tool.kxlog("checkcontrolthread", logging.INFO, "第三步，初始化所有气缸状态")
                self.controlmanger.MakeEveryposFuwei()

                if not self.b_runstaus: continue

                ipc_tool.kxlog("checkcontrolthread", logging.INFO, "第四步，监听设备小车，先监听工位内，再监听进站口")
                if not self.b_rootmode:

                    self.waitforagv()

                if not self.b_runstaus: continue

                ipc_tool.kxlog("checkcontrolthread", logging.INFO, "第五步，成功获取packid，并发送到主界面 %s"%self.s_packid)
                self.h_parent._SIG_PACKID.emit(self.s_packid)

                #控制主流程
                ipc_tool.kxlog("checkcontrolthread", logging.INFO, "第六步，进入下位机控制流程")
                if not self.controlmanger.check_control(self.list_info): continue

                if not self.b_runstaus: continue

                #小车出站
                ipc_tool.kxlog("checkcontrolthread", logging.INFO, "第六步，结束下位机控制，小车准备出站")
                if not self.b_rootmode:

                    self.sendagv2next()

                ipc_tool.kxlog("checkcontrolthread", logging.INFO, "第七步，小车已出，逻辑结束！")

            else:

                time.sleep(0.5)