from library.monitoring.BaseMonitoringWidget import KxBaseMonitoringWidget
from library.monitoring.BaseMonitoringWidget import registerkxmointorwidget
from PyQt5 import QtGui, QtWidgets, QtCore
from project.monitoring.ZSShowGridWidget import ZSImgListDetectWidget
import pyqtgraph as pg
from library.common.KxImageBuf import KxImageBuf
from project.monitoring.WidgePos import WidgetEdgePos
from library.common.readconfig import readconfig
from project.monitoring.WidgetShowResultSFC import CWidgetShowResultSFC
import imc_msg
import json
import numpy as np
import cv2
import time
from project.other.ExcelManager import CExcelManager
from project.other.globalparam import SYSPATH


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
        self.fp = None#检测结果
        self.fp_realtime = None#实时显示
        self.frameitem = None
        self.h_defectinfo = DefectClass()
        self.nrow, self.ncol = 0, 0# 大图拼成的行列
        self.n_curid = 0
        self._setexcelinfo(SYSPATH.PATH_CHECKDATA, "Sheet", ["SFC", "气泡数量", "气泡尺寸", "异物数量", "异物尺寸", "涂胶面积", "block面积", "异物面积", "气泡面积"])


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
        self.text_item = pg.TextItem(anchor=(0, 0), angle=0,color=(255, 255, 255))
        h_font = QtGui.QFont('Consolas', 14)
        self.text_item.setFont(h_font)

        self.graphicsView_small.addItem(self.text_item)
        self.verticallayout.addWidget(self.graphicsView_small, 1)

        self.widget_edgepos = WidgetEdgePos()
        self.list_defectwidgt = ZSImgListDetectWidget(self)

        self.widget_showresultsfc = CWidgetShowResultSFC()


        self.widget_right = QtWidgets.QWidget(self)
        self.verticallayout1 = QtWidgets.QVBoxLayout(self.widget_right)
        self.verticallayout1.addWidget(self.widget_showresultsfc, 1)
        self.verticallayout1.addWidget(self.widget_edgepos, 1)
        self.verticallayout1.addWidget(self.list_defectwidgt, 3)
        self.list_defectwidgt.setMaxGridNum(1000)
        self.list_defectwidgt.SigSelectDefect.connect(self._slotCellClick)
        self.horizonlayout.addWidget(self.widget_map, 2)
        self.horizonlayout.addWidget(self.widget_right, 3)
        self.horizonlayout.setContentsMargins(0, 0, 0, 0)


    def _setexcelinfo(self, exceldir_path, sheet_name, head):
        self.basedir = exceldir_path
        self.basesheet_name = sheet_name
        self.basehead = head
        self.curpath = time.strftime('%Y-%m-%d', time.localtime(time.time())) + ".xlsx"
        self.excel_checkdata = CExcelManager(self.basedir + "\\" + self.curpath, self.basesheet_name, self.basehead)
        self.timer = QtCore.QTimer(self)
        self.timer.timeout.connect(self.__timeout2createnewexccel)
        self.timer.start(1000 * 60 * 5)


    def __timeout2createnewexccel(self):
        """
        定时器五分钟更新一次当前日期，如果日期更新则改变文件名
        """
        curpath = time.strftime('%Y-%m-%d', time.localtime(time.time())) + ".xlsx"
        if curpath != self.curpath:
            self.curpath = curpath
            while 1:
                if not self.excel_checkdata.b_iswriting:  # 只要不是正在写入
                    self.excel_checkdata = CExcelManager(self.basedir + "\\" + self.curpath, self.basesheet_name,
                                                   self.basehead)
                    break
                else:
                    time.sleep(5)


    def _slotCellClick(self, img, ndefectID, pos):
        if ndefectID >= self.h_defectinfo.size():
            return
        else:
            self._paintroi(ndefectID)
        self.imgitem_small.setImage(img, autoLevels=False)
        self._showinfo(ndefectID)


    def _paintroi(self, ndefectID):
        lastroi = self.h_defectinfo.list_roi[self.n_curid]
        lastroi.setPen(1)
        roi = self.h_defectinfo.list_roi[ndefectID]
        roi.setPen(pg.mkPen(color=(255, 0, 0), width=3))
        self.n_curid = ndefectID


    def _getimage(self, fp, dict_result):
        try:
            readimagepath = dict_result['imagepath']
            startoffset = dict_result['startoffset']
            offsetlen = dict_result['imageoffsetlen']
        except AttributeError:
            return None
        if fp is None:
            try:
                fp = open(readimagepath, "rb")
            except IOError:
                return None

        fp.seek(startoffset)
        data = fp.read(offsetlen)
        Img = KxImageBuf()
        Img.unpack(data)
        arrImg = Img.Kximage2npArr()
        return arrImg


    def recmsg(self, n_stationid, n_msgtype, tuple_data):

        if n_msgtype == imc_msg.MSG_CHECK_RESULT:

            dict_result = json.loads(tuple_data)

            img = self._getimage(self.fp, dict_result)

            neww = int(img.shape[1] / self._SCALE_FACTOR)

            newh = int(img.shape[0] / self._SCALE_FACTOR)

            resizeimg = cv2.resize(img, (neww, newh))


            #nIsFull = int(dict_result['ISFULL'])

            ndefectnum = dict_result['defect num']

            ncurrow = dict_result['currow']

            ncurcol = dict_result['curcol']

            blockarea = round(int(dict_result['blockarea']) * self.f_resolution * self.f_resolution, 2)

            gulearea = round(int(dict_result['area']) * self.f_resolution * self.f_resolution, 2)

            defectarea = round(int(dict_result['defectarea']) * self.f_resolution * self.f_resolution, 2)

            qipaoarea = round(int(dict_result['qipaoarea']) * self.f_resolution * self.f_resolution, 2)


            print ("各个面积: ", blockarea, gulearea, defectarea)

            self.h_defectinfo.append_blockinfo(resizeimg, blockarea, gulearea, defectarea, qipaoarea, ncurrow,  ncurcol)

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

                    defecttype = int(singledefect['defecttype'])

                    if defecttype == 1:
                        sdefecttype = "气泡"
                    else:
                        sdefecttype = "异物"

                    self.h_defectinfo.append_defectinfo(sdefecttype, smallimg, newpos, sdefectid, ndots,
                                                        [round(pos[2] * self.f_resolution, 2), round(pos[3] * self.f_resolution, 2)],
                                                        round(ndots * self.f_resolution * self.f_resolution, 2))

                    self.list_defectwidgt.addOneDefectItemwithID(smallimg, self.h_defectinfo.size() - 1, sdefectid,
                                                                 pos[0:2], ndots * self.f_resolution * self.f_resolution)

        elif n_msgtype == imc_msg.MSG_CHECK_RESULT_FINISH:

            self.imgitem.setImage(self.h_defectinfo.getbigimg())

            #self._writedata2excel()

            self.widget_edgepos.setdata(self.h_defectinfo.list_narea, 0)

            self.widget_edgepos.setdata(self.h_defectinfo.list_ngulearea, 1)

            self.widget_edgepos.setdata(self.h_defectinfo.list_ndefectarea, 2)

            self.widget_edgepos.setdata(self.h_defectinfo.list_nqipaoarea, 3)


            for roi in self.h_defectinfo.list_roi:

                roi.sigClicked.connect(self._roiclicked)

                roi.setAcceptedMouseButtons(QtCore.Qt.LeftButton)

                self.view.addItem(roi)

            self.h_parentwidget.callback2uploaddata(1)#都弹框，确认是否放行


                #self.h_parentwidget.callback2autorun()
                #self.h_parentwidget.showmeswidget()

        elif n_msgtype == imc_msg.MSG_SHOW_IMG:

            dict_result = json.loads(tuple_data)

            showimg = self._getimage(self.fp_realtime, dict_result)

            self.imgitem.setImage(showimg, autoLevels=False)



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
        self._showinfo(nid)


    def _showinfo(self, nid):
        if nid > self.h_defectinfo.size():
            return
        s_word = "缺陷块缺陷类型：" + str(self.h_defectinfo.list_defecttype[nid]) + '\n'
        s_word = s_word + "缺陷块位置：" + str(self.h_defectinfo.list_defectword[nid]) + '\n'
        s_word = s_word + "缺陷宽：" + str(self.h_defectinfo.list_defectwh[nid][0]) + ' mm\n'
        s_word = s_word + "缺陷高：" + str(self.h_defectinfo.list_defectwh[nid][1]) + ' mm\n'
        s_word = s_word + "缺陷面积：" + str(self.h_defectinfo.list_defectarea[nid]) + ' mm²\n'
        s_word = s_word + "缺陷点数：" + str(self.h_defectinfo.list_dots[nid]) + '\n'

        self.text_item.setText(s_word)


    def _expandimg(self, bigimg, pos, nexpand=50):
        size = bigimg.shape[0:2]
        x = max(0, pos[0] - nexpand)
        y = max(0, pos[1] - nexpand)
        xend = min(size[1], pos[0] + pos[2] + nexpand)
        yend = min(size[0], pos[1] + pos[3] + nexpand)
        smallimg = bigimg[y:yend, x:xend, :]
        return smallimg, [x, y ,xend - x + 1, yend - y + 1]

    def setsfc(self, sfc):
        self.widget_showresultsfc.setsfc(sfc)

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
        self.imgitem_small.clear()
        self.widget_showresultsfc.clear()
        #self.view.autoRange()
        #self.view_small.autoRange()


    def _writedata2excel(self):
        """
        往excel表格里写数据
        """
        #["SFC", "气泡数量", "气泡尺寸", "异物数量", "异物尺寸", "涂胶面积", "block面积", "异物面积", "气泡面积"]

        sfc = self.h_parentwidget.getspackid()

        list_qipao = []
        list_yiwu = []

        list_defect = self.h_defectinfo.list_defectarea
        list_defecttype = self.h_defectinfo.list_defecttype
        for nindex, stype in enumerate(list_defecttype):
            if stype == "气泡":
                list_qipao.append(list_defect[nindex])
            else:
                list_yiwu.append(list_defect[nindex])
        list_gulearea = self.h_defectinfo.list_ngulearea
        list_blockarea = self.h_defectinfo.list_narea
        list_yiwuarea = self.h_defectinfo.list_ndefectarea
        list_qipaoarea = self.h_defectinfo.list_nqipaoarea

        self.excel_checkdata.writeExcel([[sfc, str(len(list_qipao)), str(list_qipao), str(len(list_yiwu)), str(list_yiwu),
                                         str(list_gulearea), str(list_blockarea), str(list_yiwuarea), str(list_qipaoarea)]])




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
        self.list_defectwh = [] #缺陷宽高
        self.list_defectarea = []#缺陷面积
        self.list_defectword = []#缺陷位置名称
        self.list_defecttype = []#缺陷类型
        self.list_bigimg = []   #每个块原图
        self.list_narea = []    #每个块面积
        self.list_ndefectarea = []#异物面积
        self.list_nqipaoarea = []#气泡面积
        self.list_ngulearea = []#涂胶面积
        self.nrow = 0#大图是由这么多行列组成
        self.ncol = 0
        self.bigimg = None


    def setrowcol(self, row, col):
        self.nrow = row
        self.ncol = col


    def append_defectinfo(self, sdefecttype, defectimg, pos, sdefectword, dots, list_wh, ndefectarea):
        """
        存储检测结果，针对单个缺陷
        :param sdefecttype:   缺陷类型
        :param defectimg:   单张缺陷图
        :param pos:         单个缺陷坐标
        :param sdefectword: 缺陷名称（格式： 块号_缺陷号）
        :param dots:        缺陷点数
        :param list_wh:     缺陷宽高
        :param ndefectarea: 缺陷面积
        :return:
        """
        self.list_defecttype.append(sdefecttype)
        self.list_defectimg.append(defectimg)
        self.list_defectpos.append(pos)
        self.list_dots.append(dots)
        roi = ROIwithID(pos=pos[:2], nid=len(self.list_defectpos) - 1, word=sdefectword, size=pos[2:], pen=1)
        self.list_roi.append(roi)
        self.list_defectwh.append(list_wh)
        self.list_defectarea.append(ndefectarea)
        self.list_defectword.append(sdefectword)


    def append_blockinfo(self, bigimg, narea, ngulearea, ndefectarea, nqipaoarea, nrowid, ncolid):
        """
        存储检测结果，针对单个block的信息
        :param bigimg: block原图压缩图
        :param narea:  block面积
        :param ndefectarea:  缺陷面积
        :param ngulearea:  涂胶区域面积
        :param nqipaoarea: 气泡面积
        :param nrowid:  当前图是大图的第几行
        :param ncolid:  当前图是大图的第几列
        :return:
        """
        h, w, c = bigimg.shape

        if len(self.list_bigimg) == 0:
            self.bigimg = np.zeros((h * self.nrow, w * self.ncol, c), np.uint8)

        self.list_bigimg.append(bigimg)
        self.list_narea.append(narea)
        self.list_ndefectarea.append(ndefectarea)
        self.list_ngulearea.append(ngulearea)
        self.list_nqipaoarea.append(nqipaoarea)
        self.bigimg[nrowid*h:(nrowid + 1) * h, ncolid*w:(ncolid + 1) * w] = bigimg



    def size(self):
        return len(self.list_defectpos)


    def getbigimg(self):
        if self.bigimg is None:
            return np.zeros((100, 100, 3), np.uint8)
        rotateimg = cv2.rotate(self.bigimg, cv2.ROTATE_90_COUNTERCLOCKWISE)
        ncurrow = self.ncol
        ncurcol = self.nrow

        h, w = rotateimg.shape[:2]
        nsteph = int(h / ncurrow)
        nstepw = int(w / ncurcol)

        for i in range(ncurrow):
            for j in range(ncurcol):
                pos = (j * nstepw, i * nsteph + 300)
                word = str((ncurrow - i - 1) * ncurcol + j)
                rotateimg = cv2.putText(rotateimg, word, pos, cv2.FONT_HERSHEY_SIMPLEX, 6, (0, 255, 0), 10)
        return rotateimg


    def clear(self):
        self.list_defectimg = []#小缺陷图
        self.list_defectpos = []#缺陷坐标
        self.list_dots = []     #缺陷点数
        self.list_roi = []      #缺陷roi对象
        self.list_bigimg = []   #每个块原图
        self.list_narea = []    #每个块面积
        self.list_ndefectarea = []#异物面积
        self.list_ngulearea = []#涂胶面积
        self.list_defectwh = [] #缺陷宽高
        self.list_defectarea = []#缺陷面积
        self.list_defectword = []#缺陷位置名称
        self.list_nqipaoarea = []





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