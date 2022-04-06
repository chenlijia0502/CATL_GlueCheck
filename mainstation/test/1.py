
MSG_MOTOR_Z_MOVE = [0x01, 0x02, 0x03, 0x01, 0x00, 0x00, 0x00  ]  # 向上动

MSG_MOTOR_Z_BASEMOVE = [0x01, 0x04, 0x03, 0x00, 0x00, 0x00, 0x00] # 移动距离

RATE_Z = 1.0 / 800  # Z轴分辨率，mm/_rate 转换为发送脉冲


MSG_REBACKMOTOR_Z = [0x01, 0x02, 0x03, 0x02, 0x00, 0x00, 0x00]  # Z轴复位


def hex_to_str(list_data):
    s = ""
    for data in list_data:
        high = int(data / 16)
        s += hex(high & 0x0F)[2:]
        low = data % 16
        s += hex(low)[2:]
        s+= " "
    return s

def translatedis2hex(nmovedis):
    """
        将移动距离转换为hex ->  转换为两个十六进制
        例如0x27eab: VALUE1为0x7e,VALUE2为0xab,VALUE3为0x00,VALUE4为0x02,
    :param nmovedis:
    :return:
    """
    nmovedis = int(nmovedis)
    smalldata = nmovedis % 65536
    bigdata = int(nmovedis / 65536)

    VALUE1 = int(smalldata / 256)
    VALUE2 = smalldata % 256
    VALUE3 = int(bigdata / 256)
    VALUE4 = bigdata % 256

    return [VALUE1, VALUE2, VALUE3, VALUE4]


if __name__ == "__main__":
    MSG_Z_POS = MSG_MOTOR_Z_BASEMOVE

    z_camerapos = 50# 单位mm

    MSG_Z_POS[3:] = translatedis2hex(int(z_camerapos / RATE_Z))

    print ("复位: ", hex_to_str(MSG_REBACKMOTOR_Z))

    print("运动%dmm: "%z_camerapos, hex_to_str(MSG_Z_POS))

    print("启动: ", hex_to_str(MSG_MOTOR_Z_MOVE))