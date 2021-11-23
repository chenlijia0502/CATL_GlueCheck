
import serial
import time
mySeria = serial.Serial(port='COM5', baudrate=115200)  # 波特率比较固定，没必要配置

time.sleep(5)
print ("开始清除")
mySeria.reset_input_buffer()

data = mySeria.read(7)

print (data)