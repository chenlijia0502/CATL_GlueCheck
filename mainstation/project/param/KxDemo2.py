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
from pyqtgraph.parametertree import ParameterTree
import cv2
import copy
from PIL import Image
import numpy as np
import os

class KxDemo2(KxBaseParamWidget):
    """
              测试类
    """
    def __init__(self, h_parentwidget, n_uid, n_areanum, n_stationid):
        KxBaseParamWidget.__init__(self,n_uid, n_areanum, n_stationid)
        self.h_parent = h_parentwidget
        self.ui = Ui_ParamPYLoadWidget()
        self.ui.setupUi(self)
        self._initui()
        self._initparam()
        # use for save image
        self.params.extend([
            {'name': u'检查设置', 'type': 'group', 'children': [
                {'name': u'图像路径', 'type': 'str', 'value': "", 'visible':False},
                {'name': u'二值化阈值', 'type':  'int', 'value': 30, 'step': 1, 'limits': (0, 255)},
                {'name': u'版面定位框', 'type': 'roiwithtext', 'value': {"isShow": False, "pos": u"0,0,100,100"},
                 "roi_opt": {"word": u'版面定位框', "scaleable": True}, "infovisible": False},
            ]
            },
        ])
        self.p = KxParameter.create(name='params', type='group', children=self.params)
        self.h_parameterTree.setParameters(self.p, showTop=False)
        self.ui.h_pBtSave.clicked.connect(self.saveparameters)
        self.ui.h_pBtCancel.clicked.connect(self.cancelparameter)
        self.ui.h_pBtLoad.clicked.connect(self.loadimg)
        self.p.param(u'检查设置', u'版面定位框').add2view(self.view)
        self.p.param(u'检查设置', u'版面定位框').sigposchanged.connect(self._refreshimg)
        self.p.param(u'检查设置', u'二值化阈值').sigValueChanged.connect(self._refreshimg)


    def _initparam(self):
        self.imageori = None


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


    def loadimg(self):
        file_name = QtWidgets.QFileDialog.getOpenFileName(self, "open file dialog", "D://card",
                                                      "bmp files(*.bmp)")
        if file_name[0] != "":
            self._loadimg(file_name[0])

    def _loadimg(self, file_name):
        with open(file_name, 'rb') as curfp:
            self.imageori = copy.copy(Image.open(curfp))
            self.img.setImage(np.array(self.imageori), autoLevels=False)

    def saveparameters(self):
        if self.img.image is not None:
            list_path = self.str_filedirectory.split("\\")
            list_path[-1] = "image.bmp"
            imgpath = "\\".join(list_path)
            self.p.param(u"检查设置", u"图像路径").setValue(imgpath)
            self.imageori.save(imgpath)
        KxBaseParamWidget.saveparameters(self)


    def loadparameters(self):
        KxBaseParamWidget.loadparameters(self)
        imgpath = self.p.param(u"检查设置", u"图像路径").value()
        if os.path.isfile(imgpath):
            self._loadimg(imgpath)

    def _refreshimg(self):
        if self.imageori is None:
            return
        pos = self.p.param(u'检查设置', u'版面定位框').get_list_pos()
        threshvalue = int(self.p.param(u'检查设置', u'二值化阈值').value())
        srcimg = copy.copy(np.array(self.imageori))
        cutimg = srcimg[pos[1]:pos[3], pos[0]:pos[2]]
        # algorithm = EdgeExtraction()
        # edgeimg = algorithm.FindEdge(cutimg, threshvalue)
        # srcimg[pos[1]:pos[3], pos[0]:pos[2]] = edgeimg
        # self.img.setImage(srcimg, autoLevels=False)

        algorithm = CardFFT()
        edgeimg = algorithm.solve(cutimg)
        # srcimg[pos[1]:pos[3], pos[0]:pos[2]] = edgeimg


registerkxwidget(name='KxDemo2', cls=KxDemo2, override=True)

from matplotlib import pyplot as plt


