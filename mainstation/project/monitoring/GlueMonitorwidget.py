from library.monitoring.BaseMonitoringWidget import KxBaseMonitoringWidget
from library.monitoring.BaseMonitoringWidget import registerkxmointorwidget
from PyQt5 import QtGui, QtWidgets, QtCore
from project.monitoring.ZSShowGridWidget import ZSImgListDetectWidget
import pyqtgraph as pg
from library.common.KxImageBuf import KxImageBuf
from project.monitoring.WidgePos import WidgetEdgePos
import json
import csv, codecs
import unicodecsv as ucsv
import numpy as np



class GlueMonitorWidget(KxBaseMonitoringWidget):
    """
    左上显示单个缺陷放大图，左下为缺陷位置，且位置的显示为当前这版托盘缺陷以及缺陷；
    右边为显示单个缺陷的信息，包括缩略图以及版周号，缺陷类型
    """
    _SCALE_FACTOR = 10
    def __init__(self, h_parent):
        super(GlueMonitorWidget, self).__init__(h_parent)
        self._initui()
        self.fp = None
        self.frameitem = None
        self.clear()
        #self.savecsv()
        self.list_img = []
        self.nblockid = 0
        self.list_pos = []
        self.list_roi = []
        self.ndefectid = 0
        self.list_smallimg = []
        self.list_sdefectword = []

    def _initui(self):
        self.horizonlayout = QtWidgets.QHBoxLayout(self)
        self.widget_map = QtWidgets.QWidget(self)
        self.verticallayout = QtWidgets.QVBoxLayout(self.widget_map)
        self.graphicsView = pg.GraphicsView(self.widget_map)
        self.view = pg.ViewBox()
        self.graphicsView.setCentralItem(self.view)
        self.imgitem = pg.ImageItem()
        self.view.addItem(self.imgitem)
        # self.view.setMouseEnabled(False, False)
        # self.view.setAspectLocked(True)
        self.verticallayout.addWidget(self.graphicsView, 1)
        self.graphicsView_small = pg.GraphicsView(self.widget_map)
        self.view_small = pg.ViewBox()
        self.graphicsView_small.setCentralItem(self.view_small)
        self.imgitem_small = pg.ImageItem()
        self.view_small.addItem(self.imgitem_small)
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
        if ndefectID >= len(self.list_roi):
            return
        else:
            self._paintroi(ndefectID)
        self.imgitem_small.setImage(img, autoLevels=False)

    def _paintroi(self, ndefectID):
        lastroi = self.list_roi[self.n_curid]
        lastroi.setPen(1)
        roi = self.list_roi[ndefectID]
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




    def _click2showdefectpos(self, img, BIG_ID, id):
        self.imgitem.setImage(img, autoLevels=False)
        if BIG_ID in self.dict_showpos:
            list_pos = self.dict_showpos[BIG_ID]
            pass


    def addonedefect(self, img, BIG_ID, ID, pos):
        self.list_defectwidgt.addOneDefectItemwithID(img, BIG_ID, ID, pos)


    def recmsg(self, n_stationid, n_msgtype, tuple_data):
        dict_result = json.loads(tuple_data)
        #print (dict_result)
        img = self._getimage(dict_result)
        self._appendimg(img)
        #print (img.shape)
        defects = dict_result['defect feature']
        ndefectnum = dict_result['defect num']
        for i in range(ndefectnum):
            singledefect = defects[i]
            sdefectid = singledefect["defectid"]
            pos = singledefect['pos']
            smallimg = self._expandimg(img, pos)
            self._appendalldefectpos(pos, self.nblockid, img.shape[1], img.shape[0])
            self.addonedefect(smallimg, self.ndefectid, sdefectid, pos[0:2])
            self.ndefectid += 1
            self.list_sdefectword.append(sdefectid)
            self.list_smallimg.append(smallimg)
        self.nblockid += 1


    def _appendalldefectpos(self, pos, nblockid, singleimgw, singleimgh):
        """
          把缺陷坐标放入list_pos,而且需要注意的是x
          :param nid:
          :param pos:
          :param singleimgw:
          :param singleimgh:
          :return:
          """
        x = int(nblockid / 2)
        y = int(nblockid % 2)
        nextend = 50
        list_pos = [0, 0, 0, 0]
        list_pos[0] = int(max(0, x * singleimgw + pos[0] - nextend) / self._SCALE_FACTOR)
        list_pos[1] = int(max(0, y * singleimgh + pos[1] - nextend) / self._SCALE_FACTOR)
        list_pos[2] = int((pos[2] + nextend * 2) / self._SCALE_FACTOR)  # 没做保护但无所谓了
        list_pos[3] = int((pos[3] + nextend * 2) / self._SCALE_FACTOR)
        self.list_pos.append(list_pos)


    def _appendimg(self, img):
        import cv2
        neww = int(img.shape[1] / self._SCALE_FACTOR)
        newh = int(img.shape[0] / self._SCALE_FACTOR)
        resizeimg = cv2.resize(img, (neww, newh))

        self.list_img.append(resizeimg)

        if len(self.list_img) == 6:
            w = self.list_img[0].shape[1]
            h = self.list_img[0].shape[0]
            zeroimg = np.zeros((h * 2, w * 3, 3), np.uint8)
            zeroimg[:h, :w] = self.list_img[0]
            zeroimg[h:2 * h, :w] = self.list_img[1]
            zeroimg[:h, w:2 * w] = self.list_img[2]
            zeroimg[h:2 * h, w:2 * w] = self.list_img[3]
            zeroimg[:h, 2 * w:3 * w] = self.list_img[4]
            zeroimg[h:2 * h, 2 * w:3 * w] = self.list_img[5]

            #showimg = cv2.rotate(zeroimg, cv2.ROTATE_90_COUNTERCLOCKWISE)
            self.imgitem.setImage(zeroimg, autoLevels=False)
            self.list_img = []

            for nindex, pos in enumerate(self.list_pos):
                #newpos = [pos[1], zeroimg.shape[1] - pos[0] - 1, pos[3], pos[2]]# 图像发生了旋转，那么坐标也需要旋转
                newpos = pos
                roi = ROIwithID(pos=newpos[:2], nid=nindex, word=self.list_sdefectword[nindex],size=newpos[2:], pen=1)
                roi.sigClicked.connect(self._roiclicked)
                roi.setAcceptedMouseButtons(QtCore.Qt.LeftButton)
                self.list_roi.append(roi)
                self.view.addItem(roi)


    def _roiclicked(self, *args):
        nid = args[0].nid
        self.imgitem_small.setImage(self.list_smallimg[nid], autoLevels=False)
        self._paintroi(nid)




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
        self.n_curid = 0
        self.ndefectid = 0
        self.list_defectwidgt.clear()
        self.imgitem.clear()
        self.list_img = []
        self.list_sdefectword = []



    def savecsv(self):
        import time
        s_data = time.strftime("%Y-%m-%d")
        csvpath = "F:\\模组数据.csv"
        fieldnames = ["barcode", "检测时间", "测量结果", "异物数量", "气泡数量", "涂胶面积"]
        # with codecs.open(csvpath, 'w', 'utf-8') as csvfile:
        #     # 指定csv文件指定头部
        #
        #
        #     writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        #     writer.writeheader()
        #     try:
        #         writer.writerow({fieldnames[0]: "None",
        #                          fieldnames[1]: time.strftime("%Y:%m:%d-%H:%M:%S"),
        #                          fieldnames[2]: "OK"})
        #     except Exception as e:
        #         pass
        with open(csvpath, "rb") as f:
            r = ucsv.reader(f, encoding="gbk")
            print(r.line_num)

        with open(csvpath, "wb") as f:
            w = ucsv.writer(f, encoding="gbk")
            w.writerow(fieldnames)

#print ('hone')

registerkxmointorwidget("GlueMonitorWidget", GlueMonitorWidget)


class ROIwithID(pg.ROI):
    FONT_SIZE = 8

    def __init__(self, pos, nid, word, **kwargs):
        pg.ROI.__init__(self, pos, **kwargs)
        self.nid = nid
        self.word = word

    # def paint(self, p, opt, widget):
    #     super(ROIwithID, self).paint(p, opt, widget)
    #     s_font1 = QtGui.QFont()
    #     s_font1.setWeight(90)
    #     n_size = min(self.state['size'][0], self.state['size'][1]) / 10.0
    #     if n_size < self.FONT_SIZE:
    #         n_size = self.FONT_SIZE
    #     n_size = 20
    #     s_font1.setPointSize(n_size)
    #     p.setFont(s_font1)
    #     self.fontRect = QtCore.QRectF(0, -n_size * 1.5, max(self.state['size'][0], len(self.word) * n_size * 1.5),
    #                                   n_size * 1.5)
    #     p.drawText(self.fontRect, QtCore.Qt.AlignLeft, self.word)
    #


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