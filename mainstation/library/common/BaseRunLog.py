#coding=utf-8
import os
import time
import logging
import threading
from library.ipc import ipc_tool
import numpy as np
from PyQt5 import QtGui, QtCore, QtWidgets

import imc_msg
from UI.ui_kxrunlog import Ui_RunLog
from library.common.globalparam import LogInfo

"""
    关于logging， 可以输出多份日志（handle），这意味着它可以将结果根据设置的level，记录到不同的日志文件中
"""



class KxBaseRunLog(QtWidgets.QWidget):
    """
    Created on 2017年07月18日
    creator: huber yao
    maintain:  HYH
    显示子站主站运行日志，并分级管理（imc_msg中存在分类等级LOGLEVEL，而logging也自带一套，但两套的标志位是一致，保留
    是因为历史原因）
    日志类两个作用，一个是显示，一个是记录。根据现在涉及的场景，经常还需要显示在状态栏，并且各个子界面、多进程
    都有可能需要记录日志，所以设计成当前模式：有一线程在获取日志队列里来自各个地方的数据，回调本类显示、记录，
    并且判断h_parentwidget是否存在状态栏函数，是则回调
    """
    sig_kxlog = QtCore.pyqtSignal(str, int, str)#stationname, level, sword
    dict_colour = {logging.INFO:QtGui.QBrush(), logging.WARN:QtGui.QBrush(QtCore.Qt.yellow),
                   logging.ERROR:QtGui.QBrush(QtCore.Qt.red)}
    dict_listcolor = {logging.INFO:None, logging.WARN:[255, 255, 0],
                   logging.ERROR:[255, 0, 0]}
    MAX_ROW_NUM = LogInfo.MAX_MSG_NUM
    def __init__(self , h_parentwidget=None):
        '''
        初始化界面
        '''
        
        super(KxBaseRunLog, self).__init__()
        self.ui = Ui_RunLog()
        self.ui.setupUi(self)
        self.h_parentwidget = h_parentwidget
        self.sig_kxlog.connect(self.reclogmsg)
        self.thread_refresh = _RefreshInterface_kxlog(self)
        self.thread_refresh.start()

        self.ui.h_tableWidget.horizontalHeader().setStretchLastSection(True)  #不留空后
        self.ui.h_tableWidget.setAlternatingRowColors(True)  #颜色交替
        self.ui.h_tableWidget.setSortingEnabled(False)
        self.ui.h_tableWidget.resizeColumnsToContents()
        self.dict_errlevel = {logging.INFO:self.tr(u'Normal message'), logging.WARN:self.tr(u'Warn'),
                              logging.ERROR:self.tr(u'Severe exception')}
        self.n_currentrow = 0
        self.s_savelogbuf = ""
        self.s_lastbadlog = ""
        self.s_operatorid = "NONE"


        
    def addonelinelog(self, s_stationname, tuple_data):
        """
        添加一行日志到界面上

        Parameters
        ----------
        s_stationname : str
            站点名称，准确的说应该是报错源
        tuple_data : list/tuple
            错误数据，data[0]: 错误标志位， data[1]:时间， data[2]:信息
        """
        n_errlevel = tuple_data[0]
        time = tuple_data[1]
        exceptionInf = tuple_data[2]
        tuple_showdata = [(self.dict_errlevel[n_errlevel], s_stationname, time, exceptionInf)]
        #self.list_showdata.append(tuple_showdata)
        data = np.array(tuple_showdata, dtype=[(str(self.tr(u'Message level')), object), (str(self.tr(u'Exception object')), object), (str(self.tr(u'time')), object), (str(self.tr(u'information')), object)])        
