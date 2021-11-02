#coding:utf-8
"""
    date:   2019.08.12
    author: HYH
    fun:    这个类放置一些全局参数,甚至可以把中英文放于这,不至于代码中穿插着中文
"""


class ClassID:
    WORD = u"工位"
    HEAD = ['A', 'B', 'C', 'D', 'E', 'F', 'G', 'H']

class GlobalQuality: #判断质量好坏的标准
    GOOD = 0
    BAD = 1
    BAD_ALARM = 2#报警
    BAD_MARK = 3#贴标
    BAD_ALARM_AND_MARK = 4#贴标并报警

class BYD_DEFECT_TYPE:#比亚迪毛刺缺陷类型
    TYPE = {u'断':0,  u'毛刺':1,  u'拱':2,  u'铝丝铝粉':3, u'白线比例':4, u'极片厚度':5, u'异常':6}


class BYD_DEFECT_JUST_BAD_MARKID:#比亚迪缺陷判废标志位的值
    #检查参数第八位作为标志位(0是特指断的区域，1是线上的，2是其它区域，3是指极片厚度，4是指白线比例)
    BROKEN_ID = 0
    WHITELINE_ID = 1
    ANOTHERAREA_ID = 2
    POLEPIECEHEIGHT_ID = 3
    WHITELINE_RATE = 4



class GlobalRealTimeinfo: #实时监测存于内存中的数据的最长长度，同时也是一卷之中能存下来的最大缺陷值
    MAX_LEN_INFO = 1000

# class DataAnalysePath: #数据分析保存的数据一些关键的路径
#     PATH_DATADIR = 'd:\\data\\'
#     SLISTNAME = 'list.txt'
#     EXCELNAME = 'data.xlsx'
#     PATH_INSERTIMAGE = 'd:\\insert.bmp'
#     HIST3 =  u'd:\\每种缺陷数量柱状图.jpg'
#     HIST1 = u'd:\\缺陷趋势图卷.jpg'
#     HIST2 = u'd:\\缺陷趋势图天.jpg'

class FeatureType: #特征类型
    TYPE =[u'点数', u'能量', u'X坐标', u'Y坐标', u'缺陷宽', u'缺陷高', u'中心占比率', u'物理高度', u'标志位', u'物理宽度']

class ChineseWord:
    STARTCHECK = u"开始检测"
    STOPCHECK = u"停止检测"
    STARTOFFLINECHECK = u"开始离线检测"
    STOPOFFLINECHECK = u"停止离线检测"
    PRODUCER = u'操作员'
    OPERATOR = u'ME工程师'
    IMD = "IMD解决方案工程师"
    MANAGER = u'管理员'
    CHANGEPASSWORD = u'修改密码'

    # DEFECTNAME = u'缺陷名'
    # DEFECTALL = u'缺陷总数'
    # DETAIL = u'详细'
    # QUALITY = u'品质'
    # ID = u'ID'
    # ALL = u'全部'
    # STARTTIMEERROR = u'起始时间错误'
    # ROLLNUMBER = u'卷号'
    # SHULIANG = u'数量'
    # CAMERAPOS = u'相机位'
    # LISTCAMERAPOSTYPE = [u'一', u'二', u'三', u'四', u'五', u'六', u'七', u'八']
    # DEFECTIMAGE = u'缺陷图'
    # OCCUPYERROR = u'EXCEL文件被占用，请先将占用的文件释放或关闭'
    # DATAERROR = u'当前载入的数据中有缺失问题，请重新选择卷或日期'
    # BIGGESTDATA = u'当前缺陷总量最大值为'
    # KNIFECHECK = u'刀片检查'
    # MEASURENUM = u'测量值'
    # REALTIME = u'实时图像'
    # TAPPOS = u'位置'
    # FREEZE = u"冻结"
    # CAMERAENABLE = u"相机使能选择"
    # DATE = u'日期'

    # EXPORTDATA = u"导出数据中...."
    # LOADDATA = u'请等待! 数据加载中....'

class ColorType16H:
    TYPE = ['#FF3030', '#EEEE00', '#C0FF3E', '#7CFC00', '#1C86EE', '#141414', '#00FF00', '#FFC125', '#0000FF', '#FF0000']
    HISTRANGE = [
                # '#FFFAF0', '#FFEFD5', '#FFDAB9', #浅黄
                 '#58AE9D', '#54FF9F', '#00FF00', #绿
                 '#FFEC8B', '#FFD700', '#FFC125', #橙
                 '#B0E2FF', '#00BFFF', '#0000FF', #蓝
                 '#FF6EB4', '#FF6347', '#FF0000', #红
                 ]

class DataPipeLineType:
    TYPE_GETT = 1
    TYPE_ADJUSTCAMERA = 2#设置调焦参数
    TYPE_GETDELAYPARAM = 3#获取贴标延时
    TYPE_GETREPORTTYPE = 4#获取报错方式
    TYPE_GETEXPOSURETIME = 5#获取曝光参数
    LIST_GLOBALPARAM_TYPE = [TYPE_GETT, TYPE_ADJUSTCAMERA, TYPE_GETDELAYPARAM, TYPE_GETREPORTTYPE, TYPE_GETEXPOSURETIME]

    TYPE_COPYPARAMALL = 11
    TYPE_COPYPARAMALLB = 14
    LIST_COPYSETTING = [TYPE_COPYPARAMALL, TYPE_COPYPARAMALLB]
    TYPE_COPYPARAMALL_DOB = 12
    TYPE_COPYPARAMALL_DOA = 13



class PermissionLevel:
    PRODUCER = 0
    #OPERATOR = 1
    MANAGER = 1 #等级最高

class LogInfo:#日志部分重要参数
    MAX_MSG_NUM = 500
    PATH_SAVE_LOG = "d:\\log\\"