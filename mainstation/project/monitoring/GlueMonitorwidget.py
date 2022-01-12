from library.monitoring.BaseMonitoringWidget import KxBaseMonitoringWidget
from library.monitoring.BaseMonitoringWidget import registerkxmointorwidget
from PyQt5 import QtGui, QtWidgets, QtCore
from project.monitoring.ZSShowGridWidget import ZSImgListDetectWidget
import pyqtgraph as pg
from library.common.KxImageBuf import KxImageBuf
from project.monitoring.WidgePos import WidgetEdgePos
from library.common.readconfig import readconfig
import imc_msg
import json
import numpy as np
import cv2


class GlueMonitorWidget(KxBaseMonitoringWidget):
    """
    左上显示单个缺陷放大图，左下为缺陷位置，且位置的显示为当前这版托盘缺陷以及缺陷；
    右边为显示单个缺陷的信息，包括缩略图以及版周号，缺陷类型
    """
    _SCALE_FACTOR = 10
    def __init__(self, h_parent):
        super(GlueMonitorWidget, self).__init__(h_parent)
        self._initui()
        dict_config = readconfig()
        self.f_resolution = float(dict_config["resolution"])
        self.fp = None
        self.frameitem = None
        self.h_defectinfo = DefectClass()
        self.nrow, self.ncol = 0, 0# 大图拼成的行列
        self.n_curid = 0


    def _initui(self):
        self.horizonlayout = QtWidgets.QHBoxLayout(self)
        self.widget_map = QtWidgets.QWidget(self)
        self.verticallayout = QtWidgets.QVBoxLayout(self.widget_map)
        self.graphicsView = pg.GraphicsView(self.widget_map)
        self.view = pg.ViewBox(invertY=True, enableMenu=False, lockAspect=True)
        self.graphicsView.setCentralItem(self.view)
        self.imgitem = pg.ImageItem()
        self.view.addItem(self.imgitem)
        # self.view.setMouseEnabled(False, False)
        # self.view.setAspectLocked(True)
        self.verticallayout.addWidget(self.graphicsView, 1)
        self.graphicsView_small = pg.GraphicsView(self.widget_map)
        self.view_small = pg.ViewBox(invertY=True, enableMenu=False, lockAspect=True)
        self.graphicsView_small.setCentralItem(self.view_small)
        self.imgitem_small = pg.ImageItem()
        self.view_small.addItem(self.imgitem_small)
        self.text_item = pg.TextItem(color=(255, 0, 0))
        self.view_small.addItem(self.text_item)
        self.verticallayout.addWidget(self.graphicsView_small, 1)

        self.widget_edgepos = WidgetEdgePos()
        self.list_defectwidgt = ZSImgListDetectWidget(self)
        self.widget_right = QtWidgets.QWidget(self)
        self.verticallayout1 = QtWidgets.QVBoxLayout(self.widget_right)
        self.verticallayout1.addWidget(self.widget_edgepos, 2)
        self.verticallayout1.addWidget(self.list_defectwidgt, 3)
        self.list_defectwidgt.setMaxGridNum(1000)
        self.list_defectwidgt.SigSelectDefect.connect(self._slotCellClick)
        self.horizonlayout.addWidget(self.widget_map, 2)
        self.horizonlayout.addWidget(self.widget_right, 3)
        self.horizonlayout.setContentsMargins(0, 0, 0, 0)


    def _slotCellClick(self, img, ndefectID, pos):
        print ("_slotCellClick", ndefectID)
        if ndefectID >= self.h_defectinfo.size():
            return
        else:
            self._paintroi(ndefectID)
        self.imgitem_small.setImage(img, autoLevels=False)


    def _paintroi(self, ndefectID):
        lastroi = self.h_defectinfo.list_roi[self.n_curid]
        lastroi.setPen(1)
        roi = self.h_defectinfo.list_roi[ndefectID]
        roi.setPen(pg.mkPen(color=(255, 0, 0), width=3))
        self.n_curid = ndefectID


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

        if n_msgtype == imc_msg.MSG_CHECK_RESULT:

            dict_result = json.loads(tuple_data)

            img = self._getimage(dict_result)

            neww = int(img.shape[1] / self._SCALE_FACTOR)

            newh = int(img.shape[0] / self._SCALE_FACTOR)

            resizeimg = cv2.resize(img, (neww, newh))

            print ('resizeimg, ', resizeimg.shape)

            nIsFull = int(dict_result['ISFULL'])

            ndefectnum = dict_result['defect num']

            ncurrow = dict_result['currow']

            ncurcol = dict_result['curcol']

            self.h_defectinfo.append_blockinfo(resizeimg, dict_result['area'], ncurrow,  ncurcol)

            if ndefectnum != 0:

                defects = dict_result['defect feature']

                for i in range(ndefectnum):

                    singledefect = defects[i]

                    sdefectid = singledefect["defectid"]

                    pos = singledefect['pos']

                    ndots = int(singledefect['Dots'])

                    smallimg, expandpos = self._expandimg(img, pos)

                    h, w, c = img.shape

                    newpos = self._transformpos(expandpos, w, h, ncurrow, ncurcol)

                    self.h_defectinfo.append_defectinfo(smallimg, newpos, sdefectid, ndots)

                    self.list_defectwidgt.addOneDefectItemwithID(smallimg, self.h_defectinfo.size(), sdefectid, pos[0:2], ndots * self.f_resolution * self.f_resolution)

            if nIsFull == 1:# 当前pack已检完

                self.imgitem.setImage(self.h_defectinfo.getbigimg())

                self.widget_edgepos.setdata(self.h_defectinfo.list_narea, 4)

                for roi in self.h_defectinfo.list_roi:

                    roi.sigClicked.connect(self._roiclicked)

                    roi.setAcceptedMouseButtons(QtCore.Qt.LeftButton)

                    self.view.addItem(roi)

                # TODO : 为循环检而加
                self.h_parentwidget.callback2autorun()



    def _transformpos(self, pos, w, h, rowid, colid):
        """
        这里的pos坐标是在一个block上的坐标，第一步转换为大图坐标（压缩）
        且最终显示大图为旋转图像，第二步坐标也进行旋转（逆时针旋转90度）
        :param pos:
        :param w:
        :param h:
        :param rowid:
        :param colid:
        :return:
        """
        newpos = []

        x = int(pos[0] / self._SCALE_FACTOR) + int(w / self._SCALE_FACTOR) * colid

        y = int(pos[1] / self._SCALE_FACTOR) + int(h / self._SCALE_FACTOR) * rowid

        nw = int(pos[2] / self._SCALE_FACTOR)

        nh = int(pos[3] / self._SCALE_FACTOR)

        # 偏移坐标在整图中的位置， x、y、w、h

        newpos.append(y)

        newpos.append(int(w / self._SCALE_FACTOR) * self.ncol - x - nw)

        newpos.append(nh)

        newpos.append(nw)

        return newpos


    def _roiclicked(self, *args):
        nid = args[0].nid
        s_word = args[0].word
        self.imgitem_small.setImage(self.h_defectinfo.list_defectimg[nid], autoLevels=False)
        self._paintroi(nid)
        self.text_item.setText(s_word)


    def _expandimg(self, bigimg, pos, nexpand=50):
        size = bigimg.shape[0:2]
        x = max(0, pos[0] - nexpand)
        y = max(0, pos[1] - nexpand)
        xend = min(size[1], pos[0] + pos[2] + nexpand)
        yend = min(size[0], pos[1] + pos[3] + nexpand)
        smallimg = bigimg[y:yend, x:xend, :]
        return smallimg, [x, y ,xend - x + 1, yend - y + 1]


    def clear(self):
        self.n_curid = 0
        self.list_defectwidgt.clear()
        self.imgitem.clear()
        self.view.clear()
        self.view.addItem(self.imgitem)
        self.nrow, self.ncol = self.h_parentwidget.callback2getrowcol()
        self.h_defectinfo.clear()
        self.h_defectinfo.setrowcol(self.nrow, self.ncol)
        self.widget_edgepos.clear()
        list_shead = self.h_parentwidget.callback2getlisthead()
        self.widget_edgepos.sethead(list_shead)



