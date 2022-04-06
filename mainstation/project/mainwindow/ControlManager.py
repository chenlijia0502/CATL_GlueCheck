
import time
import imc_msg
from project.other.globalparam import StaticConfigParam
from project.mainwindow.SerialManager import SerialManager
from library.ipc import ipc_tool
import logging
import numpy as np
from library.common.readconfig import readconfig, MAINSTATION_CONFIG
from library.common.globalparam import LogInfo



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
        self.b_checkstatus = False#检测状态，检测状态为False的时候停止检测，很多循环都依赖这个状态，只要不在检测状态立刻返回,方便关闭线程
        self.b_zerostatus = False# 每次调用rebackzero内会会将这个值置为True，检测开始后又置为False，目的是减少时间花费
        self.b_zmovestatus = False# 这个值代表z轴的高度，只要applicationsetting.xml的值不变，程序每次启动之后，只触发一次
        self.b_xyrestartstatus = True# 程序重启的时候这个值为True，但是设备重启了，且x y轴第一条控制命令必须为复位x y;这个实现方式不完美。我只是用了程序启动的时候必先调用rebackxy的条件
        self.z_camerapos = int(readconfig(MAINSTATION_CONFIG)['CAMERAZ_POS'])
        self.logger = logging.getLogger(__name__)
        self.logger.setLevel(level=logging.INFO)
        self.handler = logging.FileHandler("D:\\log\\control_%s.txt"%time.strftime("%Y-%m-%d"))
        self.handler.setLevel(logging.INFO)
        formatter = logging.Formatter('%(asctime)s - %(message)s')
        self.handler.setFormatter(formatter)
        self.logger.addHandler(self.handler)


    def updatelog(self):
        self.logger.removeHandler(self.handler)
        self.handler = logging.FileHandler(LogInfo.PATH_SAVE_LOG + 'control_%s.log' % time.strftime("%Y-%m-%d"))
        self.handler.setLevel(logging.INFO)
        self.formatter = logging.Formatter('%(levelname)s %(asctime)s %(message)s')
        self.handler.setFormatter(self.formatter)
        self.logger.addHandler(self.handler)


    def setserial(self, serial:SerialManager):
        self.mySeria = serial


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
        self.logger.log(logging.INFO, 'hope rec: '+ str(info))
        data = []
        if info == None:
            data = self.mySeria.read().hex()
            print ('None rec: ', data)
            self.logger.log(logging.INFO, 'read ori data: ' + str(data))
            data = str_to_hex(data)
        else:
            if len(np.array(info).shape) == 1:
                while self.b_checkstatus:
                    data = self.mySeria.read(ntimeout).hex()
                    self.logger.log(logging.INFO, 'read ori data: ' + str(data))

                    data = str_to_hex(data)
                    if data != info:
                        continue
                    else:
                        break
            else:
                list_status = [False for i in range(len(info))]
                while np.sum(list_status) != len(list_status) and self.b_checkstatus:
                    data = self.mySeria.read(ntimeout).hex()
                    print('read ori data: ', data)
                    self.logger.log(logging.INFO, 'read ori data: ' + str(data))
                    data = str_to_hex(data)
                    print('read data: ', data)

                    for nindex in range(len(info)):

                        if info[nindex] == data:

                            list_status[nindex] = True

                            break
        # if (info == imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE or info == imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_ARRIVE or
        #         info == imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Z_ARRIVE):
        #     time.sleep(0.5)# x、y、z轴到位之后会有一小段时间无法接收数据，所以直接延时等待
        time.sleep(0.3)#这个延时必须要加，下位机回复指令之后会有一小段很奇怪的时间不能接收到上位机的数据
        return data




    def _rebackZ_Base(self):
        """
        程序第一次启动将Z轴设置到ApplicationSetting.xml中的基准位置
        """
        if not self.b_zmovestatus:

            MSG_Z_POS = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Z_BASEMOVE

            print ('-------------- 移动z距离: ', self.z_camerapos)

            MSG_Z_POS[3:] = translatedis2hex(int(self.z_camerapos / StaticConfigParam.RATE_Z))

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_REBACKMOTOR_Z)

            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Z_XIANWEI)

            self._sendhardwaremsg(MSG_Z_POS)

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Z_MOVE)

            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Z_ARRIVE)

            if not self.b_checkstatus: return# 以防是因为停止检测而导致的顺利往下执行
            #
            self.b_zmovestatus = True


    def _rebackXY(self):
        """
            复位到模组的原点，模组会回到传感器并自我标定。这是因为设备有可能运动过程中丢失原点
            :return:
        """
        if not self.b_zerostatus or self.b_xyrestartstatus:  # 两种情况进入，第一种是设备未返回过原点。第二种是设备刚启动必须回原点
            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_ZERO)
            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_ARRIVE)
            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ZERO)
            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)
            self.b_zerostatus = True
            self.b_xyrestartstatus = False


    def _rebackZERO(self):
        """控制电机返回0点， 注意0点是漂的，所以程序刚启动不能控制返回0点，而应该控制返回原点"""
        if not self.b_xyrestartstatus:

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_REBACKMOTOR_X)

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_X)

            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_ARRIVE)

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_REBACKMOTOR_Y)

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)



    def _sendhardwaremsg(self, info):
        print('send, ', info)
        self.mySeria.write(info)
        self.logger.log(logging.INFO, 'send: ' + str(info))


    def setcheckstatus(self, bstatus):
        self.b_checkstatus = bstatus

        self.control_doorstatus(bstatus)

        if self.b_checkstatus == False:

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_END_CHECK)




    def _judgePackZisRight(self, list_list_pos):
        """
        判断pack高度是否正确，起防呆防错作用
        param: list_list_pos 由[[x, y, z],[x, y, z].....]组成
        """

        for list_pos in list_list_pos:

            x, y, z = list_pos

            self._control2markpos(x, y)

            nreadz = self._GetZpos()

            if nreadz is None or abs(nreadz - float(z)) > 15:  # 偏移超过15mm则处理

                return 0

        return 1


    def _GetZpos(self):
        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_GET_Z_POS)

        while self.b_checkstatus:

            list_recdata = self._waitfor_hardware_queue_result()

            if list_recdata[:2] == imc_msg.HARDWAREBASEMSG.MSG_Z_RESPOND_HEAD:

                nreadz = (int(list_recdata[3]) * 256 + int(list_recdata[4])) * 0.112 - 198.2

                print('get z pos: ', nreadz)

                return nreadz

        return None


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


    def MakeEveryposInstatus(self):
        """
        让每个气缸进入状态，且具备超时功能。成功执行返回True， 否则返回False
        """
        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_JIAJIN)

        nreadstatus = self._recmsgwithruntimeerror(
            [imc_msg.HARDWAREBASEMSG.MSG_LEFT_JIAJIN_DAOWEI, imc_msg.HARDWAREBASEMSG.MSG_RIGHT_JIAJIN_DAOWEI], 6)

        if not nreadstatus:
            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_CONTROL_ALARM)

            self.h_parent.callback2showerror("！！！夹紧气缸未到位！！！")

            return False

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_SHENG)

        nreadstatus = self._recmsgwithruntimeerror(
            [imc_msg.HARDWAREBASEMSG.MSG_LEFT_DINGSHENG_DAOWEI, imc_msg.HARDWAREBASEMSG.MSG_RIGHT_DINGSHENG_DAOWEI], 6)

        if not nreadstatus:
            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_CONTROL_ALARM)

            self.h_parent.callback2showerror("！！！顶升气缸未到位！！！")

            return False

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_DUIWEI_JIANG)

        nreadstatus = self._recmsgwithruntimeerror([imc_msg.HARDWAREBASEMSG.MSG_LEFT_DUIWEI_JIANG_DAOWEI,
                                                    imc_msg.HARDWAREBASEMSG.MSG_RIGHT_DUIWEI_JIANG_DAOWEI], 6)

        if not nreadstatus:
            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_CONTROL_ALARM)

            self.h_parent.callback2showerror("！！！定位气缸未到位！！！")

            return False

        self._clear_hardware_recqueue()  # 清除接收缓存

        return True


    def _control2markpos(self, x, y):
        movemsgx = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_BASEMOVE

        movemsgy = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_BASEMOVE

        movemsgx[3:] = translatedis2hex(int(x / self._DIS2PULSE))

        self._sendhardwaremsg(movemsgx)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_X)

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_ARRIVE)

        movemsgy[3:] = translatedis2hex(int(y / self._DIS2PULSE))

        self._sendhardwaremsg(movemsgy)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)


    def _recmsgwithruntimeerror(self, info, ntimeout):
        """
        测试接收数据时间，如果超过
        超时时间，以1s为单位
        """
        self.logger.log(logging.INFO, '_recmsgwithruntimeerror hope rec: '+ str(info))
        print('_recmsgwithruntimeerror hope rec: ', info)
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
            print ("超时部分： ", int(time.time() - curtime), time.time(), curtime)

        return b_rectargetstatus


    def _waitformsgwithonlyone(self, list_info):
        """
        当接收到 list_info 中一个值的时候，就可结束这个等待
        """
        while self.b_checkstatus:
            data = self.mySeria.read().hex()
            print('read ori data: ', data)
            self.logger.log(logging.INFO, 'read ori data: ' + str(data))

            data = str_to_hex(data)
            bfind = False
            for info in list_info:
                if data != info:
                    pass
                else:
                    bfind = True
            if bfind:
                break
            else:
                continue



    def waitforstart(self, bisfirst):
        """
            等待双启动按下
        """
        self._clear_hardware_recqueue()
        if bisfirst:
            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_REC_START)

            if self.b_checkstatus:

                self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_START_CHECK)


    def control_guangshan(self, nstatus):
        if nstatus == 0:
            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_TURNON_GUANGSHAN)
        elif nstatus == 1:
            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_TURNOFF_GUANGSHAN1)
        elif nstatus == 2:
            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_TURNOFF_GUANGSHAN2)


    def ALARM(self):
        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_CONTROL_ALARM)


    def control_doorstatus(self, bstatus):
        """
        控制门控状态
        bstatus == True 为开始检测状态，夹紧门控
        bstatus == False 为停止检测状态，松开门控
        """
        if bstatus:
            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_MENKONG_JIAJIN)
        else:
            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_MENKONG_SONGKAI)





    def control_calibrate(self, *list_data):
        """
        :param list_data: [x, y1, y2] [起拍X位置， 起拍Y位置， 拍摄结束位置Y]
        :return:
        """
        self.logger.log(logging.INFO, "--------------- 开始做首件检测 ---------------")

        ipc_tool.kxlog("ControlManager", logging.INFO, "开始做首件")

        self._clear_hardware_recqueue()

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_REC_START)

        # 把机构伸出去
        self._sendhardwaremsg([0x01, 0x03, 0x08, 0x01, 0x00, 0x00, 0x00])

        self._waitfor_hardware_queue_result([0x01, 0x02, 0x01, 0x21, 0x00, 0x00, 0x00 ])
        # 1. 复位

        self._rebackZERO()

        self._rebackXY()

        self._rebackZ_Base()

        self._clear_hardware_recqueue()
        # 3. 移动到标定块位置
        movemsgx = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_BASEMOVE

        movemsgx[3:] = translatedis2hex(list_data[0] / self._DIS2PULSE)

        self._sendhardwaremsg(movemsgx)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_X)


        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_ARRIVE)

        movemsgy = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_BASEMOVE

        movemsgy[3:] = translatedis2hex(list_data[1] / self._DIS2PULSE)

        self._sendhardwaremsg(movemsgy)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)

        # 3.
        self.h_parent.callback2dotcheck()

        time.sleep(2)#TODO：这里有个bug，硬件在y轴即将到位的时候控制所有光源的亮暗会失效且会答复00000000000

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_OPEN_ALL_LIGHT)

        self._waitfor_hardware_queue_result()

        movemsgy[3:] = translatedis2hex(list_data[2] / self._DIS2PULSE)

        self._sendhardwaremsg(movemsgy)

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)

        time.sleep(1)

        self.h_parent.callback2stopdotcheck()

        #把机构收回去

        self._sendhardwaremsg([0x01, 0x03, 0x08, 0x02, 0x00, 0x00, 0x00])

        self._waitfor_hardware_queue_result([0x01, 0x02, 0x01, 0x22, 0x00, 0x00, 0x00])

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_CLOSE_ALL_LIGHT)


    def buildmodel_getsensorhigh(self, *list_info):
        """
        list_info 中放着N对 x, y 坐标，根据根据这几个坐标进行位置设置
        :param list_info:[[x, y], [x, y]]
        :return:
        """

        self._clear_hardware_recqueue()

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_REC_START)

        if not self.MakeEveryposInstatus(): return

        self._rebackZERO()

        self._rebackXY()

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

        self._rebackZERO()

        self.b_checkstatus = False


    def check_control(self, list_info):
        """
        检测控制，跟二次建模一样的轨迹，不一样的是加了过程判断以及取图反转
        :param list_info: [[list_x, list_y, list_z]]  list_z 比较特殊，由【[x, y, z]、[x, y, z]。。。。】组成
        :return:
        """
        self.logger.log(logging.INFO, '--------------- 开始检测控制 -------------------')

        ipc_tool.kxlog("ControlManager", logging.INFO, "开始检测控制")

        self._clear_hardware_recqueue()

        ipc_tool.kxlog("ControlManager", logging.INFO, "1. 复位所有气缸状态")

        if not self.MakeEveryposInstatus(): return False

        self.h_parent.closeCamera()

        ipc_tool.kxlog("ControlManager", logging.INFO, "2. 控制 X Y 电机回零点")

        self._rebackZERO()

        if not self.b_checkstatus: return False

        ipc_tool.kxlog("ControlManager", logging.INFO, "3. 控制 X Y 电机回原点")

        self._rebackXY()

        ipc_tool.kxlog("ControlManager", logging.INFO, "4. 控制 Z 电机到目标高度")

        self._rebackZ_Base()

        self.b_zerostatus = False

        self._clear_hardware_recqueue()

        # # 判断托盘是否到位
        ipc_tool.kxlog("ControlManager", logging.INFO, "5. 判断托盘是否到位")

        nstatus = self._judgePackZisRight(list_info[2])

        if nstatus == 0:

            self.MakeEveryposFuwei()

            #self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_CONTROL_ALARM)
            self.h_parent.callback2lockwindow()

            self.h_parent.callback2showerror("！！！！托盘未到位，无法检测！！！！")

            return False

        ipc_tool.kxlog("ControlManager", logging.INFO, "6. 控制 X Y 电机回零点")

        self._rebackZERO()

        self._clear_hardware_recqueue()  # 清除接收缓存

        if not self.b_checkstatus:  return False

        movemsgx = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_BASEMOVE

        movemsgy = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_BASEMOVE

        ipc_tool.kxlog("ControlManager", logging.INFO, "7. 开始XY轴运动拍摄控制")

        for i in range(len(list_info[0])):

            ipc_tool.kxlog("ControlManager", logging.INFO, "7.%i-1 控制x轴移动"%i)

            x = list_info[0][i]

            y = list_info[1][i]

            movemsgx[3:] = translatedis2hex(x / self._DIS2PULSE)

            self._sendhardwaremsg(movemsgx)

            self._waitfor_hardware_queue_result()

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_X)

            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_ARRIVE)

            if i != 0:

                self.h_parent.sendmsg_changecol()# 通知子站图像换列，做好标记

            if not self.b_checkstatus:  return False

            ipc_tool.kxlog("ControlManager", logging.INFO, "7.%i-2 打开光源1"%i)

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_OPEN_LIGHT1)

            self._waitfor_hardware_queue_result()

            self.h_parent.onlyopencamera()

            time.sleep(1)

            movemsgy[3:] = translatedis2hex(y / self._DIS2PULSE)

            ipc_tool.kxlog("ControlManager", logging.INFO, "7.%i-2 控制Y轴正向移动"%i)

            self._sendhardwaremsg(movemsgy)

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)

            if not self.b_checkstatus:  return False

            time.sleep(1)#

            ipc_tool.kxlog("ControlManager", logging.INFO, "7.%i-3 关闭相机并更换采集方向"%i)

            self.h_parent.closeCamera()

            self.h_parent.changeCameraCapturedirection()

            ipc_tool.kxlog("ControlManager", logging.INFO, "7.%i-4 打开光源2"%i)

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_OPEN_LIGHT2)

            self._waitfor_hardware_queue_result()

            self.h_parent.onlyopencamera()

            ipc_tool.kxlog("ControlManager", logging.INFO, "7.%i-5 控制Y轴反向移动"%i)

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_REBACKMOTOR_Y)

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)

            if not self.b_checkstatus:  return False

            time.sleep(1)

            ipc_tool.kxlog("ControlManager", logging.INFO, "7.%i-6 关闭相机并更换采集方向"%i)

            self.h_parent.closeCamera()

            self.h_parent.changeCameraCapturedirection(True)

        ipc_tool.kxlog("ControlManager", logging.INFO, "8. 开始XY轴运动完成，复位气缸")

        self.MakeEveryposFuwei()

        ipc_tool.kxlog("ControlManager", logging.INFO, "9. 控制 X Y 电机回零点")

        self._rebackZERO()

        ipc_tool.kxlog("ControlManager", logging.INFO, "10. 控制 X Y 电机回原点")

        self._rebackXY()

        ipc_tool.kxlog("ControlManager", logging.INFO, "检测控制结束")

        return True


    def buildmodel(self, *list_info):
        """
        :param list_info: [nStartX, ndisX, ndisY, nXtimes] [起拍X位置， 单次X移动距离， 单次Y移动距离， X次数]
        :return:
        """
        self.logger.log(logging.INFO, '--------------- 开始取全图建模 -------------------')

        ipc_tool.kxlog("ControlManager", logging.INFO, "开始取全图建模")

        self._clear_hardware_recqueue()

        ipc_tool.kxlog("ControlManager", logging.INFO, "1. 等待按下启动按钮")

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_REC_START)

        ipc_tool.kxlog("ControlManager", logging.INFO, "2. 复位所有气缸")

        if not self.MakeEveryposInstatus(): return

        self._clear_hardware_recqueue()  # 清除接收缓存

        self.h_parent.closeCamera()

        ipc_tool.kxlog("ControlManager", logging.INFO, "3. 模组 X Y 回零点，并复位")

        self._rebackZERO()

        self._rebackXY()

        ipc_tool.kxlog("ControlManager", logging.INFO, "4. Z轴 回零点，并到基准位")

        self._rebackZ_Base()

        ipc_tool.kxlog("ControlManager", logging.INFO, "5. 打开所有光源")

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_OPEN_ALL_LIGHT)

        self._waitfor_hardware_queue_result()

        movemsgx = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_BASEMOVE

        ipc_tool.kxlog("ControlManager", logging.INFO, "6. X轴运动到偏移位置")

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

        ipc_tool.kxlog("ControlManager", logging.INFO, "7. 进入控制拍摄循环")

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

        ipc_tool.kxlog("ControlManager", logging.INFO, "8. 结束控制拍摄循环，关闭所有光源")

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_CLOSE_ALL_LIGHT)

        self.h_parent.callback2showbigimg()

        ipc_tool.kxlog("ControlManager", logging.INFO, "9. 复位所有气缸")

        self.MakeEveryposFuwei()

        ipc_tool.kxlog("ControlManager", logging.INFO, "10. 控制电机返回原点")

        self._rebackZERO()

        self.b_checkstatus = False

        self.logger.log(logging.INFO, '--------------- 结束取全图建模 -------------------')

        ipc_tool.kxlog("ControlManager", logging.INFO, "结束取全图建模")


    def buildmodel_second(self, *list_info):
        """
        二次建模控制
        :param list_info: [[list_x, list_y]]
        :return:
        """
        self.logger.log(logging.INFO, '--------------- 开始二次取图建模 -------------------')

        ipc_tool.kxlog("ControlManager", logging.INFO, "开始二次取图建模")

        self._clear_hardware_recqueue()

        ipc_tool.kxlog("ControlManager", logging.INFO, "1. 等待按下启动按钮")

        self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_REC_START)

        ipc_tool.kxlog("ControlManager", logging.INFO, "2. 复位所有气缸")

        if not self.MakeEveryposInstatus(): return

        self.h_parent.closeCamera()

        ipc_tool.kxlog("ControlManager", logging.INFO, "3. X Y 轴模组返回零点")

        self._rebackZERO()

        ipc_tool.kxlog("ControlManager", logging.INFO, "4. X Y 轴模组返回原点")

        self._rebackXY()

        ipc_tool.kxlog("ControlManager", logging.INFO, "5. Z轴模组先复位，后移动到规定位置")

        self._rebackZ_Base()

        ipc_tool.kxlog("ControlManager", logging.INFO, "6. 打开所有光源")

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_OPEN_ALL_LIGHT)

        self._waitfor_hardware_queue_result()

        movemsgx = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_BASEMOVE

        movemsgy = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_BASEMOVE

        ipc_tool.kxlog("ControlManager", logging.INFO, "7. 进入移动拍摄循环")

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

        ipc_tool.kxlog("ControlManager", logging.INFO, "7. 结束移动拍摄循环，关闭所有光源")

        self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_CLOSE_ALL_LIGHT)

        self.h_parent.callback2showbigimg_second()

        ipc_tool.kxlog("ControlManager", logging.INFO, "8. 复位所有气缸")

        self.MakeEveryposFuwei()

        ipc_tool.kxlog("ControlManager", logging.INFO, "9. 模组X Y轴返回零点")

        self._rebackZERO()

        self.b_checkstatus = False

        self.logger.log(logging.INFO, '--------------- 结束二次取图建模 -------------------')

        ipc_tool.kxlog("ControlManager", logging.INFO, "结束二次取图建模")



    def testcamera(self):
        """
        20220313 测试相机采集图像
        """
        print("jinru")
        self._clear_hardware_recqueue()

        self.h_parent.closeCamera()

        self._rebackZERO()

        if not self.b_checkstatus: return False

        self._rebackXY()

        self.b_zerostatus = False

        self._clear_hardware_recqueue()
        print("1")


        movemsgx = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_X_BASEMOVE

        movemsgy = imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_BASEMOVE

        print("2")

        self.h_parent.onlyopencamera()

        while self.b_checkstatus:

            y = 2400

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_OPEN_LIGHT1)

            self._waitfor_hardware_queue_result()

            self.h_parent.onlyopencamera()

            time.sleep(1)

            movemsgy[3:] = translatedis2hex(y / self._DIS2PULSE)

            self._sendhardwaremsg(movemsgy)

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)

            if not self.b_checkstatus:  return False

            time.sleep(2)#

            self.h_parent.closeCamera()

            self.h_parent.changeCameraCapturedirection()

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_OPEN_LIGHT2)

            self._waitfor_hardware_queue_result()

            self.h_parent.onlyopencamera()

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_REBACKMOTOR_Y)

            self._sendhardwaremsg(imc_msg.HARDWAREBASEMSG.MSG_STARTMOTOR_Y)

            self._waitfor_hardware_queue_result(imc_msg.HARDWAREBASEMSG.MSG_MOTOR_Y_ARRIVE)

            if not self.b_checkstatus:  return False

            time.sleep(2)

            self.h_parent.closeCamera()

            self.h_parent.changeCameraCapturedirection(True)



        return True
