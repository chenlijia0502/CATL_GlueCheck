import numpy as np


def translatedis2hex(nmovedis):
    """
        将移动距离转换为hex ->  转换为两个十六进制
        例如0x27eab: VALUE1为0x7e,VALUE2为0xab,VALUE3为0x00,VALUE4为0x02,
    :param nmovedis:
    :return:
    """
    smalldata = nmovedis % 65536
    bigdata = int(nmovedis / 65536)

    VALUE1 = int(smalldata / 256)
    VALUE2 = smalldata % 256
    VALUE3 = int(bigdata / 256)
    VALUE4 = bigdata % 256

    return [VALUE1, VALUE2, VALUE3, VALUE4]

#print (translatedis2hex(163499))


A = {'d':[1,2 ,3 ,4]}