#         self.ui.h_tableWidget.setData(data)        
        self.setrowdata(self.n_currentrow%self.MAX_ROW_NUM, data)
        if n_errlevel in [imc_msg.LOGLEVEL.WARN, imc_msg.LOGLEVEL.ERR]:
            item = self.ui.h_tableWidget.item(self.n_currentrow%self.MAX_ROW_NUM, 0)#QtGui.QTableWidgetItem()
            item.setBackground(self.dict_colour[n_errlevel])
        self.ui.h_tableWidget.resizeColumnsToContents()
        self.ui.h_tableWidget.setSelectionBehavior(QtWidgets.QAbstractItemView.SelectRows)
        self.ui.h_tableWidget.selectRow(self.n_currentrow%self.MAX_ROW_NUM)
        self.ui.h_tableWidget.horizontalHeader().setStretchLastSection(True)  #不留空后
        self.n_currentrow += 1
        # self.savebadlog(n_errlevel, tuple_showdata[0])
        # self.savelog(tuple_showdata[0])


    def setrowdata(self, row, data):
        startRow = row
        fn0, header0 = self.ui.h_tableWidget.iteratorFn(data)
        if fn0 is None:
            self.ui.h_tableWidget.clear()
            return
        it0 = fn0(data)
        try:
            first = next(it0)
        except StopIteration:
            return
        fn1, header1 = self.ui.h_tableWidget.iteratorFn(first)
        if fn1 is None:
            self.ui.h_tableWidget.clear()
            return
        
        firstVals = [x for x in fn1(first)]
        self.ui.h_tableWidget.setColumnCount(len(firstVals))
        
        if not self.ui.h_tableWidget.verticalHeadersSet and header0 is not None:
            labels = [self.ui.h_tableWidget.verticalHeaderItem(i).text() for i in range(self.ui.h_tableWidget.rowCount())]
            self.ui.h_tableWidget.setRowCount(startRow + len(header0))
            self.ui.h_tableWidget.setVerticalHeaderLabels(labels + header0)
            self.ui.h_tableWidget.verticalHeadersSet = True
        if not self.ui.h_tableWidget.horizontalHeadersSet and header1 is not None:
            self.ui.h_tableWidget.setHorizontalHeaderLabels(header1)
            self.ui.h_tableWidget.horizontalHeadersSet = True
        
        i = startRow
        self.ui.h_tableWidget.setRow(i, firstVals)
        for row in it0:
            i += 1
            self.ui.h_tableWidget.setRow(i, [x for x in fn1(row)])
            
        if self.ui.h_tableWidget._sorting and self.ui.h_tableWidget.horizontalHeader().sortIndicatorSection() >= self.ui.h_tableWidget.columnCount():
            self.ui.h_tableWidget.sortByColumn(0, QtCore.Qt.AscendingOrder)

    def reclogmsg(self, nstationname, level, sdata):
        """
        信号槽，当线程中公共队列收到数据则传到这里，避免重复保存，但状态栏需要保证实时状态
        """
        scurtime = time.strftime('%X', time.localtime(time.time()))
        self.addonelinelog(nstationname, (level, scurtime, sdata))
        if self.s_lastbadlog != nstationname + " " + sdata:#对于重复出现的日志不保存
            self.s_lastbadlog = nstationname + " " + sdata
            savelog = self.s_operatorid + "  " + scurtime + " " + nstationname + " " + sdata
            logging.log(level, savelog)
        if hasattr(self.h_parentwidget, "CallbackTip") and level != logging.INFO:
            self.h_parentwidget.CallbackTip(self.s_lastbadlog, self.dict_listcolor[level])

    def setid(self, id):
        self.s_operatorid = id

#QtCore.QThread
class _RefreshInterface_kxlog(threading.Thread):
    """
    刷新界面线程，不断读取处理数据队列数据，根据标志位，通过信号槽来调用各界面函数
    """
    def __init__(self, mainwindow):
        '''
        初始化，传入处理数据队列
        '''
        super(_RefreshInterface_kxlog, self).__init__()

        self.queue_kxlog = ipc_tool.getqueue_queue_kxlog(self)
        self.mainwindow = mainwindow

    def run(self):
        """
        线程开始后触发此函数,RefreshInterface线程不直接调用主界面线程函数，发送信号调用，避免一些不必要的警告,
        """
        time.sleep(0.1)  # 为了不出现程序启动时争着写异常列表，导致异常丢失
        while (1):
            if self.queue_kxlog.empty():
                time.sleep(0.001)
            while (not self.queue_kxlog.empty()):
                curtuple = self.queue_kxlog.get()
                # if curtuple[1] == imc_msg.GlobalMsgRec.MSG_LOG:
                self.mainwindow.sig_kxlog.emit(curtuple[0], curtuple[1], curtuple[2])


if __name__ == '__main__':
    app = QtWidgets.QApplication([])
    exm = KxBaseRunLog()
    exm.show()
    app.exec_()

