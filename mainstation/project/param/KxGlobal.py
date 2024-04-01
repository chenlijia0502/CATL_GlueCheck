# -*- coding: utf-8 -*-
from PyQt5 import QtCore, QtGui
import struct
import sys
#为了文件的读写设置，python2不加这两句保存或者读文件时会出现解码编码问题
sys.path.append("../../../")
import xmltodict
from kxpyqtgraph.kxparameterTree.KxCustomWidget import *
from library.parametersetting.ParamItemPY.KxBaseWidget import KxBaseParamWidget, registerkxwidget
from UI.ui_kxglobal import Ui_ParamPYWidget
from pyqtgraph.parametertree import ParameterTree
from PIL import Image
import cv2



class KxGlobal(KxBaseParamWidget):
    """
              该类用于设置全局参数接口
    """
    refreshInterfaceSig = QtCore.pyqtSignal(int, int)
    def __init__(self, h_parentwidget, n_uid, n_areanum, n_stationid):
        KxBaseParamWidget.__init__(self,n_uid, n_areanum, n_stationid)
        self.h_parent = h_parentwidget
        self.ui = Ui_ParamPYWidget()
        self.ui.setupUi(self)
        self._initui()
        self.imageori = None
        self.params.extend([
            {'name': u'图像信息', 'type': 'imageinfo',
             'value': {"isShow": True}, "infovisible": True, 'visible': False},
            {'name': u'载入图片', 'type': 'action'},
            {'name': u'显示图像', 'type': 'list', 'values': {'RGB': 0, 'R': 1, 'G': 2, 'B': 3,
                                                         'HSV': 4, 'H': 5, 'S': 6, 'V': 7}},
            {'name': '阈值', 'type': 'int', 'value': 100, 'limits': [0, 255]},

        ])
        self.p = KxParameter.create(name='params', type='group', children=self.params)
        self.h_parameterTree.setParameters(self.p, showTop=False)
        self.p.param(u'图像信息').add2view(self.ui.h_gVShowRealImg, self.img)

        self.ui.h_pBtSave.clicked.connect(self.saveparameters)
        self.ui.h_pBtCancel.clicked.connect(self.cancelparameter)
        self.p.param('载入图片').sigActivated.connect(self.loadimg)
        self.p.param('显示图像').sigValueChanged.connect(self._changeshowimg)
        self.p.param('阈值').sigValueChanged.connect(self._threshimg)


    def _threshimg(self, *even):
        nindex = int(self.p.param("显示图像").value())
        if nindex != 0 and nindex != 4:
            img = self._getshowimg(nindex)
            if img is not None:
                ret, threshimg = cv2.threshold(img, even[1], 255, cv2.THRESH_BINARY)
                self.img.setImage(threshimg, autoLevels=False)


    def _changeshowimg(self, *even):
        img = self._getshowimg(even[1])
        if img is not None:
            self.img.setImage(img, autoLevels=False)


    def _getshowimg(self, nindex):
        if self.imageori is not None:
            if len(self.imageori.shape) == 2:
                return self.imageori
            else:
                if nindex == 0:
                    return self.imageori
                elif nindex == 1:
                    rgb = cv2.split(self.imageori)
                    return rgb[0]
                elif nindex == 2:
                    rgb = cv2.split(self.imageori)
                    return rgb[1]
                elif nindex == 3:
                    rgb = cv2.split(self.imageori)
                    return rgb[2]
                elif nindex == 4:
                    hsv = cv2.cvtColor(self.imageori, cv2.COLOR_RGB2HSV)
                    cv2.imwrite("d:\\img\\hsvnew.bmp", hsv)
                    return hsv
                elif nindex == 5:
                    hsv = cv2.cvtColor(self.imageori, cv2.COLOR_RGB2HSV)
                    phsv = cv2.split(hsv)
                    return phsv[0]
                elif nindex == 6:
                    hsv = cv2.cvtColor(self.imageori, cv2.COLOR_RGB2HSV)
                    phsv = cv2.split(hsv)
                    return phsv[1]
                elif nindex == 7:
                    hsv = cv2.cvtColor(self.imageori, cv2.COLOR_RGB2HSV)
                    phsv = cv2.split(hsv)
                    return phsv[2]


    def _initui(self):
        """补充UI，qtdesigner无法一步到位"""
        self.h_parameterTree = ParameterTree(self.ui.widget_param, False)
        self.h_parameterTree.setStyleSheet("QTreeWidget::item{height:30px;}")
        self.verlayout = QtWidgets.QVBoxLayout(self.ui.widget_param)
        self.verlayout.addWidget(self.h_parameterTree)
        self.verlayout.setContentsMargins(0, 0, 0, 0)
        self.verlayout.setSpacing(0)
        self.view = pg.ViewBox(invertY = True, enableMenu=False)
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

    def loadimg(self):
        file_name = QtWidgets.QFileDialog.getOpenFileName(self, "open file dialog", "D://card",
                                                      "bmp files(*.bmp)")
        if file_name[0] != "":
            self._loadimg(file_name[0])

    def _loadimg(self, file_name):
        with open(file_name, 'rb') as curfp:
            self.imageori = np.array(copy.copy(Image.open(curfp)))
            self.img.setImage(self.imageori, autoLevels=False)


registerkxwidget(name='KxGlobal', cls=KxGlobal, override=True)


if __name__ == '__main__':
    app = QtGui.QApplication([])
    w = KxGlobal(None, 1, 2 ,3)
    w.show() 
    app.exec_()       