class CardFFT(object):
    def __init__(self):
        plt.subplot(121)

    def solve(self, srcimg):
        # coding=utf-8
        import cv2
        import numpy as np

        img = cv2.cvtColor(srcimg, cv2.COLOR_RGB2GRAY)
        f = np.fft.fft2(img)
        fshift = np.fft.fftshift(f)
        # 这里构建振幅图的公式没学过
        magnitude_spectrum = 20 * np.log(np.abs(fshift))  # 先取绝对值,表示取模。取对数,将数据范围变小
        # return magnitude_spectrum
        # plt.subplot(121), plt.imshow(img, cmap='gray')
        # plt.title('Input Image'), plt.xticks([]), plt.yticks([])
        plt.imshow(magnitude_spectrum, cmap='gray')
        plt.title('Magnitude Spectrum'), plt.xticks([]), plt.yticks([])
        plt.show()
        # cv2.namedWindow("fft", 0)
        # cv2.imshow("fft", magnitude_spectrum)
        # cv2.waitKey(100)



class EdgeExtraction(object):
    def __init__(self):
        pass

    def FindEdge(self, srcimg, nthreshvalue):
        """
        算法步骤：
        （1）高斯平滑，两个方向梯度权值相加
        （2）对梯度图像二值化
        （3）提取最大blob
        （4）填充blob，但对blob有要求，并有默认条件，详情看_fill_image
        （5）blob与原图相与，得到检测区域

        应用场景：无法通过单纯的二值化实现图像的分割的场景，对图像的边缘要求具备完整性
        Parameters
        ----------
        srcimg
        nthreshvalue

        Returns
        -------

        """

        if len(srcimg.shape) >= 3:
            solveimg = cv2.cvtColor(srcimg, cv2.COLOR_RGB2GRAY)
        else:
            solveimg = copy.copy(srcimg)
        img = cv2.GaussianBlur(solveimg, (11, 11), 0)
        kernel = np.array([[1, 0, -1], [1, 0, -1], [1, 0, -1]])
        fil1 = cv2.filter2D(img, cv2.CV_16S, kernel)
        fil2 = cv2.filter2D(img, cv2.CV_16S, kernel.T)

        fil1abs = cv2.convertScaleAbs(fil1)
        fil2abs = cv2.convertScaleAbs(fil2)
        re = cv2.addWeighted(fil1abs, 0.5, fil2abs, 0.5, 0)
        res, threshimg = cv2.threshold(re, nthreshvalue, 255, cv2.THRESH_BINARY)

        openkernel = np.ones((5, 5))
        openimg = cv2.morphologyEx(threshimg, cv2.MORPH_CLOSE, openkernel)
        singleblobimg = self._blob(openimg)
        if singleblobimg is not None:
            resultimg = cv2.morphologyEx(singleblobimg, cv2.MORPH_ERODE, openkernel)
            if len(srcimg.shape) >= 3:
                resultimg = cv2.merge([resultimg, resultimg, resultimg])
            andimg = cv2.bitwise_and(resultimg, srcimg)
            return andimg
        else:
            return srcimg

    def _blob(self, binaryimg):
        retval, labels, stats, centroids = cv2.connectedComponentsWithStats(binaryimg, connectivity=8)
        if retval > 1: #1为只有背景
            array_dot = stats[1:, -1]
            index = array_dot.argmax()
            nbloblabel = index + 1
            label8u = cv2.convertScaleAbs(labels)
            ret, threshimg = cv2.threshold(label8u, nbloblabel, 0, cv2.THRESH_TOZERO_INV)
            ret, resultimg = cv2.threshold(threshimg, nbloblabel-1, 255, cv2.THRESH_BINARY)
            return self._fill_image(resultimg)
        else:
            return None


    def _fill_image(self, image):
        """这个函数使用有个前提，整个算法找到的blob需是闭合的，才能填充内部；并且默认（0,0）位置像素一定是零"""
        copyImage = image.copy()#复制原图像
        h, w = image.shape[:2]#读取图像的宽和高
        mask = np.zeros([h+2, w+2], np.uint8)#新建图像矩阵  +2是官方函数要求
        cv2.floodFill(copyImage, mask, (0, 0), (100))
        ret, threshimg1 = cv2.threshold(copyImage, 100, 255, cv2.THRESH_TOZERO_INV)
        ret, threshimg = cv2.threshold(threshimg1, 100 -1, 255, cv2.THRESH_BINARY)
        whiteimg = np.ones(threshimg.shape, dtype=np.uint8) * 255
        resultimg = whiteimg - threshimg
        return resultimg





if __name__ == '__main__':
    app = QtWidgets.QApplication([])
    w = KxDemo2(None, 1, 2 ,3)
    w.show() 
    app.exec_()       