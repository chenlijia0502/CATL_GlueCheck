import serial
import imc_msg


myserial = serial.Serial(port='COM1', baudrate=115200)

myserial.write(imc_msg.HARDWAREBASEMSG.MSG_JIAJIN)
print (imc_msg.HARDWAREBASEMSG.MSG_JIAJIN)
while 1:
    data  = myserial.read(7).hex()
    print(data)