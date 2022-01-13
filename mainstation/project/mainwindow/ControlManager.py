
import time
import imc_msg
from project.other.globalparam import StaticConfigParam
from project.mainwindow.SerialManager import SerialManager
import logging
import numpy as np

def translatedis2hex(nmovedis):
    """
        将移动距离转换为hex ->  转换为两个十六进制
        例如0x27eab: VALUE1为0x7e,VALUE2为0xab,VALUE3为0x00,VALUE4为0x02,
    :param nmovedis:
    :return:
    """
    nmovedis = int(nmovedis)
    smalldata = nmovedis % 65536
    bigdata = int(nmovedis / 65536)

    VALUE1 = int(smalldata / 256)
    VALUE2 = smalldata % 256
    VALUE3 = int(bigdata / 256)
    VALUE4 = bigdata % 256

    return [VALUE1, VALUE2, VALUE3, VALUE4]

# def str_to_hex(data):
#     sdata = str(data)[2:-1]
#     list_data = sdata.split('\\x')
#     list_hex = []
#     for strdata in list_data:
#         if strdata != '':
#             list_hex.append(int(strdata, 16))
#     return list_hex

def str_to_hex(data):
    list_hex = []
    for i in range(0, len(data), 2):
        list_hex.append(int(data[i:i+2], 16))
    return list_hex


