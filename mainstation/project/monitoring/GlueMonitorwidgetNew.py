import json
import cv2
import numpy as np
import pyqtgraph as pg
from library.monitoring.BaseMonitoringWidget import KxBaseMonitoringWidget
from library.monitoring.BaseMonitoringWidget import registerkxmointorwidget
from PyQt5 import QtGui, QtWidgets, QtCore
from UI.ui_gluemonitor import Ui_glue_monitor
from library.common.KxImageBuf import KxImageBuf
from project.monitoring.WidgePos import WidgetEdgePos


class GlueMonitorWidgetNew(KxBaseMonitoringWidget):
    _SCALE_FACTOR = 10
    def __init__(self, h_parent):
        super(GlueMonitorWidgetNew, self).__init__(h_parent)
        self.ui = Ui_glue_monitor()
        self.ui.setupUi(self)
        self._initui()
        self._initconnect()
        self.fp = None
        self.list_img = []# 绘制成缩略图
        self.list_data = []


    def _initui(self):
        self.verticallayout = QtWidgets.QVBoxLayout(self.ui.widget_2)
        self.biggraphicsView = pg.GraphicsView(self.ui.widget_2)
        self.bigview = pg.ViewBox()
        self.biggraphicsView.setCentralItem(self.bigview)
        self.bigimgitem = pg.ImageItem()
        self.bigview.addItem(self.bigimgitem)
        self.verticallayout.addWidget(self.biggraphicsView)
        self.verticallayout.setContentsMargins(0,0,0,0)
        self.bigview.setMouseEnabled(False, False)

        self.verticallayout1 = QtWidgets.QVBoxLayout(self.ui.widget_3)
        self.smallgraphicsView = pg.GraphicsView(self.ui.widget_3)
        self.smallview = pg.ViewBox()
        self.smallgraphicsView.setCentralItem(self.smallview)
        self.smallimgitem = pg.ImageItem()
        self.smallview.addItem(self.smallimgitem)
        self.verticallayout1.addWidget(self.smallgraphicsView)
        self.verticallayout1.setContentsMargins(0,0,0,0)

        self.verticallayout2 = QtWidgets.QVBoxLayout(self.ui.widget_5)
        self.tabwidget_defectid = QtWidgets.QTableWidget(1000,1)
        self.tabwidget_defectid.setHorizontalHeaderLabels(['缺陷ID'])
        self.tabwidget_defectid.horizontalHeader().setSectionResizeMode(QtWidgets.QHeaderView.Stretch)
        self.verticallayout2.addWidget(self.tabwidget_defectid)

        self.verticallayout3 = QtWidgets.QVBoxLayout(self.ui.widget)
        self.tabwidget_packid = QtWidgets.QTableWidget(1000,2)
        self.tabwidget_packid.setHorizontalHeaderLabels(['PACK ID', '时间'])
        #self.tabwidget_packid.horizontalHeader().setSectionResizeMode(QtWidgets.QHeaderView.Stretch)
        self.verticallayout3.addWidget(self.tabwidget_packid)

        self.crossitem = CrossItem()
        self.crossitem.add2item(self.bigview)
        self.crossitem.setpos(0, 0)

    def _initconnect(self):
        self.tabwidget_defectid.cellClicked.connect(self._showdefect)

    def _getimage(self, dict_result):
        try:
            readimagepath = dict_result['imagepath']
            startoffset = dict_result['startoffset']
            offsetlen = dict_result['imageoffsetlen']
        except AttributeError:
            return None
        if self.fp is None:
            try:
                self.fp = open(readimagepath, "rb")
            except IOError:
                return None
        self.fp.seek(startoffset)
        data = self.fp.read(offsetlen)
        Img = KxImageBuf()
        Img.unpack(data)
        arrImg = Img.Kximage2npArr()
        return arrImg

    def recmsg(self, n_stationid, n_msgtype, tuple_data):
        dict_result = json.loads(tuple_data)
        #print (dict_result)
        img = self._getimage(dict_result)
        #defects = dict_result['defect feature']
        #ndefectnum = dict_result['defect num']
        self._appenddefect(img, dict_result)
        self._appendimg(img)


    def _appenddefect(self, img, dict_result):
        ndefectnum = dict_result['defect num']
        if ndefectnum == 0:
            return
        defects = dict_result['defect feature']
        nid = dict_result["id"]
        row = nid % 2
        col = int(nid / 2)
        h, w = img.shape[0:2]
        for i in range(ndefectnum):
            singledefect = defects[i]
            pos = singledefect['pos']
            smallimg = self._expandimg(img, pos)
            x = int((pos[0] + pos[2] / 2 + col * w) / self._SCALE_FACTOR)
            y = int((pos[1] + pos[3] / 2 + row * h) / self._SCALE_FACTOR)
            self.list_data.append(ImgAndPos(smallimg, (x, y)))

    def _expandimg(self, bigimg, pos, nexpand=100):
        size = bigimg.shape[0:2]
        x = max(0, pos[0] - nexpand)
        y = max(0, pos[1] - nexpand)
        xend = min(size[1], pos[0] + pos[2] + nexpand)
        yend = min(size[0], pos[1] + pos[3] + nexpand)
        smallimg = bigimg[y:yend, x:xend, :]
        return smallimg

    def _appendimg(self, img):
        neww = int(img.shape[1] / self._SCALE_FACTOR)
        newh = int(img.shape[0] / self._SCALE_FACTOR)
        resizeimg = cv2.resize(img, (neww, newh))
        self.list_img.append(resizeimg)

        if len(self.list_img) == 6:
            w = self.list_img[0].shape[1]
            h = self.list_img[0].shape[0]
            zeroimg = np.zeros((h*2, w*3, 3), np.uint8)
            zeroimg[:h, :w] = self.list_img[0]
            zeroimg[h:2*h, :w] = self.list_img[1]
            zeroimg[:h, w:2*w] = self.list_img[2]
            zeroimg[h:2*h, w:2*w] = self.list_img[3]
            zeroimg[:h, 2*w:3*w] = self.list_img[4]
            zeroimg[h:2*h, 2*w:3*w] = self.list_img[5]
            self.bigimgitem.setImage(zeroimg, autoLevels=False)
            self.list_img = []

            item1 = pg.InfiniteLine(pos=h, angle=0, pen={'color':(255, 255, 255), 'width':3}, movable=False)
            item2 = pg.InfiniteLine(pos=w, angle=90, pen={'color':(255, 255, 255), 'width':3}, movable=False)
            item3 = pg.InfiniteLine(pos=2*w, angle=90, pen={'color':(255, 255, 255), 'width':3}, movable=False)
            self.bigview.addItem(item1)
            self.bigview.addItem(item2)
            self.bigview.addItem(item3)
            self.settablewidgetinfo()

    def settablewidgetinfo(self):
        for i in range(len(self.list_data)):
            newitem = QtWidgets.QTableWidgetItem(str(i))
            self.tabwidget_defectid.setItem(i, 0, newitem)

    def _showdefect(self, *even):
        if self.bigimgitem.image is not None and even[0] < len(self.list_data):

            self.smallimgitem.setImage(self.list_data[even[0]].img)

            self.crossitem.setpos(self.list_data[even[0]].pos[0], self.list_data[even[0]].pos[1])


    def clear(self):
        self.list_img=[]
        self.bigview.clear()
        self.bigview.addItem(self.bigimgitem)
        self.crossitem.setpos(0, 0)
        self.crossitem.add2item(self.bigview)
        self.list_data = []
        self.tabwidget_defectid.clear()


registerkxmointorwidget("GlueMonitorWidgetNew", GlueMonitorWidgetNew)



class CrossItem(object):
    def __init__(self):
        super(CrossItem, self).__init__()
        self.verline = pg.InfiniteLine(pos=0, angle=90, pen={'color':(255, 0, 0), 'width':2}, movable=False)
        self.horline = pg.InfiniteLine(pos=0, angle=0, pen={'color':(255, 0, 0), 'width':2}, movable=False)

    def add2item(self, viewbox:pg.ViewBox):
        viewbox.addItem(self.verline)
        viewbox.addItem(self.horline)

    def setpos(self, x, y):
        self.verline.setPos(x)
        self.horline.setPos(y)


class ImgAndPos(object):
    def __init__(self, img, pos):
        super(ImgAndPos, self).__init__()
        self.img = img
        self.pos = pos

    # def setinfo(self, img, pos):
    #     self.img = img
    #     self.pos = pos

