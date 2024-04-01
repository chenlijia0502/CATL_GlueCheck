import socket

"""
    测试小车通信
    先监听19工位，然后放行19，然后监听20工位，然后获取packid, 然后放行20，
"""
MSG_LISTEN =[[0x4A, 0x54, 0x58, 0x02, 0xE1, 0x13, 0xA2, 0x00],
             [0x4A, 0x54, 0x58, 0x02, 0xE1, 0x14, 0xA2, 0x00],
             [0x4A, 0x54, 0x58, 0x02, 0xE1, 0x15, 0xA2, 0x00]]

MSG_CONTROL = [[0x4A, 0x54, 0x58, 0x03, 0xE3, 0x13, 0x02, 0xA2, 0x00],
               [0x4A, 0x54, 0x58, 0x03, 0xE3, 0x14, 0x02, 0xA2, 0x00],
               [0x4A, 0x54, 0x58, 0x03, 0xE3, 0x15, 0x02, 0xA2, 0x00]]

MSG_GETPACKID = [0x4A, 0x54, 0x58, 0x02, 0xE7, 0x00, 0x00, 0x00]


def str_to_hex(data):
    list_hex = []
    for i in range(0, len(data), 2):
        list_hex.append(int(data[i:i+2], 16))
    return list_hex

ip = '192.168.101.1'
port = 9000
tcp_agvclient = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcp_agvclient.connect((ip, int(port)))

tcp_agvclient.settimeout(2)
try:
    MSG_LISTEN[1][5] = 19
    tcp_agvclient.send(bytes(MSG_LISTEN[1]))
    data = tcp_agvclient.recv(100).hex()
    print (data)
except Exception as e:
    print (e)

#
# nagvid = 0
#
# while 1:
#     tcp_agvclient.send(bytes(MSG_LISTEN[1]))
#
#     data = tcp_agvclient.recv(200).hex()
#
#     list_data = str_to_hex(data)
#
#     print ('rec: ', list_data)
#
#     if list_data[8] == 3:# 3代表站点有车
#
#         nagvid = list_data[7]
#
#         break
#
# print(nagvid)
#
#
# """根据上面的agv id ，获取packid"""
#
# # MSG_GETPACKID[5] = nagvid
# #
# # tcp_agvclient.send(bytes(MSG_GETPACKID))
# #
# # readdata = tcp_agvclient.recv(26).hex()
# # list_readdata = str_to_hex(readdata)
# #
# # datalen = list_readdata[3]
# #
# # targetlist = list_readdata[6 : 6 + datalen - 2]
# #
# # print(targetlist)
# # spackid = ""
# # for data in targetlist:
# #     spackid += chr(data)
# # print(spackid)
#
#
#
# """发送工位，控制其走"""
#
# # tcp_agvclient.send(bytes(MSG_CONTROL[0]))
# # print(tcp_agvclient.recv(11).hex())
#
# """测试小车是否停住"""
#
# # while 1:
# #     tcp_agvclient.send(bytes(MSG_LISTEN[1]))
# #
# #     data = tcp_agvclient.recv(11).hex()
# #
# #     list_data = str_to_hex(data)
# #
# #     print ('rec: ', list_data)
# #
# #     if list_data[8] == 3:# 3代表站点有车
# #
# #         nagvid = list_data[7]
# #
# #         break
# #
# # print("小车已到： ", nagvid)