
import serial
import time
mySeria = serial.Serial(port='COM1', baudrate=115200, timeout=3)  # 波特率比较固定，没必要配置

# time.sleep(5)
# print ("开始清除")
# mySeria.reset_input_buffer()

# SEND = [0x01, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00]
# #SEND = [0xff, 0xff, 0xff, 0x31, 0x01, 0x01, 0x01]
#
# mySeria.write(SEND)

print(time.time())
data = mySeria.read(7).hex()
print (data == "")
print(time.time())


def str_to_hex(data):
    list_hex = []
    for i in range(0, len(data), 2):
        list_hex.append(int(data[i:i+2], 16))
    return list_hex


print (str_to_hex(data))

