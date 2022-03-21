import serial
import threading
import time
import imc_msg
import time
import logging
from library.common.globalparam import LogInfo



class SerialManager(object):
    def __init__(self, h_parent, port, baudrate, nreadbuffersize):
        super(SerialManager, self).__init__()
        self.h_parent = h_parent

        try:
            self.myserial = serial.Serial(port=port, baudrate=baudrate)
        except Exception as E:
            pass
        # 通信日志独此一份log
        self.logger = logging.getLogger(__name__)
        self.logger.setLevel(level=logging.INFO)
        self.handler = logging.FileHandler(LogInfo.PATH_SAVE_LOG + 'HARDWARE%s.log' % time.strftime("%Y-%m-%d"))
        self.handler.setLevel(logging.INFO)
        formatter = logging.Formatter('%(asctime)s - %(message)s')
        self.handler.setFormatter(formatter)
        self.logger.addHandler(self.handler)

        self.queue_write = SqQueue(100)
        self.queue_read = SqQueue(100)
        self.n_readbuffersize = nreadbuffersize
        self.thread1 = threading.Thread(target=self._cycle_write)
        self.thread2 = threading.Thread(target=self._cycle_read)
        self.thread1.start()
        self.thread2.start()


    def updatelog(self):
        self.logger.removeHandler(self.handler)
        self.handler = logging.FileHandler(LogInfo.PATH_SAVE_LOG + 'HARDWARE_%s.log' % time.strftime("%Y-%m-%d"))
        self.handler.setLevel(logging.INFO)
        self.formatter = logging.Formatter('%(levelname)s %(asctime)s %(message)s')
        self.handler.setFormatter(self.formatter)
        self.logger.addHandler(self.handler)


    def read(self, ntimeout=None):
        """

        :param ntimeout: 超时单位秒,超时返回None
        :return:
        """
        if ntimeout == None:
            while True:

                if not self.queue_read.isEmpty():

                    data = self.queue_read.pop()

                    if data == None:

                        self.logger.log(logging.ERROR, "SerialManager错误，读取queue_read队列时竟然为空")

                    else:

                        return data

                else:

                    time.sleep(0.2)
        else:
            nstarttime = time.time()

            data = b'F' * self.n_readbuffersize * 2

            while time.time() - nstarttime <= ntimeout:

                if not self.queue_read.isEmpty():

                    data = self.queue_read.pop()

                    break

                else:

                    time.sleep(0.2)

            return data

    def write(self, data):
        self.queue_write.push(data)


    def reset_input_buffer(self):

        self.myserial.reset_input_buffer()

        self.queue_read.clear()


    def isOpen(self):

        return self.myserial.isOpen()

    def reconnect(self):
        self.myserial.open()


    def _cycle_read(self):
        """
        读取线程，一直循环读取buffer
        """
        while True:

            data = self.myserial.read(self.n_readbuffersize)

            hexdata = data.hex()

            self.logger.log(logging.INFO, "read: " + hexdata)

            if len(hexdata) > 3:

                if data.hex() in imc_msg.HARDWAREBASEMSG.MSG_GUANGSHAN:

                    self.h_parent.callback2showerror("！！！光栅被遮挡，请注意！！！")

                elif data.hex() in imc_msg.HARDWAREBASEMSG.MSG_JITING:

                        self.h_parent.callback2showerror("！！！急停按下！！！")

                elif data.hex() in imc_msg.HARDWAREBASEMSG.MSG_MENKONG:

                        self.h_parent.callback2showerror("！！！门控打开！！！")

                else:
                    self.queue_read.push(data)

            time.sleep(0.2)


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

            if not self.queue_write.isEmpty():

                data = self.queue_write.pop()

                self.logger.log(logging.INFO, "write: " + self._hex_to_str(data))

                self.myserial.write(data)

            time.sleep(0.2)





class SqQueue(object):
    def __init__(self, maxsize):
        self.queue = [None] * maxsize
        self.maxsize = maxsize
        self.front = 0
        self.rear = 0

    # 返回当前队列的长度
    def QueueLength(self):
        return (self.rear - self.front + self.maxsize) % self.maxsize

    # 如果队列未满，则在队尾插入元素，时间复杂度O(1)
    def push(self, data):
        if self.isFull():
            #print("The queue is full!")
            return None
        else:
            self.queue[self.rear] = data
           # self.queue.insert(self.rear,data)
            self.rear = (self.rear + 1) % self.maxsize

    # 如果队列不为空，则删除队头的元素,时间复杂度O(1)
    def pop(self):
        if self.isEmpty():
            #print("The queue is empty!")
            return None
        else:
            data = self.queue[self.front]
            self.queue[self.front] = None
            self.front = (self.front + 1) % self.maxsize
            return data

    def isEmpty(self):
        return self.rear == self.front

    def isFull(self):
        return (self.rear + 1) % self.maxsize == self.front

    # 输出队列中的元素
    def ShowQueue(self):
        for i in range(self.maxsize):
            print(self.queue[i],end=',')
        print(' ')

    def clear(self):
        self.queue = [None] * len(self.queue)
        self.front = 0
        self.rear = 0