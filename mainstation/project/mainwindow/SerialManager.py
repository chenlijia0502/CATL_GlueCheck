import serial
import threading
import time
import imc_msg

class SerialManager(object):
    def __init__(self, h_parent, port, baudrate, nreadbuffersize):
        super(SerialManager, self).__init__()
        self.h_parent = h_parent
        self.myserial = serial.Serial(port=port, baudrate=baudrate)
        self.list_write = []
        self.list_read = []
        self.n_readbuffersize = nreadbuffersize
        self.thread1 = threading.Thread(target=self._cycle_write)
        self.thread2 = threading.Thread(target=self._cycle_read)
        self.thread1.start()
        self.thread2.start()


    def read(self):

        while True:

            if self.list_read != []:

                data = self.list_read[0]

                del self.list_read[0]

                return data

            else:

                time.sleep(0.2)


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

            if data != "":

                if data.hex() == imc_msg.HARDWAREBASEMSG.MSG_GUANGSHAN:

                    self.h_parent.callback2warnguangshan()

                else:

                    self.list_read.append(data)


    def _cycle_write(self):

        while True:

            if self.list_write != []:

                data = self.list_write[0]

                self.myserial.write(data)

                del self.list_write[0]

                time.sleep(0.1)

            else:

                time.sleep(0.1)
