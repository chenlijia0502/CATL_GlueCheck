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
from PIL import Image
# from pyqtgraph.imageview.ImageView import ImageView


class GuleParam(KxBaseParamWidget):
    """
              涂胶项目
    """
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
        self.params.extend([
            {'name': u'主站设置', 'type': 'group', 'visible':False, 'children': [
                {'name': u'图像信息', 'type': 'imageinfo',
                 'value': {"isShow": True}, "infovisible": True},
                {'name': '底板路径', 'type':'str'},
            ]},
            {'name': '相机信息', 'type': 'group', 'visible':False, 'children':[
                {'name': '图像宽度', 'type': 'str', 'value': '2448'},
                {'name': '图像高度', 'type': 'str', 'value': '2048'},
                {'name': '纵向分辨率', 'type': 'str'},
            ]},
            {'name': '拍摄控制', 'type': 'group', 'children':[
                {'name': '横向拍摄长度', 'type': 'int', 'value': 200, 'limits': [1, 5000]},
                {'name': '纵向拍摄长度', 'type': 'int', 'value': 200, 'limits': [1, 5000]},
                {'name': '横向拍摄张数', 'type': 'int', 'value': 5, 'limits': [1, 500]},
                {'name': '纵向拍摄张数', 'type': 'int', 'value': 5, 'limits': [1, 500]},
                {'name': '横向重叠行数', 'type': 'int', 'value': 0, 'limits': [1, 5000], 'visible':False},
                {'name': '纵向重叠行数', 'type': 'int', 'value': 0, 'limits': [1, 5000], 'visible':False}
            ]},

            {'name': '检测设置', 'type': 'group', 'children': [
                {'name': '提取异物灰度', 'type': 'int', 'value': 100, 'limits': [0, 255]},
                {'name': '异物最小点数', 'type': 'int', 'value': 20, 'limits': [0, 20000]},

            ]},
            {'name': '平面度设置', 'type': 'group', 'children': [
                {'name': '最大方差设置', 'type': 'int', 'value': 100, 'limits': [0, 255]},

            ]},
        ])
        self.appendqualityinspectionstandards(self.params)
        self.p = KxParameter.create(name='params', type='group', children=self.params)
        self.h_parameterTree.setParameters(self.p, showTop=False)
        self.p.param(u'主站设置', u'图像信息').add2view(self.ui.h_gVShowRealImg, self.h_imgitem)
        self.p.param('质量检查标准', '检查标准组数').sigValueChanged.connect(self._addqualdetectslot)

    def _addqualdetectslot(self, *even):
        if int(even[1]) > self.n_qualitytreenum:
            for n_i in range(int(even[1])):
                self.p.param('质量检查标准','质量检查标准' + str(n_i)).show()
        else:
            for n_i in range(int(even[1]), self.n_qualitytreenum):
                self.p.param('质量检查标准', '质量检查标准' + str(n_i)).hide()
        self.n_qualitytreenum = int(even[1])


    def recmsg(self, n_stationid, n_msgtype, tuple_data):
        '''
        接收子站发送过来的消息
        '''
        # import imc_msg
        # if n_msgtype == imc_msg.MSG_SEND_REAL_TIME_IMAGE:
        #     self.ReceiveImages(tuple_data[0])

    def appendqualityinspectionstandards(self, dict_params):
        '''
        判废标准模块
        '''
        # tip 取决子站主站共同定义的json文件
        s_tip = u'x[0]-点数\nx[1]-能量\nx[2]-左上X\nx[3]-左上Y\nx[4]-缺陷宽\nx[5]-缺陷高\n'
        self.n_maxstandardnum = 12
        list_standardschildrenitems = [
            {'name': '检查标准组数', 'type': 'int', 'value': self.n_qualitytreenum, 'step': 1,
             'limits': (0, self.n_maxstandardnum)}
        ]
        for i in range(0, self.n_maxstandardnum):
            list_standardschildrenitems.append(
            {'name': '质量检查标准' + str(i), 'type': 'group', 'expanded': False, 'visible': False, 'children': [
            {'name': '缺陷名', 'type': 'str', 'value': 'defectErr', 'visible': True},
            # {'name': '判废数', 'type': 'int', 'value': 1, 'step': 1, 'limits': (0, 4), 'visible': True},
            {'name': '表达式', 'type': 'kxtext', 'value': 'x[1]>100', 'tip': s_tip, 'visible': True},]}
        )


        dict_standardsitem = {'name': '质量检查标准', 'type': 'group',
                              'children': list_standardschildrenitems}
        dict_params.append(dict_standardsitem)


registerkxwidget(name='GuleParam', cls=GuleParam, override=True)


if __name__ == '__main__':
    app = QtWidgets.QApplication([])
    w = GuleParam(None, 1, 2 ,3)
    w.show()
    app.exec_()