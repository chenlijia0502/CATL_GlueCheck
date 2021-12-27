import socket
#
# msg = [0x4A, 0x54, 0x58, 0x02, 0xE1, 0x13, 0xA2, 0x00]
# #elif num == 20:
# msg = [0x4A, 0x54, 0x58, 0x02, 0xE1, 0x14, 0xA2, 0x00]
# #elif num == 21:
# msg = [0x4A, 0x54, 0x58, 0x02, 0xE1, 0x15, 0xA2, 0x00]

MSG_AGV_GETPACKID = [0x4A, 0x54, 0x58, 0x02, 0xE7, ]

AGV_GONGWEI_ID = 21

MSG_AGV_BASE_GET_STATUS = [0x4A, 0x54, 0x58, 0x02, AGV_GONGWEI_ID, 0x15, 0xA2, 0x00]

MSG_AGV_BASE_SET_STATUS = [0x4A, 0x54, 0x58, 0x02, 0xE1, 0x15, 0xA2, 0x00]



class AGVconnection(object):
    def __init__(self):
        super(AGVconnection, self).__init__()
        self.tcp_client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        ip = '192.168.3.100'
        port = 9000
        self.tcp_client.connect((ip, port))

    def sendmsg(self, list_data):
        self.tcp_client.send(bytes(list_data))


if __name__ == "__main__":
    agv = AGVconnection()

    agv.sendmsg()