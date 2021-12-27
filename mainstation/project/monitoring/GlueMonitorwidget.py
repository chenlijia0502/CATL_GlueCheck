from library.monitoring.BaseMonitoringWidget import KxBaseMonitoringWidget
from library.monitoring.BaseMonitoringWidget import registerkxmointorwidget
from PyQt5 import QtGui, QtWidgets, QtCore
from project.monitoring.ZSShowGridWidget import ZSImgListDetectWidget
import pyqtgraph as pg
from library.common.KxImageBuf import KxImageBuf
from project.monitoring.WidgePos import WidgetEdgePos
import json
import cv2
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
        self.list_img = []
        self.list_defectpos = []
        self.nblockid = 0
        self.list_roi = []

    def _initui(self):
        print('here')
        self.horizonlayout = QtWidgets.QHBoxLayout(self)
        self.widget_map = QtWidgets.QWidget(self)
        self.verticallayout = QtWidgets.QVBoxLayout(self.widget_map)
        self.graphicsView = pg.GraphicsView(self.widget_map)
        self.view = pg.ViewBox()
        self.graphicsView.setCentralItem(self.view)
        self.imgitem = pg.ImageItem()
        self.view.addItem(self.imgitem)
        #self.view.setMouseEnabled(False, False)
        #self.view.setAspectLocked(True)
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


    def _slotCellClick(self, img, BIG_ID, ID):
        pass
        #ceshi
        # print("_slot")
        # import numpy as np
        # h = 500
        # w = 2000
        # zeroimg = np.ones((h, w), np.uint8) * 100
        # zeroimg[int(h/2) - 2 : int(h/2) + 2, :] = 255
        # zeroimg[:, int(w/2) - 2 : int(w/2) + 2] = 255
        # zeroimg[:, :3] = 255
        # zeroimg[:, w-3:] = 255
        # zeroimg[:3, :] = 255
        # zeroimg[h-3:, :] = 255
        # print (zeroimg.shape)
        # self.imgitem.setImage(zeroimg)
        # self.imgitem.setImage(img)
        # self._click2showdefectpos(img, BIG_ID, ID)


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


    def _click2showdefectpos(self, img, BIG_ID, id):
        self.imgitem.setImage(img, autoLevels=False)
        if BIG_ID in self.dict_showpos:
            list_pos = self.dict_showpos[BIG_ID]
            # for idx , pos in enumerate(list_pos):
            #     if idx != id:
            #         self.plot.plot(x=[pos[0]], y=[pos[1]],  symbolBrush=(0, 0, 200), symbolPen='w',
            #                        symbol='o', symbolSize=14)
            #     else:
            #         self.plot.plot(x=[pos[0]], y=[pos[1]],  symbolBrush=(200, 0, 0), symbolPen='w',
            #                        symbol='t', symbolSize=14)


    def addonedefect(self, img, BIG_ID, ID, pos):
        self.list_defectwidgt.addOneDefectItemwithID(img, BIG_ID, ID)
        self.appenddefectpos(BIG_ID, pos)


    def recmsg(self, n_stationid, n_msgtype, tuple_data):
        dict_result = json.loads(tuple_data)
        img = self._getimage(dict_result)
        defects = dict_result['defect feature']
        ndefectnum = dict_result['defect num']
        for i in range(ndefectnum):
            singledefect = defects[i]
            pos = singledefect['pos']
            smallimg = self._expandimg(img, pos)
            self._appenddefect(self.nblockid, pos, img.shape[1], img.shape[0])
            self.addonedefect(smallimg, 0, i, pos[0:2])
        self._appendimg(img)
        self.nblockid += 1



    def _appenddefect(self, nid, pos, singleimgw, singleimgh):
        """
        把缺陷坐标放入list_pos,而且需要注意的是x
        :param nid:
        :param pos:
        :param singleimgw:
        :param singleimgh:
        :return:
        """
        x = int(nid / 2)
        y = int(nid % 2)
        print('x, y, ', x, y)
        nextend = 50
        list_pos = [0, 0, 0, 0]
        list_pos[0] = int(max(0, x * singleimgw + pos[0] - nextend) / self._SCALE_FACTOR)
        list_pos[1] = int(max(0, y * singleimgh + pos[1] - nextend) / self._SCALE_FACTOR)
        list_pos[2] = int((pos[2] + nextend*2) / self._SCALE_FACTOR)#没做保护但无所谓了
        list_pos[3] = int((pos[3] + nextend*2) / self._SCALE_FACTOR)
        self.list_defectpos.append(list_pos)



    def _appendimg(self, img):
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
            rotateimg = cv2.rotate(zeroimg, cv2.ROTATE_90_COUNTERCLOCKWISE)
            self.imgitem.setImage(rotateimg, autoLevels=False)
            self.list_img = []
            for nindex, pos in enumerate(self.list_defectpos):
                newpos = [pos[1], zeroimg.shape[1] - pos[0] - 1, pos[3], pos[2]]
                roi = ROIwithID(pos=newpos[:2], nid=nindex, size=newpos[2:], pen=1)
                roi.sigClicked.connect(self.test)
                roi.setAcceptedMouseButtons(QtCore.Qt.LeftButton)
                self.list_roi.append(roi)
                self.view.addItem(roi)

    def test(self, *args):
        print ('click', args)
        print ('clicked, ', args[0].nid)

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
        self.list_defectwidgt.clear()
        self.imgitem.clear()

#print ('hone')

class ROIwithID(pg.ROI):
    def __init__(self, pos, nid, **kwargs):
        pg.ROI.__init__(self, pos, **kwargs)
        self.nid = nid


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