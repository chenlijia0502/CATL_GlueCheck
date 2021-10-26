# -*- coding: utf-8 -*-
from pyqtgraph.Qt import QtGui, QtCore

from library.monitoring.Display_interface.subwidget import *
from PyQt5.QtCore import Qt
from library.monitoring.Display_interface.BASEMONI import BASEMONI


import cv2


MSG_LEARN_DEFECT = 1006  # 单个缺陷学习
MSG_LEARN_ONE = 1008  # 单张学习





class StatisticsInterface(QtGui.QWidget,BASEMONI):
    """
    统计界面。
    对于kxshow输入的数据，如果希望最佳显示，那么缺陷字典里必须包含"Dots"、"Energy"、"defect name"、 "pos"
    如果输入是好卡，则需输入图片，其余参数默认。

    """
    sigslotlearn = QtCore.pyqtSignal(dict)
    def __init__(self, h_parentwidget=None, nmaxindex = 100):
        super(StatisticsInterface, self).__init__()
        self.dict_save = {}  # 保存字典  {id：{整个字典}}
        self.list_image = [] #保存图片
        self.list_quality = [] #检测结果（判定）
        self.n_maxindex = nmaxindex  # 保存字典的个数
        self.dict_current = {}  # 保存当前字典
        self.id=0
        h_widget = self.__makewidget__()
        self.h_parentwidget = h_parentwidget
        h_layout = QtGui.QHBoxLayout()
        h_layout.addWidget(h_widget)
        self.setLayout(h_layout)

    def kxshow(self, image=None,quality=0, list_dictfeature=[], *args):
        """

        :param image:               输入图像
        :quality:    传入图像是好品还是坏品
        :param list_dictfeature:    列表-字典，每个字典代表一个缺陷的特征参数(好品则不输入)，如下
        exp:[{"defect name":"污渍", "Dots": 100, "Energy": 100, "pos": [[左上角x, 左上角y]，[宽, 高]], .......}, {...}....]
        :param *args:               继承该类的界面会使用到的参数，根据各个界面不同使用
        :return:
        """

        self.dict_save[self.id % self.n_maxindex]=list_dictfeature
        if self.id < self.n_maxindex:
            self.list_image.append(image)
            self.list_quality.append(quality)
        else:
            self.list_image[self.id % self.n_maxindex] = image
            self.list_quality[self.id % self.n_maxindex] = quality
        self.h_datamonitor.numcalc(quality)
        self.h_datalist.addmyrow(self.id,list_dictfeature)
        self._updateUI(quality, image, list_dictfeature)
        self.id+=1

    def _updateUI(self, quality, image, list_dictfeature):
        self.h_featureList.appendData(self.__getarr(list_dictfeature))
        self._showimage(quality, image, list_dictfeature)


    def kxclear(self):
        """ 清除数据 """
        self.dict_save = {}  # 保存字典  {id：{整个字典}}
        self.dict_current = {}  # 保存当前字典
        self.list_image = []
        self.id = 0
        self.h_datalist.kxclear()
        self.h_imagelist.clear()
        self.h_featureList.clear()
        self.h_datamonitor.clear()
        self.h_imagedisplay.clear()

    def __makewidget__(self):
        """
                    构建界面的方法，通过重写该方法实现界面修改
        """
        h_inwidegt = QtGui.QWidget()

        # self.h_datamonitor = DataMonitorWidget({"统计":["总数", "好品", "坏品", "好品率"]})
        self.h_datamonitor = DataMonitorWidget({u"统计": \
                                                    [str(self.tr("Total")), str(self.tr("Good")), str(self.tr("Bad")),
                                                     str(self.tr("Ratio"))]})

        self.h_imagelist = ImageListWidget()

        self.h_imagedisplay = ImageDisplayWidget()
        # self.h_imagedisplay.sigslotlearn.connect(self.sigslotlearn)

        self.h_datalist = DataListWidget(self.dict_save, [(u"序号", int), (u"缺陷类型", object)])
        self.h_datalist.setindexmax(self.n_maxindex)
        self.h_datalist.cellClicked.connect(self._clickslot)
        self.h_datalist.Sigkeypress.connect(self._clickslot)

        self.h_featureList = FeatureList()#特征列表
        self.h_tabwidget = QtGui.QTabWidget()

        self.h_tabwidget.addTab(self.h_datalist, self.tr("缺陷列表"))
        self.h_tabwidget.addTab(self.h_featureList, self.tr("特征列表"))

        self.MAX_ROW_NUM = 16
        self.n_currentrow = 0

        h_grid = QtGui.QGridLayout()
        h_grid.setColumnStretch(0, 3)
        h_grid.setColumnStretch(1, 3)
        h_grid.setColumnStretch(2, 3)
        h_grid.setRowStretch(0, 2)
        h_grid.setRowStretch(1, 3)
        h_grid.setRowStretch(2, 3)
        h_grid.addWidget(self.h_datamonitor, 0, 0, 1, 1)
        h_grid.addWidget(self.h_tabwidget, 1, 0, 2, 1)
        h_grid.addWidget(self.h_imagedisplay, 0, 1, 2, 2)
        h_grid.addWidget(self.h_imagelist, 2, 1, 2, 2)

        h_widget = QtGui.QWidget()
        h_layout = QtGui.QHBoxLayout()

        h_widget.setLayout(h_layout)

        h_grid.addWidget(h_widget, 2, 2, 1, 1)
        h_inwidegt.setLayout(h_grid)

        return h_inwidegt

    def _clickslot(self, *arg):
        """
        缺陷列表选中某项，更新
        Returns
        -------

        """
        nindex = arg[0]
        if nindex in self.dict_save:
            self._updateUI(self.list_quality[nindex], self.list_image[nindex], self.dict_save[nindex])

    def _showimage(self,quality, image, list_dictfeature):
        """
            将数据传送到各个窗口和表单
        :param quality 好坏品
        :param image: 卡片图像
        :param list_dictfeature: 缺陷数据
        :return:
        """
        self.h_imagedisplay.getdict(quality, image, list_dictfeature)
        self.h_imagelist.clear()
        list_rois = []
        list_defectword = []
        for i, feature in enumerate(list_dictfeature):
            rois = feature.get("pos", (0, 0, 0, 0))
            list_rois.append(rois)
            list_defectword.append('Dots: ' + str(feature.get("Dots", '-1')) + ',' + 'Energy: ' + str(feature.get("Energy", '-1')))
        self.h_imagelist.setdata(image, list_defectword, list_rois)

    def __getarr(self, list_dict_data):  # list_dict = [{},{},{},{}]
        """
        将数据转换成表单能直接接收的格式
        :param list_dict: 要转化的数据
        :return:
        """
        list_return = []
        self.h_featureList.setRowCount(0)
        for dict_data in list_dict_data:
            for key in dict_data.keys():
                list_return.append((key, str(dict_data[key])))

        return np.array(list_return)

    def sendmsg(self, n_stationid, n_msgtype, s_extdata=''):
        '''
        每个界面文件初始化时都设置父窗口成员变量，并在本窗口加此方法，目的是不耦合的给子站发送消息。
        '''
        # MSG_LEARN_DEFECT = 1006   #单个缺陷学习
        # MSG_LEARN_ONE = 1008  #单张学习
        self.h_parentwidget.sendmsg(n_stationid, n_msgtype, s_extdata)

    def mousePressEvent(self, even):
        if even.button() == QtCore.Qt.RightButton:
            self.h_imagedisplay.autorangeview()

def demo():
    import numpy as np
    app = QtGui.QApplication([])
    exm = StatisticsInterface()
    image = cv2.imdecode(np.fromfile("1.bmp", dtype=np.uint8), 1)
    # exm.kxshow(image,1,[{"缺陷名":"污渍", "点数": 100, "能量": 100, "pos": [100, 100,40, 60]}])
    image = cv2.imdecode(np.fromfile("2.bmp", dtype=np.uint8), 1)
    # exm.kxshow(image,1,[{"缺陷名":"划痕", "点数": 100, "能量": 100, "pos": [ 200, 200,100, 160]},{"缺陷名":"污渍", "点数": 100, "能量": 100, "pos": [100, 100,40, 60]}])
    # exm.kxshow(image,0)
    # exm.clear()
    exm.kxshow(image,1,[{"缺陷名":"划痕", "点数": 100, "能量": 100, "pos": [1811, 985, 5, 22]},{"缺陷名":"污渍", "点数": 100, "能量": 100, "pos": [100, 100,40, 60]}])
    # exm.kxshow(image,0,[])

    exm.show()

    app.exec_()


if __name__ == '__main__':
    import struct

    demo()
