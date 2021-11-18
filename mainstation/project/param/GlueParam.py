# -*- coding: utf-8 -*-
from PyQt5 import QtCore, QtGui
import struct
import sys
#为了文件的读写设置，python2不加这两句保存或者读文件时会出现解码编码问题

sys.path.append("../../../")
import xmltodict
from kxpyqtgraph.kxparameterTree.KxCustomWidget import *
from library.parametersetting.ParamItemPY.KxBaseWidget import KxBaseParamWidget, registerkxwidget
from kxpyqtgraph.kxparameterTree.KxParameter import KxParameter
from UI.ui_kxglobal1 import Ui_ParamPYLoadWidget
from library.ipc import ipc_tool
import imc_msg
# from pyqtgraph.imageview.ImageView import ImageView

#节拍  分析
#



class GuleParam(KxBaseParamWidget):
    """
              涂胶项目
    """
    _MAX_ROI_NUM = 32
    _MAX_SCAN_NUM = 6
    _BUILD_MODEL_SCALE_FACTOR = 4#建模的时候对图像进行压缩，不然会卡顿
    def __init__(self, h_parentwidget, n_uid, n_areanum, n_stationid):
        KxBaseParamWidget.__init__(self,n_uid, n_areanum, n_stationid)
        self.h_parent = h_parentwidget
        self.ui = Ui_ParamPYLoadWidget()
        self.ui.setupUi(self)
        self._initui()
        self._initparam()

    def _initui(self):
        """补充UI，qtdesigner无法一步到位"""
        self.h_parameterTree = ParameterTree(self.ui.widget_param, False)
        self.h_parameterTree.setStyleSheet("QTreeWidget::item{height:30px;}")
        self.verlayout = QtWidgets.QVBoxLayout(self.ui.widget_param)
        self.verlayout.addWidget(self.h_parameterTree)
        self.verlayout.setContentsMargins(0, 0, 0, 0)
        self.verlayout.setSpacing(0)
        self.view = pg.ViewBox(invertY=True, enableMenu=False)
        self.ui.h_gVShowRealImg.setCentralItem(self.view)
        self.view.setAspectLocked(True)
        self.h_imgitem = pg.ImageItem()
        self.view.addItem(self.h_imgitem)
        self.ui.h_pBtSave.clicked.connect(self.saveparameters)

    def _initparam(self):
        self.s_imgpath = None
        self.n_qualitytreenum = 0#当前已显示质量检查组数
        self.n_checkarea = 0#当前检测区域
        self.params.extend([
            {'name': u'主站设置', 'type': 'group', 'visible':False, 'children': [
                {'name': u'图像信息', 'type': 'imageinfo',
                 'value': {"isShow": True}, "infovisible": True},
                {'name': '底板路径', 'type':'str'},
            ]},
            {'name': '全局拍摄控制', 'type': 'group', 'children':[
                {'name': '相机横向像素数', 'type': 'int', 'value': 6000, 'limits': [1, 8192]},
                {'name': '相机横向分辨率', 'type': 'float', 'value': 0.1, 'limits': [0, 1]},
                {'name': '拍摄长度', 'type': 'int', 'value': 1000, 'limits': [0, 3000]},
                {'name': '起拍位置', 'type': 'int', 'value': 0, 'limits':[0, 2000]},
                {'name': '拍摄组数', 'type': 'int', 'value': 3, 'limits': [1, self._MAX_SCAN_NUM]},
                {'name': u'全局取图', 'type': 'action'},
                {'name': u'扫描区域取图', 'type': 'action'},
            ]},
            {'name': u'检测区域数量', 'type': 'int', 'value': 0, 'step': 1, 'limits': (0, self._MAX_ROI_NUM)},
            {'name': u'显示图像', 'type':'list', 'values': {'组数一': 0, '组数二': 1, '组数三':2, '组数四':3,
                                                        '组数五':4, '组数六':5}},

        ])

        self._append_scan_area(self.params)
        self._append_checkarea_param(self.params)
        self.p = KxParameter.create(name='params', type='group', children=self.params)
        self.h_parameterTree.setParameters(self.p, showTop=False)
        self.p.param(u'主站设置', u'图像信息').add2view(self.ui.h_gVShowRealImg, self.h_imgitem)
        for nindex in range(self._MAX_ROI_NUM):
             self.p.param(u'检测区域' + str(nindex), u'检测区域').add2view(self.view)
        for nindex in range(self._MAX_SCAN_NUM):
             self.p.param('扫描区域' , '扫描区域' + str(nindex)).add2view(self.view)
        self._initsignal()

    def _initsignal(self):
        self.p.param('检测区域数量').sigValueChanged.connect(self._add_checkarea)
        self.p.param('全局拍摄控制', '全局取图').sigActivated.connect(self._captureimg)
        self.p.param('全局拍摄控制', '扫描区域取图').sigActivated.connect(self._captureimg_second)


    def _addqualdetectslot(self, *even):
        if int(even[1]) > self.n_qualitytreenum:
            for n_i in range(int(even[1])):
                self.p.param('质量检查标准','质量检查标准' + str(n_i)).show()
        else:
            for n_i in range(int(even[1]), self.n_qualitytreenum):
                self.p.param('质量检查标准', '质量检查标准' + str(n_i)).hide()

        self.n_qualitytreenum = int(even[1])

    def _add_checkarea(self, *even):
        if int(even[1]) > self.n_checkarea:
            for n_i in range(int(even[1])):
                self.p.param('检测区域' + str(n_i)).show()
                self.p.param('检测区域' + str(n_i), '检测区域').isShow(True)
        else:
            for n_i in range(int(even[1]), self.n_checkarea):
                self.p.param('检测区域' + str(n_i)).hide()
                self.p.param('检测区域' + str(n_i), '检测区域').isShow(False)
        self.n_checkarea = int(even[1])

    def _append_checkarea_param(self, dict_params):
        list_standardschildrenitems = []
        for i in range(0, self._MAX_ROI_NUM):
            list_standardschildrenitems.append(
                {'name': '检测区域' + str(i), 'type': 'group', 'expanded': False, 'visible': False, 'children': [
                    {'name': u'检测区域', 'type': 'roiwithtext', 'value': {"isShow": False, "pos": u"0,0,100,100"},
                      "roi_opt": {"word": '检测区域'+ str(i), "scaleable": True, 'pen':3}, "infovisible": False},
                    {'name': u'扫描组号', 'type': 'list', 'values': {'0': 0, '1': 1, '2':2, '3':3, '4':4, '5':5},
                     'value': 0},
                    {'name': u'扫描方向', 'type': 'list', 'values': {'纵向': 0, '横向': 1}, 'value': 0},
                    {'name': '提取异物灰度', 'type': 'int', 'value': 100, 'limits': [0, 255]},
                    {'name': '异物最小点数', 'type': 'int', 'value': 20, 'limits': [0, 20000]},
                ]}

            )
        dict_params.extend(list_standardschildrenitems)

    def _append_scan_area(self, dict_params):

        list_standardschildrenitems = []
        for i in range(0, self._MAX_SCAN_NUM):
            list_standardschildrenitems.append(
                {'name': u'扫描区域'+str(i), 'type': 'roiwithtext', 'value': {"isShow": False, "pos": u"0,0,100,100"},
                 "roi_opt": {"word": '扫描区域'+str(i), "scaleable": True, 'pen': 3}, "infovisible": False},
            )
        dict_scan = {'name': '扫描区域', 'type': 'group', 'visible':False, 'children':list_standardschildrenitems}

        dict_params.append(dict_scan)

    def _captureimg(self):
        """
        全局队列将参数送到界面，控制第一次全局拍照参数
        :return:
        """
        nStartX = int(self.p.param('全局拍摄控制', '起拍位置').value())
        ndisX = int(int(self.p.param('全局拍摄控制', '相机横向像素数').value()) *
                    float(self.p.param('全局拍摄控制', '相机横向分辨率').value()))
        ndisY = int(self.p.param('全局拍摄控制', '拍摄长度').value())
        nXtimes = int(self.p.param('全局拍摄控制', '拍摄列数').value())
        ipc_tool.getqueue_processedData().put((-1, imc_msg.MSG_BUILD_MODEL, [nStartX, ndisX, ndisY, nXtimes]))

    def _captureimg_second(self):
        """
        第二次采集全局参数，根据roi框的位置进行拍摄
        :return:
        """
        nXtimes = int(self.p.param('全局拍摄控制', '拍摄列数').value())
        list_posx = []
        for i in range(nXtimes):
            list_posx.append(self.p.param('扫描区域', '扫描区域' + str(i)).get_list_pos())
        list_x = []
        list_y = []
        nStartX = int(self.p.param('全局拍摄控制', '起拍位置').value())

        for roipos in list_posx:
            list_x.append(int((roipos[0] + roipos[2]) / 2 + nStartX))
            list_y.append(int(roipos[3]))
        ipc_tool.getqueue_processedData().put((-1, imc_msg.MSG_BUILD_MODEL, [list_x, list_y]))

    def recmsg(self, n_stationid, n_msgtype, tuple_data):
        '''
        接收子站发送过来的消息
        '''
        if n_msgtype == imc_msg.MSG_BUILD_MODEL_IMG:
            self._ReceiveBuildModelImg(tuple_data)
        elif n_msgtype == imc_msg.MSG_BUILD_MODEL_IMG_SECOND:
            self._ReceiveBuilModelImgSecond(tuple_data)

    def _ReceiveBuildModelImg(self, tuple_data):
        """
        参考卷绕将图像拼接，并进行显示。而且需要注意当前接收是第几张，因为有个列数问题，按列拼接
        并且把扫描区域ROI显示出来。后面根据这些扫描区域的位置，确定起拍位置
        :param tuple_data:
        :return:
        """
        pass


    def _ReceiveBuilModelImgSecond(self, tuple_data):
        """
        接收图像进行二次建模。且图像根据扫描组数变为N张，可进行界面选择切换
        :param tuple_data:
        :return:
        """
        pass


    def callback2changecol(self):
        print ('callback2changecol')

registerkxwidget(name='GuleParam', cls=GuleParam, override=True)


if __name__ == '__main__':
    app = QtWidgets.QApplication([])
    w = GuleParam(None, 1, 2 ,3)
    w.show()
    app.exec_()