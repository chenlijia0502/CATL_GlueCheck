

class StaticConfigParam:
    DIS2PULSE = 0.01 # mm转脉冲
    DIS2PIXEL = 10   # mm转像素, 这个值会根据ApplicationSetting.xml的值发生变化。目前是建模的时候被改变
    MAX_Y_LEN = 2450 # Y轴最长距离，单位mm
    MAX_X_LEN = 900  # x轴最长距离，单位mm
    RUN_MORE_DIS = 50# 控制y轴取整后多走，目的是使它拍多不到一张。原因电机跟相机分辨率不一样，所以导致电机运动2000mm，不一定拍摄20000行，所以多走一点，让他凑不够一张但能保证之前的图是够的

    #BASE_Z = -109.0  # 宁德z轴基准值
    BASE_Z = -128 #溧阳z轴基准值

    RATE_Z = 1.0 / 800  # Z轴分辨率，mm/_rate 转换为发送脉冲

LOCK_STYLESHEET = """QToolButton#toolButton_userlevel\n 
{ border-image: url(res/lock.png); }  \n 
QToolButton:hover#toolButton_userlevel\n 
{background-color:transparent;border:0px;\n
 background-color:lightBlue;\n}\n"""

UNLOCK_STYLESHEET = """QToolButton#toolButton_userlevel\n 
{ border-image: url(res/unlock.png); }  \n 
QToolButton:hover#toolButton_userlevel\n 
{background-color:transparent;border:0px;\n
 background-color:lightBlue;\n}\n"""
