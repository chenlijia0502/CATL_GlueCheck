from library.monitoring.BaseMonitoringWidget import KxBaseMonitoringWidget
from library.monitoring.BaseMonitoringWidget import registerkxmointorwidget
from PyQt5 import QtGui, QtWidgets
from project.monitoring.ZSShowGridWidget import ZSImgListDetectWidget
import pyqtgraph as pg
from library.common.KxImageBuf import KxImageBuf
import json

class GlueMonitorWidget(KxBaseMonitoringWidget):
    """
    左上显示单个缺陷放大图，左下为缺陷位置，且位置的显示为当前这版托盘缺陷以及缺陷；
    右边为显示单个缺陷的信息，包括缩略图以及版周号，缺陷类型
    """
    def __init__(self, h_parent):
        super(GlueMonitorWidget, self).__init__(h_parent)
        self._initui()
        self.fp = None
        self.frameitem = None
        self.clear()
        self.plotframe([10000, 10000])

    def _initui(self):
        self.horizonlayout = QtWidgets.QHBoxLayout(self)
        self.widget_map = QtWidgets.QWidget(self)
        self.verticallayout = QtWidgets.QVBoxLayout(self.widget_map)
        self.graphicsView = pg.GraphicsView(self.widget_map)
        self.view = pg.ViewBox()
        self.graphicsView.setCentralItem(self.view)
        self.imgitem = pg.ImageItem()
        self.view.addItem(self.imgitem)
        self.verticallayout.addWidget(self.graphicsView, 1)
        self.mapview = pg.GraphicsLayoutWidget()
        self.plot = self.mapview.addPlot()
        self.verticallayout.addWidget(self.mapview, 1)
        self.list_defectwidgt = ZSImgListDetectWidget(self)
        self.list_defectwidgt.setMaxGridNum(1000)
        self.list_defectwidgt.SigSelectDefect.connect(self._slotCellClick)
        self.horizonlayout.addWidget(self.widget_map, 2)
        self.horizonlayout.addWidget(self.list_defectwidgt, 3)
        self.horizonlayout.setContentsMargins(0, 0, 0, 0)

    def _slotCellClick(self, img, BIG_ID, ID):
        self.imgitem.setImage(img)
        self._click2showdefectpos(img, BIG_ID, ID)

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

    def plotframe(self, size:[], frame=None):
        """
        输入框架大小，进行尺寸绘制
        :param size: [width, height]
        :return:
        """
        self.plot.clear()
        if frame is not None:
            self.plot.addItem(self.frameitem)
        else:
            self.frameitem = self.plot.plot(x=[0, size[0], size[0], 0, 0], y=[0, 0, size[1], size[1], 0])

    def appenddefectpos(self, BIG_ID, pos:[]):
        """
        叠加缺陷位置，当切换托盘的时候重新清零
        :param BIG_ID:
        :param pos:
        :return:
        """
        if BIG_ID in self.dict_showpos:
            self.dict_showpos[BIG_ID].append(pos)
        else:
            self.dict_showpos[BIG_ID] = []
            self.dict_showpos[BIG_ID].append(pos)
        if self.n_curid != BIG_ID:# 切换到下一个id
            self.n_curid = BIG_ID
            self.plotframe([], self.frameitem)
        self.plot.plot(x=[pos[0]], y=[pos[1]], pen=(0, 0, 200), symbolBrush=(0, 0, 200), symbolPen='w', symbol='o', symbolSize=14)


    def _click2showdefectpos(self, img, BIG_ID, id):
        self.imgitem.setImage(img, autoLevels=False)
        self.plotframe([], self.frameitem)
        if BIG_ID in self.dict_showpos:
            list_pos = self.dict_showpos[BIG_ID]
            for idx , pos in enumerate(list_pos):
                if idx != id:
                    self.plot.plot(x=[pos[0]], y=[pos[1]],  symbolBrush=(0, 0, 200), symbolPen='w',
                                   symbol='o', symbolSize=14)
                else:
                    self.plot.plot(x=[pos[0]], y=[pos[1]],  symbolBrush=(200, 0, 0), symbolPen='w',
                                   symbol='t', symbolSize=14)

    def addonedefect(self, img, BIG_ID, ID, pos):
        self.list_defectwidgt.addOneDefectItemwithID(img, BIG_ID, ID)
        self.appenddefectpos(BIG_ID, pos)

    def recmsg(self, n_stationid, n_msgtype, tuple_data):
        dict_result = json.loads(tuple_data)
        #print (dict_result)
        img = self._getimage(dict_result)
        #print (img.shape)
        defects = dict_result['defect feature']
        ndefectnum = dict_result['defect num']
        for i in range(ndefectnum):
            singledefect = defects[i]
            pos = singledefect['pos']
            smallimg = self._expandimg(img, pos)
            self.addonedefect(smallimg, 0, i, pos[0:2])


    def _expandimg(self, bigimg, pos, nexpand=50):
        size = bigimg.shape[0:2]
        x = max(0, pos[0] - nexpand)
        y = max(0, pos[1] - nexpand)
        xend = min(size[1], pos[0] + pos[2] + nexpand)
        yend = min(size[0], pos[1] + pos[3] + nexpand)
        smallimg = bigimg[y:yend, x:xend, :]
        return smallimg

    def clear(self):
        self.dict_showpos = {}
        self.n_curid = None
        self.plot.clear()
        self.list_defectwidgt.clear()
        self.imgitem.clear()

#print ('hone')

registerkxmointorwidget("GlueMonitorWidget", GlueMonitorWidget)

if __name__ == "__main__":
    A = QtWidgets.QApplication([])
    widget = GlueMonitorWidget(None)
    widget.plotframe([100, 200])

    import numpy as np
    img1 = np.ones([100, 100], np.uint8) * 255
    img2 = np.ones([100, 100], np.uint8) * 255
    widget.addonedefect(img1, 0, 0, [50, 50])
    widget.addonedefect(img2, 0, 1, [70, 70])
    img3 = np.ones([100, 100], np.uint8) * 100
    img4 = np.ones([100, 100], np.uint8) * 50
    widget.addonedefect(img3, 1, 0, [50, 60])
    widget.addonedefect(img4, 1, 1, [60, 70])

    widget.show()
    A.exec_()