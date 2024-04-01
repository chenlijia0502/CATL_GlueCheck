#coding=utf-8

import sys
import os
import threading
import time
import imc_msg
from PyQt5 import QtGui, QtCore, QtWidgets
from library.parametersetting.ParamItem import ParamItem
from library.parametersetting.ModelManage.ModelManage import ModelManage
from library.common.globalparam import ImportantName
from library.ipc import ipc_tool
from library.common.globalparam import tabWidgetStyle

class KxBaseParameterSetting(QtWidgets.QWidget):
    """
    Date:       2017.07 —— 2020.02
    Author:     HYH
    Desc:       参数设置类，主要管控模板类跟站点类，对这两种类起中介作用，
                站点类可能有多个
    """
    sig_params = QtCore.pyqtSignal(int, int, object)#参数界面
    def __init__(self, hparent, dict_config):
        super(KxBaseParameterSetting, self).__init__()
        self.h_parent = hparent
        self.sig_params.connect(self.recmsg)
        self.thread_refreshinterface = _RefreshInterface_Param(self)
        self.thread_refreshinterface.start()
        self.h_verticallayout = QtWidgets.QVBoxLayout(self)
        self.h_verticallayout.setContentsMargins(10, 10, 0, 0)
        self.h_verticallayout.setSpacing(0)
        self.h_verticallayout.setObjectName("h_verticallayout")
        self.h_tabwidget = QtWidgets.QTabWidget(self)
        self.h_tabwidget.setStyleSheet(tabWidgetStyle)
        self.h_verticallayout.addWidget(self.h_tabwidget)
        self.h_modelmanagewidget = ModelManage(dict_config)
        self.h_tabwidget.addTab(self.h_modelmanagewidget, self.tr("参数集"))
        self.list_paramitemwidget = []#存放ParamItem类

        self.dict_sysparams = dict_config
        # if self.h_modelmanagewidget.getcurmodelnum() != 0:
        self._updateparamitem()
        self.h_modelmanagewidget.h_paramchanges.connect(self._hideparamitemwidget)
        self.h_modelmanagewidget.setfirst()# 设置打开显示第一个界面

    
    def _updateparamitem(self):
        '''
        根据工位数重新初始化参数项
        '''
        if self.dict_sysparams is not None:
            self.n_substationcount = len(self.dict_sysparams[u'substation'] )
            for stationid in range(self.n_substationcount):

                if int(self.dict_sysparams[u'istranslate']):
                    s_stationname = ImportantName.STATION_CHINESE + str(stationid + 1)
                else:
                    s_stationname = ImportantName.STATION + str(stationid + 1)
                paramitemwidget = ParamItem(self, stationid, self.dict_sysparams)
                self.list_paramitemwidget.append(paramitemwidget)
                self.h_tabwidget.addTab(paramitemwidget, s_stationname)
                self.h_modelmanagewidget.h_paramchanges.connect(paramitemwidget.updateparam)
            self.h_tabwidget.setCurrentWidget(self.list_paramitemwidget[0])

    def _hideparamitemwidget(self, even0, even1):
        """
        第一次打开程序的时候理论没有模板文件夹，这个时候不希望出现参数界面。
        当后续模板管理类继续操作，这个信号还是会被触发，所以判断当界面只存在
        一个模板管理界面是才需要更新，也即只操作一次
        """
        if even1 is False:
            pass
        else:
            if self.h_tabwidget.count() == 1:
                self._updateparamitem()

    def getcurmodelposition(self):
        """检查时需要获取当前的模板路径，好让子站读取模板"""
        list_curmodelxmlposition = []
        for paramitemwidget in self.list_paramitemwidget:
            list_curmodelxmlposition.append(paramitemwidget.getpositionofmodel())
        return list_curmodelxmlposition
 

        
    def sendmsg(self, n_stationid, n_msgtype, s_extdata=b''):
        '''
        每个界面文件初始化时都设置父窗口成员变量，并在本窗口加此方法，目的是不耦合的给子站发送消息。
        '''
        self.h_parent.sendmsg(n_stationid, n_msgtype, s_extdata)
    
    def recmsg(self, n_stationid, n_msgtype, tuple_data):
        '''
        接收子站发送过来的消息
        '''   
        import imc_msg
        self.list_paramitemwidget[n_stationid].recmsg(n_stationid, n_msgtype, tuple_data)
    

    def lock(self):
        #self.h_modelmanagewidget.lock()
        for paramitemwidget in self.list_paramitemwidget:
            if paramitemwidget is not None:
                paramitemwidget.lock()

    def unlock(self):
        #self.h_modelmanagewidget.unlock()
        for paramitemwidget in self.list_paramitemwidget:
            if paramitemwidget is not None:
                paramitemwidget.unlock()

    def modelmanage_load_lock(self):
        self.h_modelmanagewidget.lock()

    def modelmanage_load_unlock(self):
        self.h_modelmanagewidget.unlock()

    def str2paramitemfun(self, nstationid, nuid, strfun, *param):
        """
        通过输入函数名str来进行调用参数集界面的函数，实现ParameterSetting对ParamItem接口的开发
        Parameters
        ----------
        nstationid  站点号
        nuid        界面号
        strfun      函数名，不带括号 '( )'
        *param      调用函数时输入的参数，可以有多个


        Returns     调用函数的结果，若无则返回None
        -------

        """
        if nstationid < len(self.list_paramitemwidget):
            return self.list_paramitemwidget[nstationid].str2fun(nuid, strfun, *param)
        else:
            return None

#QtCore.QThread
class _RefreshInterface_Param(threading.Thread):
    """
    刷新界面线程，不断读取处理数据队列数据，根据标志位，通过信号槽来调用各界面函数
    """
    def __init__(self, mainwindow):
        '''
        初始化，传入处理数据队列
        '''
        super(_RefreshInterface_Param, self).__init__()

        self.queue_processedData_param = ipc_tool.getqueue_processedData_param()
        self.mainwindow = mainwindow

    def run(self):
        """
        线程开始后触发此函数,RefreshInterface线程不直接调用主界面线程函数，发送信号调用，避免一些不必要的警告,
        """
        time.sleep(0.1)  # 为了不出现程序启动时争着写异常列表，导致异常丢失
        while (1):
            if self.queue_processedData_param.empty():
                time.sleep(0.001)
            while (not self.queue_processedData_param.empty()):
                curtuple = self.queue_processedData_param.get()
                if curtuple[1] in imc_msg.list_params_module:
                    self.mainwindow.sig_params.emit(curtuple[0], curtuple[1], curtuple[2])





if __name__ == "__main__":
    import sys
    app = QtGui.QApplication(sys.argv)
    widget = KxParameterSetting()
    widget.show()
    app.exec_()
