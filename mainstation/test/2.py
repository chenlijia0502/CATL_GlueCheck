import serial
import time


mySeria = serial.Serial(port="com3", baudrate=115200)  # 波特率比较固定，没必要配置

#mySeria.write([1, 2, 3, 4, 5, 6])
# while 1:
#     data = mySeria.read(7)
#     print (data)
#     time.sleep(10)
#     mySeria.reset_input_buffer()
#     data = mySeria.read(7)
#     print (data)
#     break