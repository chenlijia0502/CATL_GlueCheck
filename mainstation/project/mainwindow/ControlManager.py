
import time
import imc_msg
from project.other.globalparam import StaticConfigParam
from project.mainwindow.SerialManager import SerialManager

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


    def _waitfor_hardware_queue_result(self, info = None):
        """
        等待答复，如果确认是一直在等待某个值就一直等
        :return:
        """
        print("HOPE REC: ", info)

        if info == None:
            data = self.mySeria.read().hex()
            print ('None rec: ', data)
        else:
            while(1):
                data = self.mySeria.read().hex()
                print('read ori data: ', data)
                data = str_to_hex(data)
                print ('read data: ', data)
                if data != info:
                    continue
                else:
                    break
        time.sleep(0.2)
        return
        # return self.str_to_hex(data)



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
        self._clear_hardware_recqueue()  # 清除接收缓存

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_CLOSE_ALL_LIGHT)

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_REC_START) #接收到下位机按下开始触发指令

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_JIAJIN)

        time.sleep(5)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_SHENG)

        time.sleep(8)

        self._clear_hardware_recqueue()  # 清除接收缓存

        self.h_parent.closeCamera()

        self._rebackXY()

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

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_JIANG)

        time.sleep(8)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_SONGKAI)

        time.sleep(5)


    def _control_calibrate(self):

        # 1. 复位
        self._rebackXY()

        # 2. 移动到标定块位置
        movemsgx = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_BASEMOVE

        movemsgx[3:] = translatedis2hex(900 / self._DIS2PULSE)

        self._sendhardwaremsg(movemsgx)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_X)

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_ARRIVE)

        movemsgy = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_BASEMOVE

        movemsgy[3:] = translatedis2hex(100 / self._DIS2PULSE)

        self._sendhardwaremsg(movemsgy)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)

        # 3.

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_OPEN_ALL_LIGHT)

        self._waitfor_hardware_queue_result()

        movemsgy[3:] = translatedis2hex(600 / self._DIS2PULSE)

        self._sendhardwaremsg(movemsgy)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_CLOSE_ALL_LIGHT)


    #
    # def _controlmachine(self):
    #
    #     # 发送启动命令 #
    #
    #     # 循环等待控制完成
    #
    #     # while(1):
    #     #     data = self.mySeria.read(7)
    #     #     list_data = self.str_to_hex(data)
    #     #     if list_data == imc_msg.HARDWAREBASEMSG.MSG_DUIWEI_QIGANG_DONE:# 对位气缸对位成功
    #     #         break
    #
    #     # 1. 复位
    #     # self._rebackXY()
    #
    #     print("复位")
    #     time.sleep(0.1)
    #
    #     # 2. y轴前进
    #     movemsgy = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_BASEMOVE
    #
    #     movemsgy[3:] = self.translatedis2hex(int(2500 / 0.01))
    #
    #     self.mySeria.write(movemsgy)
    #
    #     self._waitfor_hardware_queue_result()
    #
    #     time.sleep(0.1)
    #
    #     self.mySeria.write(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)
    #     print("Y轴移动")
    #
    #     self._waitfor_hardware_queue_result()
    #
    #     self._waitfor_hardware_queue_result()
    #     time.sleep(20)
    #
    #     # 这里将读取到位信息，看到位会收到什么信息 #
    #
    #     # 3. x轴移动
    #     movemsgx = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_BASEMOVE
    #
    #     list_movx = translatedis2hex(int(300 / 0.01))
    #
    #     movemsgx[3:] = list_movx
    #
    #     self.mySeria.write(movemsgx)
    #
    #     self._waitfor_hardware_queue_result()
    #
    #     time.sleep(0.1)
    #
    #     self.mySeria.write(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_X)
    #
    #     print("x轴移动")
    #
    #     self._waitfor_hardware_queue_result()
    #
    #     self._waitfor_hardware_queue_result()
    #
    #     time.sleep(5)
    #
    #     # 4. y轴移动
    #
    #     self.mySeria.write(imc_msg.HARDWAREBASEMSG.MSG_REBACKMOTOR_Y)
    #     self._waitfor_hardware_queue_result()
    #     time.sleep(20)
    #
    #     # 判断y轴移动到上限位 #
    #
    #     # 5. x轴移动
    #
    #     self.mySeria.write(movemsgx)
    #
    #     self._clear_hardware_recqueue()
    #
    #     time.sleep(0.1)
    #
    #     self.mySeria.write(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_X)
    #     print("x轴移动")
    #     time.sleep(5)
    #
    #     self._waitfor_hardware_queue_result()
    #
    #     # 判断x移动到位 #
    #
    #     # 6. y轴移动
    #     movemsgy[3:] = self.translatedis2hex(2500 / 0.01)
    #
    #     self.mySeria.write(movemsgy)
    #
    #     self._clear_hardware_recqueue()
    #
    #     time.sleep(0.1)
    #
    #     self.mySeria.write(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)
    #     print("y轴移动")
    #
    #     self._waitfor_hardware_queue_result()



