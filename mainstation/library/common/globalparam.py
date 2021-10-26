#coding:utf-8
#A~Z,有必要再自己加吧



class GlobalQuality: #判断质量好坏的标准
    GOOD = 0
    BAD = 1
    BAD_ALARM = 2#报警
    BAD_MARK = 3#贴标
    BAD_ALARM_AND_MARK = 4#贴标并报警


class ChineseWord:
    """中文名"""
    DEFECTNAME = u'缺陷名'
    DEFECTALL = u'缺陷总数'
    DETAIL = u'详细'
    QUALITY = u'品质'
    ID = u'ID'
    ALL = u'全部'
    STARTTIMEERROR = u'起始时间错误'
    ROLLNUMBER = u'卷号'
    SHULIANG = u'数量'
    CAMERAPOS = u'相机位'
    LISTCAMERAPOSTYPE = [u'一', u'二', u'三', u'四', u'五', u'六', u'七', u'八']
    DEFECTIMAGE = u'缺陷图'
    OCCUPYERROR = u'EXCEL文件被占用，请先将占用的文件释放或关闭'
    DATAERROR = u'当前载入的数据中有缺失问题，请重新选择卷或日期'
    BIGGESTDATA = u'当前缺陷总量最大值为'
    KNIFECHECK = u'刀片检查'
    MEASURENUM = u'测量值'
    REALTIME = u'实时图像'
    TAPPOS = u'位置'
    FREEZE = u"冻结"
    CAMERAENABLE = u"相机使能选择"
    DATE = u'日期'
    PRODUCER = u'生产员'
    OPERATOR = u'操作员'
    MANAGER = u'管理员'
    CHANGEPASSWORD = u'修改密码'
    EXPORTDATA = u"导出数据中...."
    LOADDATA = u'请等待! 数据加载中....'


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

class ImportantName:
    """所有的文件都是以英文命名"""
    STATION = "station"
    STATION_CHINESE = u"站点"
    XML_MODELCONFIGNAME = "check"
    XML_MODELCONFIGNAME_CH = u"检查"#上面的中文
    XML_GLOBALCONFIGNAME = "global"
    XML_GLOBALCONFIGNAME_CH = u"全局参数"

tabWidgetStyle = '''
                QTabBar::tab { 
                border-color: black; 
                border-width: 3px; 
                border-top-left-radius: 6px; 
                border-top-right-radius: 6px; 
                background:rbg(255, 255, 255); 
                color:black; 
                min-width:30ex; 
                min-height:10ex; 
                font:10pt 'Consolas';
                } 
                QTabBar::tab:selected{ 
                background:#188bd1; 
                color:white; 
                } 
                QTabBar::tab:!selected{ 
                background:none; 
                color:black; 
                } 
                QTabBar::tab:!selected:hover { 
                margin-left: 5px; 
                }
                '''


# tabWidgetStyle = '''
#                 QTabBar::tab {
#                 border-color: black;
#                 border-width: 3px;
#                 border-top-left-radius: 6px;
#                 border-top-right-radius: 6px;
#                 background:lightGray;
#                 color:black;
#                 min-width:30ex;
#                 min-height:10ex;
#                 font:10pt 'Consolas';
#                 }
#                 QTabBar::tab:selected{
#                 background:rgb(69,160,178);
#                 color:white;
#                 }
#                 QTabBar::tab:!selected{
#                 background:none;
#                 color:black;
#                 }
#                 QTabBar::tab:!selected:hover {
#                 margin-left: 5px;
#                 }
#                 '''
