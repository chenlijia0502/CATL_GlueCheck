
import time
import imc_msg
from project.other.globalparam import StaticConfigParam
from project.mainwindow.SerialManager import SerialManager
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

    def setserial(self, serial:SerialManager):
        self.mySeria = serial

    def buildmodel(self, *list_info):
        """
        :param list_info: [nStartX, ndisX, ndisY, nXtimes] [起拍X位置， 单次X移动距离， 单次Y移动距离， X次数]
        :return:
        """
        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_JIAJIN)

        time.sleep(5)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_SHENG)

        time.sleep(8)

        self._clear_hardware_recqueue()  # 清除接收缓存

        self._clear_hardware_recqueue()# 清除接收缓存

        self.h_parent.closeCamera()

        self._rebackXY()

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

            print ("---------------------------- 准备启动y轴电机 --------------------------------------------")

            self.h_parent.openCamera()

            time.sleep(1)# 开环延时，给与相机打开时间准备

            movemsgy[3:] = translatedis2hex(list_info[2] / self._DIS2PULSE)

            self._sendhardwaremsg(movemsgy)

            self._waitfor_hardware_queue_result()

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)

            while self.h_parent.callback2judgeisfull() != True:

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

                print ("fa song bian shang yidong juli: ", movemsgx)

                self._sendhardwaremsg(movemsgx)

                self._waitfor_hardware_queue_result()

                self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_X)

                self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_ARRIVE)

        # while self.h_parent.callback2judgeisfull() != True:
        #
        #     time.sleep(0.5)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_CLOSE_ALL_LIGHT)

        self.h_parent.callback2showbigimg()

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_JIANG)

        time.sleep(8)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_SONGKAI)

        time.sleep(5)



    def buildmodel_second(self, *list_info):
        """
        二次建模控制
        :param list_info: [[list_x, list_y]]
        :return:
        """
        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_JIAJIN)

        time.sleep(5)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_SHENG)

        time.sleep(8)

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

            while not self.h_parent.callback2judgeisfull_second():

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

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_JIANG)

        time.sleep(8)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_SONGKAI)

        time.sleep(5)


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
        if info == None:
            data = self.mySeria.read().hex()
            print ('None rec: ', data)
            data = str_to_hex(data)
        else:
            if len(np.array(info).shape) == 1:
                while(1):
                    data = self.mySeria.read().hex()
                    print('read ori data: ', data)
                    data = str_to_hex(data)
                    print ('read data: ', data)
                    if data != info:
                        continue
                    else:
                        break
            else:
                list_status = [False for i in range(len(info))]
                while np.sum(list_status) != len(list_status):
                    data = self.mySeria.read().hex()
                    print('read ori data: ', data)
                    data = str_to_hex(data)
                    print('read data: ', data)

                    for nindex in range(len(info)):

                        if info[nindex] == data:

                            list_status[nindex] = True

                            break


        time.sleep(0.5)
        return data



    def _rebackXY(self):

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_REBACKMOTOR_X)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_X)

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_X_XIANWEI)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_REBACKMOTOR_Y)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_Y_XIANWEI)




    def _sendhardwaremsg(self, info):
        print('send, ', info)
        self.mySeria.write(info)
        time.sleep(0.5)


    def test_saveimg(self):
        # 复位Y，

        self._clear_hardware_recqueue()  # 清除接收缓存

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_REBACKMOTOR_Y)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)

        self.h_parent.openCamera()

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_OPEN_LIGHT1)

        self._waitfor_hardware_queue_result()

        movemsgy = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_BASEMOVE

        y = 2000

        movemsgy[3:] = translatedis2hex(y / self._DIS2PULSE)

        self._sendhardwaremsg(movemsgy)

        self._waitfor_hardware_queue_result()

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)

        time.sleep(5)

        self.h_parent.closeCamera()

        time.sleep(1)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_OPEN_LIGHT2)

        self._waitfor_hardware_queue_result()

        self.h_parent.changeCameraCapturedirection()

        time.sleep(2)

        self.h_parent.openCamera()

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_REBACKMOTOR_Y)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)

        time.sleep(5)

        print('----------done-----------')


    def setcheckstatus(self, bstatus):
        self.b_checkstatus = bstatus


    def check_control(self, list_info):
        """
        检测控制，跟二次建模一样的轨迹，不一样的是加了过程判断以及取图反转
        :param list_info: [[list_x, list_y]]
        :return:
        """
        self._MakeEveryposFuwei()

        self._clear_hardware_recqueue()

        while 1:
            list_data = self._waitfor_hardware_queue_result()

            if list_data == imc_msg.HARDWAREBASEMSG.MSG_REC_START or list_data == imc_msg.HARDWAREBASEMSG.MSG_REC_START1:

                break

        self.h_parent.closeCamera()

        self._clear_hardware_recqueue()  # 清除接收缓存

        self._rebackXY()

        # 判断托盘是否到位

        nstatus = self._judgeZisRight()

        if nstatus == 0:

            self.h_parent.callback2showerror("！！！！托盘未到位，无法检测！！！！")

            return

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_CLOSE_ALL_LIGHT)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_JIAJIN)

        self._waitfor_hardware_queue_result([imc_msg.HARDWAREBASEMSG.MSG_LEFT_JIAJIN_DAOWEI, imc_msg.HARDWAREBASEMSG.MSG_RIGHT_JIAJIN_DAOWEI])

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_SHENG)

        self._waitfor_hardware_queue_result([imc_msg.HARDWAREBASEMSG.MSG_LEFT_DINGSHENG_DAOWEI, imc_msg.HARDWAREBASEMSG.MSG_RIGHT_DINGSHENG_DAOWEI])

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_DUIWEI_JIANG)

        self._waitfor_hardware_queue_result([imc_msg.HARDWAREBASEMSG.MSG_LEFT_DUIWEI_DAOWEI, imc_msg.HARDWAREBASEMSG.MSG_RIGHT_RIGHT_DAOWEI])

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

        self._waitfor_hardware_queue_result([imc_msg.HARDWAREBASEMSG.MSG_LEFT_DUIWEI_SONGKAI_DAOWEI, imc_msg.HARDWAREBASEMSG.MSG_RIGHT_DUIWEI_SONGKAI_DAOWEI])

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_JIANG)

        self._waitfor_hardware_queue_result([imc_msg.HARDWAREBASEMSG.MSG_LEFT_DINGSHENG_XIAJIANG_DAOWEI, imc_msg.HARDWAREBASEMSG.MSG_RIGHT_DINGSHENG_XIAJIANG_DAOWEI])

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_SONGKAI)

        self._waitfor_hardware_queue_result([imc_msg.HARDWAREBASEMSG.MSG_LEFT_JIAJIN_SONGKAI_DAOWEI, imc_msg.HARDWAREBASEMSG.MSG_RIGHT_JIAJIN_SONGKAI_DAOWEI])



    def control_calibrate(self):

        self._clear_hardware_recqueue()

        while 1:
            list_data = self._waitfor_hardware_queue_result()

            if list_data == imc_msg.HARDWAREBASEMSG.MSG_REC_START or list_data == imc_msg.HARDWAREBASEMSG.MSG_REC_START1:
                break

        # 1. 复位
        self._rebackXY()

        # 2. z轴抬升100mm
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

        movemsgx[3:] = translatedis2hex(900 / self._DIS2PULSE)

        self._sendhardwaremsg(movemsgx)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_X)

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_ARRIVE)

        movemsgy = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_BASEMOVE

        movemsgy[3:] = translatedis2hex(200 / self._DIS2PULSE)

        self._sendhardwaremsg(movemsgy)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)

        # 3.
        self.h_parent.callback2dotcheck()

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_OPEN_ALL_LIGHT)

        self._waitfor_hardware_queue_result()

        movemsgy[3:] = translatedis2hex(450 / self._DIS2PULSE)

        self._sendhardwaremsg(movemsgy)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)

        time.sleep(1)

        self.h_parent.callback2stopdotcheck()

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_CLOSE_ALL_LIGHT)


    def _judgeZisRight(self):
        """判断Z轴是否到位"""
        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_GET_Z_POS)

        while 1:

            list_recdata = self._waitfor_hardware_queue_result()

            if list_recdata[:2] == imc_msg.HARDWAREBASEMSG.MSG_Z_RESPOND_HEAD:

                nreadz = (int(list_recdata[3]) * 256 + int(list_recdata[4])) * 0.112 - 198.2

                print ("读到数值: ", nreadz)

                if abs(nreadz - StaticConfigParam.BASE_Z) > 10:# 偏移超过10则处理

                    return 0

                else:

                    return 1



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




    def _MakeEveryposFuwei(self):
        """
        让夹紧，顶升，对位气缸复位，原因是为了初始化最初状态
        :return:
        """
        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_CLOSE_ALL_LIGHT)
        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_DUIWEI_SHENG)
        time.sleep(1)
        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_JIANG)
        time.sleep(1)
        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_SONGKAI)
        time.sleep(5)
        self._clear_hardware_recqueue()