class ControlManager(object):
    """
    将控制逻辑放到这个类中，方便管理

    """
    _HARDWARE_QUEUELEN = 7
    _BSE_HEIGHT = -59
    def __init__(self, h_parent):
        self.h_parent = h_parent
        self.mySeria = None
        self._DIS2PULSE = StaticConfigParam.DIS2PULSE
        self.b_checkstatus = False#检测状态，检测状态为False的时候停止检测，很多循环都依赖这个状态，只要不在检测状态立刻返回
        self.logger = logging.getLogger('UI.%s' % self.__class__.__name__)
        self.b_zerostatus = False# 每次调用rebackzero内会会将这个值置为True，检测开始后又置为False，目的是减少时间花费


    def setserial(self, serial:SerialManager):
        self.mySeria = serial

    def buildmodel(self, *list_info):
        """
        :param list_info: [nStartX, ndisX, ndisY, nXtimes] [起拍X位置， 单次X移动距离， 单次Y移动距离， X次数]
        :return:
        """

        self.logger.log(logging.INFO, "开始取全图建模")

        self._clear_hardware_recqueue()

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_REC_START)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_JIAJIN)

        self._waitfor_hardware_queue_result([imc_msg.HARDWAREBASEMSG.MSG_LEFT_JIAJIN_DAOWEI, imc_msg.HARDWAREBASEMSG.MSG_RIGHT_JIAJIN_DAOWEI])

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_SHENG)

        self._waitfor_hardware_queue_result([imc_msg.HARDWAREBASEMSG.MSG_LEFT_DINGSHENG_DAOWEI, imc_msg.HARDWAREBASEMSG.MSG_RIGHT_DINGSHENG_DAOWEI])

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_DUIWEI_JIANG)

        nreadstatus = self._recmsgwithruntimeerror([imc_msg.HARDWAREBASEMSG.MSG_LEFT_DUIWEI_JIANG_DAOWEI, imc_msg.HARDWAREBASEMSG.MSG_RIGHT_DUIWEI_JIANG_DAOWEI], 3)

        if not nreadstatus:

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_CONTROL_ALARM)

            self.h_parent.callback2showerror("！！！定位气缸未到位！！！")

            return

        self._clear_hardware_recqueue()  # 清除接收缓存

        self.h_parent.closeCamera()

        self._rebackXY()

        self._rebackZERO()

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_OPEN_ALL_LIGHT)

        self._waitfor_hardware_queue_result()

        movemsgx = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_BASEMOVE

        if list_info[0] != 0:

            movemsgx[3:] = translatedis2hex(list_info[0] / self._DIS2PULSE)

            self._clear_hardware_recqueue()

            self._sendhardwaremsg(movemsgx)

            self._waitfor_hardware_queue_result()

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_X)

            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_ARRIVE)

        movemsgy = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_BASEMOVE

        ncycletimes = list_info[3]

        nrecordx = 0

        for i in range(ncycletimes):

            self.h_parent.openCamera()

            time.sleep(1)# 开环延时，给与相机打开时间准备

            movemsgy[3:] = translatedis2hex(list_info[2] / self._DIS2PULSE)

            self._sendhardwaremsg(movemsgy)

            self._waitfor_hardware_queue_result()

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)

            while self.h_parent.callback2judgeisfull() != True and self.b_checkstatus:

                time.sleep(0.5)

            self.h_parent.closeCamera()

            time.sleep(3)

            self._clear_hardware_recqueue()  # 清除接收缓存

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_REBACKMOTOR_Y)

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)

            if i != ncycletimes - 1:#最后一次不用向右移

                self.h_parent.callback2changecol()  # 告知参数界面开始切换下一个列拼图

                nrecordx += list_info[1]

                movemsgx[3:] = translatedis2hex(nrecordx / self._DIS2PULSE)

                self._sendhardwaremsg(movemsgx)

                self._waitfor_hardware_queue_result()

                self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_X)

                self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_ARRIVE)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_CLOSE_ALL_LIGHT)

        self.h_parent.callback2showbigimg()

        self.MakeEveryposFuwei()


    def buildmodel_second(self, *list_info):
        """
        二次建模控制
        :param list_info: [[list_x, list_y]]
        :return:
        """

        self.logger.log(logging.INFO, "开始二次取图建模")

        self._clear_hardware_recqueue()

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_REC_START)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_JIAJIN)

        self._waitfor_hardware_queue_result(
            [imc_msg.HARDWAREBASEMSG.MSG_LEFT_JIAJIN_DAOWEI, imc_msg.HARDWAREBASEMSG.MSG_RIGHT_JIAJIN_DAOWEI])

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_SHENG)

        self._waitfor_hardware_queue_result(
            [imc_msg.HARDWAREBASEMSG.MSG_LEFT_DINGSHENG_DAOWEI, imc_msg.HARDWAREBASEMSG.MSG_RIGHT_DINGSHENG_DAOWEI])

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_DUIWEI_JIANG)

        nreadstatus = self._recmsgwithruntimeerror([imc_msg.HARDWAREBASEMSG.MSG_LEFT_DUIWEI_JIANG_DAOWEI,
                                                    imc_msg.HARDWAREBASEMSG.MSG_RIGHT_DUIWEI_JIANG_DAOWEI], 3)

        if not nreadstatus:
            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_CONTROL_ALARM)

            self.h_parent.callback2showerror("！！！定位气缸未到位！！！")

            return

        self._clear_hardware_recqueue()# 清除接收缓存

        self.h_parent.closeCamera()

        self._rebackXY()

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_OPEN_ALL_LIGHT)

        self._waitfor_hardware_queue_result()

        movemsgx = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_BASEMOVE

        movemsgy = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_BASEMOVE

        # nlastmovex = 0


        for i in range(len(list_info[0])):

            x = list_info[0][i]

            y = list_info[1][i]

            # x_realmove = x - nlastmovex #让电机移动在上次的基础上移动
            #
            # nlastmovex = x

            movemsgx[3:] = translatedis2hex(x / self._DIS2PULSE)

            self._sendhardwaremsg(movemsgx)

            self._waitfor_hardware_queue_result()

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_X)

            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_ARRIVE)

            self.h_parent.openCamera()

            movemsgy[3:] = translatedis2hex(y / self._DIS2PULSE)

            self._sendhardwaremsg(movemsgy)

            self._waitfor_hardware_queue_result()

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)

            while not self.h_parent.callback2judgeisfull_second() and self.b_checkstatus:

                time.sleep(0.5)

            self.h_parent.closeCamera()

            time.sleep(1)

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_REBACKMOTOR_Y)

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)

            if i != len(list_info[0]) - 1:

                self.h_parent.callback2changecol_second()

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_CLOSE_ALL_LIGHT)

        self.h_parent.callback2showbigimg_second()

        self.MakeEveryposFuwei()


    def buildmodel_getsensorhigh(self, *list_info):
        """
        list_info 中放着N对 x, y 坐标，根据根据这几个坐标进行位置设置
        :param list_info:[[x, y], [x, y]]
        :return:
        """
        self._clear_hardware_recqueue()

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_REC_START)

        self._rebackXY()

        self._rebackZERO()

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_JIAJIN)

        self._waitfor_hardware_queue_result(
            [imc_msg.HARDWAREBASEMSG.MSG_LEFT_JIAJIN_DAOWEI, imc_msg.HARDWAREBASEMSG.MSG_RIGHT_JIAJIN_DAOWEI])

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_SHENG)

        self._waitfor_hardware_queue_result(
            [imc_msg.HARDWAREBASEMSG.MSG_LEFT_DINGSHENG_DAOWEI, imc_msg.HARDWAREBASEMSG.MSG_RIGHT_DINGSHENG_DAOWEI])

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_DUIWEI_JIANG)

        nreadstatus = self._recmsgwithruntimeerror([imc_msg.HARDWAREBASEMSG.MSG_LEFT_DUIWEI_JIANG_DAOWEI,
                                                    imc_msg.HARDWAREBASEMSG.MSG_RIGHT_DUIWEI_JIANG_DAOWEI], 3)

        if not nreadstatus:
            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_CONTROL_ALARM)

            self.h_parent.callback2showerror("！！！定位气缸未到位！！！")

            return

        list_result = []

        for pos in list_info:

            x = pos[0]

            y = pos[1]

            movemsgy = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_BASEMOVE

            movemsgy[3:] = translatedis2hex(y / self._DIS2PULSE)

            self._sendhardwaremsg(movemsgy)

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)

            movemsgx = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_BASEMOVE

            movemsgx[3:] = translatedis2hex(x / self._DIS2PULSE)

            self._sendhardwaremsg(movemsgx)

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_X)

            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_ARRIVE)

            list_result.append(self._GetZpos())

        self.h_parent.callback2set_highsensor_point(list_result)


        self.MakeEveryposFuwei()




    def _clear_hardware_recqueue(self):
        if self.mySeria is not None:
            self.mySeria.reset_input_buffer()


    def _waitfor_hardware_queue_result(self, info = None, ntimeout=None):
        """
        等待答复，如果确认是一直在等待某个值就一直等
        info        数据，可能是二维，二维表示全部接收到才算
        ntimeout    接收超时设置
        :return:    None
        """
        print('hope rec: ', info)
        data = []
        if info == None:
            data = self.mySeria.read().hex()
            print ('None rec: ', data)
            data = str_to_hex(data)
        else:
            if len(np.array(info).shape) == 1:
                while self.b_checkstatus:
                    data = self.mySeria.read(ntimeout).hex()
                    print('read ori data: ', data)
                    data = str_to_hex(data)
                    print ('read data: ', data)
                    if data != info:
                        continue
                    else:
                        break
            else:
                list_status = [False for i in range(len(info))]
                while np.sum(list_status) != len(list_status) and self.b_checkstatus:
                    data = self.mySeria.read(ntimeout).hex()
                    print('read ori data: ', data)
                    data = str_to_hex(data)
                    print('read data: ', data)

                    for nindex in range(len(info)):

                        if info[nindex] == data:

                            list_status[nindex] = True

                            break


        #time.sleep(0.5)
        return data


    def _rebackXY(self):

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_REBACKMOTOR_X)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_X)

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_ARRIVE)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_REBACKMOTOR_Y)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)


    def _rebackZERO(self):
        """
        返回模组的原点，这是因为设备有可能运动过程中丢失原点
        :return:
        """
        if not self.b_zerostatus:
            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_ZERO)
            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_ARRIVE)
            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ZERO)
            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)
            self.b_zerostatus = True



    def _sendhardwaremsg(self, info):
        print('send, ', info)
        self.mySeria.write(info)


    def setcheckstatus(self, bstatus):
        self.b_checkstatus = bstatus

        if self.b_checkstatus == False:

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_END_CHECK)

    def waitforstart(self, bisfirst):
        """
            等待双启动按下
        """
        self._clear_hardware_recqueue()
        if bisfirst:
            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_REC_START)

            if self.b_checkstatus:

                self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_START_CHECK)


    def check_control(self, list_info):
        """
        检测控制，跟二次建模一样的轨迹，不一样的是加了过程判断以及取图反转
        :param list_info: [[list_x, list_y]]
        :return:
        """
        print ('--------------- 开始检测控制 -------------------')

        self._clear_hardware_recqueue()


        self.h_parent.closeCamera()

        self._rebackXY()

        if not self.b_checkstatus: return

        self._rebackZERO()

        self.b_zerostatus = False

        #self.MakeEveryposFuwei()

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_JIAJIN)

        self._waitfor_hardware_queue_result(
            [imc_msg.HARDWAREBASEMSG.MSG_LEFT_JIAJIN_DAOWEI, imc_msg.HARDWAREBASEMSG.MSG_RIGHT_JIAJIN_DAOWEI])

        self._control2markpos()

        self._clear_hardware_recqueue()

        # # 判断托盘是否到位
        # #
        # nstatus = self._judgeZisRight()
        #
        # if nstatus == 0:
        #
        #     self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_SONGKAI)
        #
        #     self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_CONTROL_ALARM)
        #
        #     self.h_parent.callback2showerror("！！！！托盘未到位，无法检测！！！！")
        #
        #     return

        self._rebackXY()

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_SHENG)

        self._waitfor_hardware_queue_result([imc_msg.HARDWAREBASEMSG.MSG_LEFT_DINGSHENG_DAOWEI, imc_msg.HARDWAREBASEMSG.MSG_RIGHT_DINGSHENG_DAOWEI])

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_DUIWEI_JIANG)

        nreadstatus = self._recmsgwithruntimeerror([imc_msg.HARDWAREBASEMSG.MSG_LEFT_DUIWEI_JIANG_DAOWEI, imc_msg.HARDWAREBASEMSG.MSG_RIGHT_DUIWEI_JIANG_DAOWEI], 3)

        if not nreadstatus:

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_CONTROL_ALARM)

            self.h_parent.callback2showerror("！！！定位气缸未到位！！！")

            return

        self._clear_hardware_recqueue()  # 清除接收缓存

        if not self.b_checkstatus:  return

        movemsgx = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_BASEMOVE

        movemsgy = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_BASEMOVE

        for i in range(len(list_info[0])):

            x = list_info[0][i]

            y = list_info[1][i]

            movemsgx[3:] = translatedis2hex(x / self._DIS2PULSE)

            self._sendhardwaremsg(movemsgx)

            self._waitfor_hardware_queue_result()

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_X)

            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_ARRIVE)

            if i != 0:

                self.h_parent.sendmsg_changecol()# 通知子站图像换列，做好标记

            if not self.b_checkstatus:  return

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_OPEN_LIGHT1)

            self._waitfor_hardware_queue_result()

            self.h_parent.onlyopencamera()

            time.sleep(1)

            movemsgy[3:] = translatedis2hex(y / self._DIS2PULSE)

            self._sendhardwaremsg(movemsgy)

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)

            if not self.b_checkstatus:  return

            time.sleep(1)

            self.h_parent.closeCamera()

            self.h_parent.changeCameraCapturedirection()

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_OPEN_LIGHT2)

            self._waitfor_hardware_queue_result()

            self.h_parent.onlyopencamera()

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_REBACKMOTOR_Y)

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)

            if not self.b_checkstatus:  return

            time.sleep(1)

            self.h_parent.closeCamera()

            self.h_parent.changeCameraCapturedirection(True)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_CLOSE_ALL_LIGHT)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_DUIWEI_SHENG)

        self._waitfor_hardware_queue_result([imc_msg.HARDWAREBASEMSG.MSG_LEFT_DUIWEI_SHENG_DAOWEI, imc_msg.HARDWAREBASEMSG.MSG_RIGHT_DUIWEI_SHENG_DAOWEI])

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_JIANG)

        self._waitfor_hardware_queue_result([imc_msg.HARDWAREBASEMSG.MSG_LEFT_DINGSHENG_XIAJIANG_DAOWEI, imc_msg.HARDWAREBASEMSG.MSG_RIGHT_DINGSHENG_XIAJIANG_DAOWEI])

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_SONGKAI)

        self._waitfor_hardware_queue_result([imc_msg.HARDWAREBASEMSG.MSG_LEFT_JIAJIN_SONGKAI_DAOWEI, imc_msg.HARDWAREBASEMSG.MSG_RIGHT_JIAJIN_SONGKAI_DAOWEI])

        self._rebackXY()

        self._rebackZERO()


    def control_calibrate(self, *list_data):
        """
        :param list_data: [x, y1, y2] [起拍X位置， 起拍Y位置， 拍摄结束位置Y]
        :return:
        """

        self._clear_hardware_recqueue()

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_REC_START)

        # 1. 复位
        self._rebackXY()

        self._rebackZERO()

        # 2. z轴抬升100mm #溧阳先注释
        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_REBACKMOTOR_Z)

        time.sleep(2)

        diff_pulse = int(80 / StaticConfigParam.RATE)

        basemovez = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Z_BASEMOVE

        high = int(diff_pulse / 256)

        low = int(diff_pulse % 256)

        basemovez[3] = high

        basemovez[4] = low

        self._sendhardwaremsg(basemovez)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Z_MOVE)

        time.sleep(2)

        self._clear_hardware_recqueue()
        # 3. 移动到标定块位置
        movemsgx = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_BASEMOVE

        movemsgx[3:] = translatedis2hex(list_data[0] / self._DIS2PULSE)

        self._sendhardwaremsg(movemsgx)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_X)

        time.sleep(5)

        #self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_ARRIVE)

        movemsgy = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_BASEMOVE

        movemsgy[3:] = translatedis2hex(list_data[1] / self._DIS2PULSE)

        self._sendhardwaremsg(movemsgy)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)

        # 3.
        self.h_parent.callback2dotcheck()

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_OPEN_ALL_LIGHT)

        self._waitfor_hardware_queue_result()

        movemsgy[3:] = translatedis2hex(list_data[2] / self._DIS2PULSE)

        self._sendhardwaremsg(movemsgy)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)

        time.sleep(1)

        self.h_parent.callback2stopdotcheck()

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_CLOSE_ALL_LIGHT)


    def _judgeZisRight(self):
        """判断Z轴是否到位"""
        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_GET_Z_POS)

        while self.b_checkstatus:

            list_recdata = self._waitfor_hardware_queue_result()

            if list_recdata[:2] == imc_msg.HARDWAREBASEMSG.MSG_Z_RESPOND_HEAD:

                nreadz = (int(list_recdata[3]) * 256 + int(list_recdata[4])) * 0.112 - 198.2

                print ("读到数值: ", nreadz)

                if abs(nreadz - StaticConfigParam.BASE_Z) > 10:# 偏移超过10则处理

                    return 0

                else:

                    return 1


    def _GetZpos(self):
        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_GET_Z_POS)

        while self.b_checkstatus:

            list_recdata = self._waitfor_hardware_queue_result()

            if list_recdata[:2] == imc_msg.HARDWAREBASEMSG.MSG_Z_RESPOND_HEAD:

                nreadz = (int(list_recdata[3]) * 256 + int(list_recdata[4])) * 0.112 - 198.2

                return nreadz


    def control_adjust_z(self):

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_REBACKMOTOR_Z)

        time.sleep(3)

        self._clear_hardware_recqueue()

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_GET_Z_POS)

        self._waitfor_hardware_queue_result()

        list_recdata = self._waitfor_hardware_queue_result()

        nreadz =  (int(list_recdata[3]) * 256 + int(list_recdata[4])) * 0.112 - 198.2

        print ("du dao shuzhi :", nreadz)
        #
        # diff_pulse = int((nreadz - _BASE_Z) / _RATE)
        #
        # basemovez = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Z_BASEMOVE
        #
        # high = int(diff_pulse / 256)
        #
        # low = int(diff_pulse % 256)
        #
        # basemovez[3] = high
        #
        # basemovez[4] = low
        #
        # self._sendhardwaremsg(basemovez)
        #
        # self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Z_MOVE)




    def MakeEveryposFuwei(self):
        """
        让光源，夹紧，顶升，对位气缸复位，原因是为了初始化最初状态
        :return:
        """
        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_CLOSE_ALL_LIGHT)
        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_DUIWEI_SHENG)
        self._waitfor_hardware_queue_result([imc_msg.HARDWAREBASEMSG.MSG_LEFT_DUIWEI_SHENG_DAOWEI, imc_msg.HARDWAREBASEMSG.MSG_RIGHT_DUIWEI_SHENG_DAOWEI])
        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_JIANG)
        self._waitfor_hardware_queue_result([imc_msg.HARDWAREBASEMSG.MSG_LEFT_DINGSHENG_XIAJIANG_DAOWEI, imc_msg.HARDWAREBASEMSG.MSG_RIGHT_DINGSHENG_XIAJIANG_DAOWEI])
        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_SONGKAI)
        self._waitfor_hardware_queue_result([imc_msg.HARDWAREBASEMSG.MSG_LEFT_JIAJIN_SONGKAI_DAOWEI, imc_msg.HARDWAREBASEMSG.MSG_RIGHT_JIAJIN_SONGKAI_DAOWEI])
        self._clear_hardware_recqueue()


    def _control2markpos(self):
        movemsgx = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_BASEMOVE

        movemsgy = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_BASEMOVE

        movemsgx[3:] = translatedis2hex(300 / self._DIS2PULSE)

        self._sendhardwaremsg(movemsgx)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_X)

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_ARRIVE)

        movemsgy[3:] = translatedis2hex(50 / self._DIS2PULSE)

        self._sendhardwaremsg(movemsgy)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)


    def _recmsgwithruntimeerror(self, info, ntimeout):
        """
        测试接收数据时间，如果超过
        超时时间，以1s为单位
        """
        print('hope rec: ', info)
        curtime = time.time()

        b_rectargetstatus = False

        if len(np.array(info).shape) == 1:
            while (time.time() - curtime < ntimeout):
                data = self.mySeria.read(1).hex()
                data = str_to_hex(data)
                if data != info:
                    continue
                else:
                    b_rectargetstatus = True
                    break
        else:
            list_status = [False for i in range(len(info))]

            while (np.sum(list_status) != len(list_status))  and (int(time.time() - curtime) <= ntimeout):

                data = self.mySeria.read(1).hex()
                data = str_to_hex(data)

                for nindex in range(len(info)):
                    if info[nindex] == data:
                        list_status[nindex] = True
                        break
            b_rectargetstatus = (np.sum(list_status) == len(list_status))

        return b_rectargetstatus



    def control_guangshan(self, nstatus):
        if nstatus:
            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_TURNON_GUANGSHAN)
            print ("----------------- 打开光栅 -----------------------")
        else:
            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_TURNOFF_GUANGSHAN)
