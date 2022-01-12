import serial
import threading
import time
import imc_msg
import time
import logging



class SerialManager(object):
    def __init__(self, h_parent, port, baudrate, nreadbuffersize):
        super(SerialManager, self).__init__()
        self.h_parent = h_parent
        self.myserial = serial.Serial(port=port, baudrate=baudrate)

        # 通信日志独此一份log
        # self.logger = logging.getLogger(__name__)
        # self.logger.setLevel(level=logging.INFO)
        # handler = logging.FileHandler("D:\\log\\HARDWARE%s.txt"%time.strftime("%Y-%m-%d"))
        # handler.setLevel(logging.INFO)
        # formatter = logging.Formatter('%(asctime)s - %(message)s')
        # handler.setFormatter(formatter)
        # self.logger.addHandler(handler)
        self.logger = logging.getLogger('UI.%s' % self.__class__.__name__)


        self.list_write = []
        self.list_read = []
        self.n_readbuffersize = nreadbuffersize
        self.thread1 = threading.Thread(target=self._cycle_write)
        self.thread2 = threading.Thread(target=self._cycle_read)
        self.thread1.start()
        self.thread2.start()


    def read(self, ntimeout=None):
        """

        :param ntimeout: 超时单位秒,超时返回None
        :return:
        """
        if ntimeout == None:
            while True:

                if self.list_read != []:

                    data = self.list_read[0]

                    del self.list_read[0]

                    return data

                else:

                    time.sleep(0.2)
        else:
            nstarttime = time.time()

            readdata = b'F' * self.n_readbuffersize * 2

            while time.time() - nstarttime <= ntimeout:

                if self.list_read != []:

                    readdata = self.list_read[0]

                    del self.list_read[0]

                    break

                else:

                    time.sleep(0.2)

            return readdata

    def write(self, data):

        self.list_write.append(data)


    def reset_input_buffer(self):

        self.myserial.reset_input_buffer()

        self.list_read = []


    def isOpen(self):

        return self.myserial.isOpen()

    def reconnect(self):
        self.myserial.open()


    def _cycle_read(self):

        while True:

            data = self.myserial.read(self.n_readbuffersize)

            hexdata = data.hex()

            self.logger.log(logging.INFO, "read: " + hexdata)

            print(hexdata)


            if len(hexdata) > 3:

                if data.hex() in imc_msg.HARDWAREBASEMSG.MSG_GUANGSHAN:

                    self.h_parent.callback2showerror("！！！光栅被遮挡，请注意！！！")

                elif data.hex() in imc_msg.HARDWAREBASEMSG.MSG_JITING:

                        self.h_parent.callback2showerror("！！！急停按下！！！")

                elif data.hex() in imc_msg.HARDWAREBASEMSG.MSG_MENKONG:

                        self.h_parent.callback2showerror("！！！门控打开！！！")

                else:

                    self.list_read.append(data)

            time.sleep(0.5)


    def _hex_to_str(self, list_data):
        s = ""
        for data in list_data:
            high = int(data / 16)
            s += hex(high & 0x0F)[2:]
            low = data % 16
            s += hex(low)[2:]
        return s

    def _cycle_write(self):

        while True:

            if self.list_write != []:

                data = self.list_write[0]

                self.logger.log(logging.INFO, "write: " + self._hex_to_str(data))

                self.myserial.write(data)

                del self.list_write[0]

                time.sleep(0.5)

            else:

                time.sleep(0.5)
