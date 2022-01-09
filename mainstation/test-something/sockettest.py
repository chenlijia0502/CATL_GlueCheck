import socket


ip = '192.168.101.1'
port = 9000
tcp_agvclient = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcp_agvclient.connect((ip, int(port)))

msg = [0x4A, 0x54, 0x58, 0x02, 0xE1, 0x13, 0xA2, 0x00]
tcp_agvclient.send(bytes(msg))

tcp_agvclient.recv()