registerkxmointorwidget("GlueMonitorWidget", GlueMonitorWidget)


class ROIwithID(pg.ROI):
    FONT_SIZE = 8

    def __init__(self, pos, nid, word, **kwargs):
        pg.ROI.__init__(self, pos, **kwargs)
        self.nid = nid
        self.word = word

class DefectClass(object):
    """
    用于存储当前pack id 结果的数据
    """
    def __init__(self):
        super(DefectClass, self).__init__()
        self.list_defectimg = []#小缺陷图
        self.list_defectpos = []#缺陷坐标
        self.list_roi = []      #缺陷roi对象
        self.list_dots = []     #缺陷点数
        self.list_bigimg = []   #每个块原图
        self.list_narea = []    #每个块面积
        self.nrow = 0#大图是由这么多行列组成
        self.ncol = 0

        self.bigimg = None

    def setrowcol(self, row, col):
        self.nrow = row
        self.ncol = col

    def append_defectinfo(self, defectimg, pos, sdefectword, dots):
        """
        存储检测结果，针对单个缺陷
        :param defectimg:   单张缺陷图
        :param pos:         单个缺陷坐标
        :param sdefectword: 缺陷名称（格式： 块号_缺陷号）
        :param dots:        缺陷点数
        :return:
        """
        self.list_defectimg.append(defectimg)
        self.list_defectpos.append(pos)
        self.list_dots.append(dots)
        roi = ROIwithID(pos=pos[:2], nid=len(self.list_defectpos) - 1, word=sdefectword, size=pos[2:], pen=1)
        self.list_roi.append(roi)


    def append_blockinfo(self, bigimg, narea, nrowid, ncolid):
        """
        存储检测结果，针对单个block的信息
        :param bigimg: block原图压缩图
        :param narea:  block面积
        :param nrowid:  当前图是大图的第几行
        :param ncolid:  当前图是大图的第几列
        :return:
        """
        h, w, c = bigimg.shape

        if len(self.list_bigimg) == 0:
            self.bigimg = np.zeros((h * self.nrow, w * self.ncol, c), np.uint8)

        self.list_bigimg.append(bigimg)
        self.list_narea.append(narea)
        self.bigimg[nrowid*h:(nrowid + 1) * h, ncolid*w:(ncolid + 1) * w] = bigimg



    def size(self):
        return len(self.list_defectpos)


    def getbigimg(self):
        rotateimg = cv2.rotate(self.bigimg, cv2.ROTATE_90_COUNTERCLOCKWISE)

        return rotateimg


    def clear(self):
        self.list_defectimg = []#小缺陷图
        self.list_defectpos = []#缺陷坐标
        self.list_dots = []     #缺陷点数
        self.list_roi = []      #缺陷roi对象
        self.list_bigimg = []   #每个块原图
        self.list_narea = []    #每个块面积


class testviewwidget(QtWidgets.QWidget):
    def __init__(self):
        super(testviewwidget, self).__init__()
        self.horlayout = QtWidgets.QHBoxLayout(self)

        self.graphicsView = pg.GraphicsView(self)
        self.view = pg.ViewBox()
        self.graphicsView.setCentralItem(self.view)
        self.imgitem = pg.ImageItem()
        self.view.addItem(self.imgitem)
        self.horlayout.addWidget(self.graphicsView)

        zeroimg = np.ones((1000, 1000), np.uint8) * 200
        self.imgitem.setImage(zeroimg, autoLevels=False)

        roi = ROIwithID(pos=[100, 100], nid=1, word="3_2", size=[100, 100], pen=1)
        self.view.addItem(roi)


if __name__ == "__main__":
    A = QtWidgets.QApplication([])
    widget = testviewwidget()

    widget.show()
    A.exec_()