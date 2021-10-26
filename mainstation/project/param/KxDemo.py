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
from UI.ui_kxglobal import Ui_ParamPYWidget
from pyqtgraph.parametertree import ParameterTree

class KxDemo(KxBaseParamWidget):
    """
              测试类
    """
    def __init__(self, h_parentwidget, n_uid, n_areanum, n_stationid):
        KxBaseParamWidget.__init__(self,n_uid, n_areanum, n_stationid)
        self.h_parent = h_parentwidget
        self.ui = Ui_ParamPYWidget()
        self.ui.setupUi(self)
        self._initui()
        # use for save image
        self.params.extend([
            {'name': u'检查设置', 'type': 'group', 'children': [
                {'name': u'二值化阈值', 'type':  'int', 'value': 30, 'step': 1, 'limits': (0, 255)},]
            },
        ])
        self.p = KxParameter.create(name='params', type='group', children=self.params)
        self.h_parameterTree.setParameters(self.p, showTop=False)
        self.ui.h_pBtSave.clicked.connect(self.saveparameters)
        self.ui.h_pBtCancel.clicked.connect(self.cancelparameter)

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
        self.img = pg.ImageItem()
        self.view.addItem(self.img)

    def recmsg(self, n_stationid, n_msgtype, tuple_data):
        '''
        接收子站发送过来的消息
        '''   
        # import imc_msg
        # if n_msgtype == imc_msg.MSG_SEND_REAL_TIME_IMAGE:
        #     self.ReceiveImages(tuple_data[0])




registerkxwidget(name='KxDemo', cls=KxDemo, override=True)


if __name__ == '__main__':
    app = QtWidgets.QApplication([])
    w = KxDemo(None, 1, 2 ,3)
    w.show() 
    app.exec_()       