#coding:utf-8
from PyQt5 import QtGui, QtCore, QtWidgets
import threading
from library.ipc import ipc_tool
import time
import imc_msg
from library.common.globalfun import ischildof





KXMOINTOR_TYPES = {}
KXMOINTOR_NAMES = {}
def registerkxmointorwidget(name, cls, override=False):
    """将类的字符名跟类放入公共字典，目的是实例化时可以根据字符串来实例化该类，快速开发界面"""
    global KXMOINTOR_TYPES
    if name in KXMOINTOR_TYPES and not override:
        raise Exception("Mointor type '%s' already exists (use override=True to replace)" % name)
    if not ischildof(cls, KxBaseMonitoringWidget):
        raise Exception(name + u"非KxBaseMonitoringWidget的继承类")
    KXMOINTOR_TYPES[name] = cls
    KXMOINTOR_NAMES[cls] = name


class KxBaseMonitoringWidget(QtWidgets.QWidget):
    """
        实时界面基类(也即监控界面)
    """
    sig_monitoring = QtCore.pyqtSignal(int, int, object)#监控相关数据
    def __init__(self, h_parent):
        super(KxBaseMonitoringWidget, self).__init__()
        self.h_parentwidget = h_parent
        self.sig_monitoring.connect(self.recmsg)
        self.thread_refreshinterface = _RefreshInterface_Monitoring(self)
        self.thread_refreshinterface.start()

    @staticmethod
    def create(**kwargs):
        s_classname = kwargs.get('name', None)
        if s_classname is None:
            return
        else:
            cls = KXMOINTOR_TYPES[s_classname]
        return cls(kwargs.get('h_parent', None))

    def run(self, dict_ex):
        """
        调用run（dict_ex）方法刷新数据， dict_ex为当前字典
        """
        pass

    def setstopflag(self, bool_stopflag):
        """
         停止标志设置
        """
        pass

    def clear(self):
        """
         清空重置
        """
        pass

    def sendmsg(self, n_stationid, n_msgtype, s_extdata=''):
        '''
        每个界面文件初始化时都设置父窗口成员变量，并在本窗口加此方法，目的是不耦合的给子站发送消息。
        '''
        self.h_parentwidget.sendmsg(n_stationid, n_msgtype, s_extdata)

    def recmsg(self, n_stationid, n_msgtype, tuple_data):
        '''
        接收子站发送过来的消息
        '''
        pass



#QtCore.QThread
class _RefreshInterface_Monitoring(threading.Thread):
    """
    刷新界面线程，不断读取处理数据队列数据，根据标志位，通过信号槽来调用各界面函数
    """
    def __init__(self, mainwindow):
        '''
        初始化，传入处理数据队列
        '''
        super(_RefreshInterface_Monitoring, self).__init__()

        self.queue_processedData = ipc_tool.getqueue_processedData_monitoring()
        self.mainwindow = mainwindow

    def run(self):
        """
        线程开始后触发此函数,RefreshInterface线程不直接调用主界面线程函数，发送信号调用，避免一些不必要的警告,
        开始不断根据共享内存标志位，改变状态栏对应位置颜色，表明各子站与硬件是否连接成功
        """
        time.sleep(0.1)  # 为了不出现程序启动时争着写异常列表，导致异常丢失
        while (1):
            if self.queue_processedData.empty():
                time.sleep(0.001)
            while (not self.queue_processedData.empty()):
                curtuple = self.queue_processedData.get()
                if curtuple[1] in imc_msg.list_monitoring_module:
                    self.mainwindow.sig_monitoring.emit(curtuple[0], curtuple[1], curtuple[2])
