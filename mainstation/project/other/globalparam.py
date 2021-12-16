

class StaticConfigParam:
    DIS2PULSE = 0.01 # mm转脉冲
    DIS2PIXEL = 10   # mm转像素
    MAX_Y_LEN = 2620 # Y轴最长距离，单位mm
    MAX_X_LEN = 900  # x轴最长距离，单位mm
    RUN_MORE_DIS = 20# 控制y轴取整后多走，目的是使它拍多不到一张。原因电机跟相机分辨率不一样，所以导致电机运动2000mm，不一定拍摄20000行，所以多走一点，让他凑不够一张但能保证之前的图是够的
