# -*- coding: utf-8 -*-
from PyQt5 import QtCore, QtGui
import struct
import sys
#为了文件的读写设置，python2不加这两句保存或者读文件时会出现解码编码问题

sys.path.append("../../../")
import xmltodict
import json
import cv2
import numpy as np
from kxpyqtgraph.kxparameterTree.KxCustomWidget import *
from library.parametersetting.ParamItemPY.KxBaseWidget import KxBaseParamWidget, registerkxwidget
from kxpyqtgraph.kxparameterTree.KxParameter import KxParameter
from UI.ui_kxglobal1 import Ui_ParamPYLoadWidget
from library.ipc import ipc_tool
import imc_msg
from project.other.globalparam import StaticConfigParam
from library.common.KxImageBuf import KxImageBuf
from project.param.MergeImg import CMergeImg, CMergeImgToList
from PIL import Image
from project.param.WaitDialogWithText import WaitDialogWithText
import  logging
# from pyqtgraph.imageview.ImageView import ImageView

#节拍  分析
#

class BuildStatus:#建模状态
    STATUS_INIT = 0
    STATUS_ALL = 1
    STATUS_SECOND = 2

class GuleParam(KxBaseParamWidget):
    """
              涂胶项目
    """
    _SCAN_MAP_TIMES = 3 #全地图扫描次数
    _MAX_ROI_NUM = 32
    _MAX_SCAN_NUM = 6
    _BUILD_MODEL_SCALE_FACTOR = 4#建模的时候对图像进行压缩，不然会卡顿
    def __init__(self, h_parentwidget, n_uid, n_areanum, n_stationid):
        KxBaseParamWidget.__init__(self,n_uid, n_areanum, n_stationid)
        self.h_parent = h_parentwidget
        self.ui = Ui_ParamPYLoadWidget()
        self.ui.setupUi(self)
        self._initui()
        self._initparam()
        self.fp = None
        self.h_mergeobj = CMergeImg()
        self.h_mergelistobj = CMergeImgToList()
        self.build_status = BuildStatus.STATUS_INIT
        self.list_img = []
        self.h_bigimage = None
        self.threadWaitDialog = None
        self._connectlog()
        #self.ui.h_pBtLoad.clicked.connect(self.loadimg)


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
        self.n_measurehighnum = 0#测量高度的点
        self.n_checkarea = 0#当前检测区域
        dict_head = {'name': u'主站设置', 'type': 'group', 'visible':False, 'children': [
                {'name': u'图像信息', 'type': 'imageinfo',
                 'value': {"isShow": True}, "infovisible": True},
            ]}
        list_path = [{'name': '底板路径' + str(i), 'type': 'str'} for i in range(self._MAX_SCAN_NUM)]
        dict_head['children'].extend(list_path)
        self.params.append(dict_head)
        self.params.append(self._append_fangdaifangcuo_point())
        self.params.extend([
            {'name': '全局拍摄控制', 'type': 'group', 'children':[
                {'name': '相机横向像素数', 'type': 'int', 'value': 8192, 'limits': [1, 8192]},
                {'name': '相机横向分辨率', 'type': 'float', 'value': 0.1, 'limits': [0, 1]},
                {'name': '相机纵向像素数', 'type': 'int', 'value': 2000, 'limits': [200, 6600]},
                {'name': '拍摄长度', 'type': 'int', 'value': 2000, 'limits': [0, StaticConfigParam.MAX_Y_LEN]},
                {'name': '起拍位置', 'type': 'int', 'value': 0, 'limits':[0, 2000]},
                {'name': '拍摄组数', 'type': 'int', 'value': 3, 'limits': [1, self._MAX_SCAN_NUM]},
                {'name': '横向重叠区域', 'type': 'int', 'value': 1, 'limits': [1, 2000]},
                {'name': '全局取图', 'type': 'action'},
                {'name': '扫描区域取图', 'type': 'action'},
            ]},
            {'name': '建模图像缩放系数', 'type': 'str', 'value':str(self._BUILD_MODEL_SCALE_FACTOR),'visible':False,
             'readonly':True },
            {'name': u'检测区域数量', 'type': 'int', 'value': 0, 'step': 1, 'limits': (0, self._MAX_ROI_NUM)},
            {'name': u'显示图像', 'type':'list', 'values': {'组数一': 0, '组数二': 1, '组数三':2, '组数四':3,
                                                        '组数五':4, '组数六':5}},
            {'name': '检测参数', 'type': 'group', 'children': [
                {'name': '检高灵敏度', 'type': 'int', 'value': 3, 'limits': [0, 25]},
                {'name': '检低灵敏度', 'type': 'int', 'value': 3, 'limits': [0, 25]},
            ]},
            {'name': '拼图信息', 'type': 'group', 'visible':False,'children': [
                {'name': '行数', 'type': 'int', 'value': 0, 'limits': [0, self._MAX_SCAN_NUM]},
                {'name': '列数', 'type': 'int', 'value': 0, 'limits': [0, self._MAX_SCAN_NUM]},
                {'name': '最大检测框高', 'type': 'str', 'value': ''},
            ]},
        ])

        self._append_scan_area(self.params)
        self._append_checkarea_param(self.params)
        self.p = KxParameter.create(name='params', type='group', children=self.params)
        self.h_parameterTree.setParameters(self.p, showTop=False)
        self.p.param(u'主站设置', u'图像信息').add2view(self.ui.h_gVShowRealImg, self.h_imgitem)
        for nindex in range(self._MAX_ROI_NUM):
             self.p.param(u'检测区域' + str(nindex), u'检测区域').add2view(self.view)
        for nindex in range(self._MAX_SCAN_NUM):
             self.p.param('扫描区域', '扫描区域' + str(nindex)).add2view(self.view)
        for nindex in range(self._MAX_SCAN_NUM):
             self.p.param('扫描区域', '匹配位置' + str(nindex)).add2view(self.view)
        self._initsignal()


    def _connectlog(self):
        self.p.param("全局拍摄控制").sigTreeStateChanged.connect(self._logparamchange)
        self.p.param("检测参数").sigTreeStateChanged.connect(self._logparamchange)
        self.p.param("检测区域数量").sigTreeStateChanged.connect(self._logparamchange)
        self.p.param("防呆防错设置").sigTreeStateChanged.connect(self._logparamchange)
        for i in range(0, self._MAX_ROI_NUM):
            self.p.param('检测区域' + str(i)).sigTreeStateChanged.connect(self._logparamchange)


    def _disconnectlog(self):
        self.p.param("全局拍摄控制").sigTreeStateChanged.disconnect(self._logparamchange)
        self.p.param("检测参数").sigTreeStateChanged.disconnect(self._logparamchange)
        self.p.param("检测区域数量").sigTreeStateChanged.disconnect(self._logparamchange)
        self.p.param("防呆防错设置").sigTreeStateChanged.disconnect(self._logparamchange)
        for i in range(0, self._MAX_ROI_NUM):
            self.p.param('检测区域' + str(i)).sigTreeStateChanged.disconnect(self._logparamchange)


    def _logparamchange(self, *args):
        s_log = args[0].name() + "--" + args[1][0][0].name() +\
                " 由 " + str(args[1][0][0].defaultValue()) + "改为 " + str(args[1][0][2])
        ipc_tool.kxlog("main", logging.INFO, s_log)


    def _initsignal(self):
        self.p.param('检测区域数量').sigValueChanged.connect(self._add_checkarea)
        self.p.param('全局拍摄控制', '全局取图').sigActivated.connect(self._captureimg)
        self.p.param('全局拍摄控制', '扫描区域取图').sigActivated.connect(self._captureimg_second)
        self.p.param('显示图像').sigValueChanged.connect(self._changeshowimg)
        self.p.param('全局拍摄控制', '横向重叠区域').sigValueChanged.connect(self._adjustbigimgsize)
        self.p.param('防呆防错设置', '基准点数量').sigValueChanged.connect(self._addpointparam)
        self.p.param('防呆防错设置', '标定基准高度').sigActivated.connect(self._calibrate_highsensor_point)


    def _addpointparam(self, *even):
        if int(even[1]) > self.n_measurehighnum:
            for n_i in range(self.n_measurehighnum, int(even[1])):
                self.p.param('防呆防错设置','防呆防错点' + str(n_i)).show()
        else:
            for n_i in range(int(even[1]), self.n_measurehighnum):
                self.p.param('防呆防错设置', '防呆防错点' + str(n_i)).hide()
        self.n_measurehighnum = int(even[1])


    def _add_checkarea(self, *even):
        if int(even[1]) > self.n_checkarea:
            for n_i in range(self.n_checkarea, int(even[1])):
                self.p.param('检测区域' + str(n_i)).show()
                self.p.param('检测区域' + str(n_i), '检测区域').isShow(True)
        else:
            for n_i in range(int(even[1]), self.n_checkarea):
                self.p.param('检测区域' + str(n_i)).hide()
                self.p.param('检测区域' + str(n_i), '检测区域').isShow(False)
        self.n_checkarea = int(even[1])


    def _append_checkarea_param(self, dict_params):
        list_standardschildrenitems = []
        for i in range(0, self._MAX_ROI_NUM):
            list_standardschildrenitems.append(
                {'name': '检测区域' + str(i), 'type': 'group', 'expanded': False, 'visible': False, 'children': [
                    {'name': u'检测区域', 'type': 'roiwithtext', 'value': {"isShow": False, "pos": u"0,0,100,100"},
                      "roi_opt": {"word": '检测区域'+ str(i), "scaleable": True, 'pen':1}, "infovisible": False},
                    {'name': u'扫描组号', 'type': 'list', 'values': {'组数一': 0, '组数二': 1, '组数三':2, '组数四':3,
                                                        '组数五':4, '组数六':5}},
                    {'name': u'当前组ID', 'type': 'int', 'value': 0, 'limits':[0, 6], 'readonly':True},
                    {'name': '提取异物灰度', 'type': 'int', 'value': 100, 'limits': [0, 255]},
                    {'name': '异物最小点数', 'type': 'int', 'value': 20, 'limits': [0, 20000]},
                ]}
            )
        dict_params.extend(list_standardschildrenitems)


    def _append_scan_area(self, dict_params):

        list_standardschildrenitems = []
        for i in range(0, self._MAX_SCAN_NUM):
            list_standardschildrenitems.append(
                {'name': u'扫描区域'+str(i), 'type': 'roiwithtext', 'value': {"isShow": False, "pos": u"0,0,100,100"},
                 "roi_opt": {"word": '扫描区域'+str(i), "scaleable": True, 'pen': 3}, "infovisible": False},
            )

        for i in range(0, self._MAX_SCAN_NUM):
            list_standardschildrenitems.append(
                {'name': '扫描区域%d图像数量'%i, 'type' : "str", 'value':'12'},
            )

        for i in range(0, self._MAX_SCAN_NUM):
            list_standardschildrenitems.append(
                {'name': u'匹配位置' + str(i), 'type': 'roiwithtext', 'value': {"isShow": False, "pos": u"0,0,100,100"},
                 "roi_opt": {"word": '匹配位置' + str(i), "scaleable": True, 'pen': 2}, "infovisible": False},
            )

        dict_scan = {'name': '扫描区域', 'type': 'group', 'visible':False, 'children':list_standardschildrenitems}

        dict_params.append(dict_scan)


    def _append_fangdaifangcuo_point(self):
        list_child = [{'name': '基准点数量', 'type': 'int', 'value': 0, 'limits': [0, 6]}]

        for i in range(6):
            dict_child = {'name': '防呆防错点%d'%i, 'type': 'group','expanded':False, 'visible':False, 'children': [
            {'name': 'X位置', 'type': 'int', 'value': 0, 'limits': [0, StaticConfigParam.MAX_X_LEN]},
            {'name': 'Y位置', 'type': 'int', 'value': 0, 'limits': [0, StaticConfigParam.MAX_Y_LEN]},
            {'name':'基准值', 'type': 'str', 'value' : '0', 'readonly':True},
            ]}
            list_child.append(dict_child)

        list_child.append({'name': '标定基准高度', 'type': 'action'})

        dict_result = {'name': '防呆防错设置', 'type': 'group', 'children': list_child}

        return dict_result


    def _captureimg(self):
        """
        全局队列将参数送到界面，控制第一次全局拍照参数
        :return:
        """
        # self.threadWaitDialog = WaitDialogWithText('正在建模，请稍候...')
        # self.threadWaitDialog.clear()
        # self.threadWaitDialog.setProcessBarRange(0, 100)
        # self.threadWaitDialog.show()
        #QtCore.QCoreApplication.processEvents(QtCore.QEventLoop.ExcludeUserInputEvents)
        nStartX = int(self.p.param('全局拍摄控制', '起拍位置').value())
        # ndisX = int(int(self.p.param('全局拍摄控制', '相机横向像素数').value()) *
        #             float(self.p.param('全局拍摄控制', '相机横向分辨率').value()))
        ndisY = int(self.p.param('全局拍摄控制', '拍摄长度').value())
        nXtimes = self._SCAN_MAP_TIMES

        imgH = int(self.p.param('全局拍摄控制', '相机纵向像素数').value())
        imgW = int(self.p.param('全局拍摄控制', '相机横向像素数').value())

        if nXtimes > 1:
            ndisX = (StaticConfigParam.MAX_X_LEN - nStartX) / (nXtimes - 1)
        else:
            ndisX = StaticConfigParam.MAX_X_LEN - nStartX

        n_firstbuild_imgnum = (ndisY * StaticConfigParam.DIS2PIXEL) / imgH + 1

        ndisY = min((n_firstbuild_imgnum + 1) * imgH /  StaticConfigParam.DIS2PIXEL, StaticConfigParam.MAX_Y_LEN )#当取到最长的时候这个地方就起作用

        n_firstbuild_imgnum = int(ndisY * StaticConfigParam.DIS2PIXEL / imgH)

        nbigimgH = int(n_firstbuild_imgnum * imgH / self._BUILD_MODEL_SCALE_FACTOR)

        nbigimgW = int(imgW / self._BUILD_MODEL_SCALE_FACTOR) * nXtimes

        self.h_mergeobj.clear()

        #print("全局取图： ", ndisY, n_firstbuild_imgnum, nbigimgW, nbigimgH)

        self.h_mergeobj.initinfo(n_firstbuild_imgnum, nbigimgW, nbigimgH)

        self.build_status = BuildStatus.STATUS_ALL

        ipc_tool.getqueue_processedData().put((-1, imc_msg.MSG_BUILD_MODEL, [nStartX, ndisX, ndisY, nXtimes]))


    def _captureimg_second(self):
        """
        第二次采集全局参数，根据roi框的位置进行拍摄
        :return:
        """
        # self.threadWaitDialog.clear()
        # self.threadWaitDialog.setProcessBarRange(0, 100)
        # self.threadWaitDialog.show()
        #QtCore.QCoreApplication.processEvents(QtCore.QEventLoop.ExcludeUserInputEvents)

        nXtimes = int(self.p.param('全局拍摄控制', '拍摄组数').value())

        nStartX = int(self.p.param('全局拍摄控制', '起拍位置').value())

        imgW = int(self.p.param('全局拍摄控制', '相机横向像素数').value())

        imgH = int(self.p.param('全局拍摄控制', '相机纵向像素数').value())

        nmiddleoffset = int(imgW / 2 / self._BUILD_MODEL_SCALE_FACTOR)

        # 二次建模，对一次建模的roi进行隐藏，只需记录结果即可
        for n_i in range(int(self._MAX_SCAN_NUM)):

            self.p.param('扫描区域', '扫描区域' + str(n_i)).isShow(False)

        list_posx = []

        list_x = []

        list_y = []

        list_buildimgnum = []

        list_bigimgH = []

        for i in range(nXtimes):

            list_posx.append(self.p.param('扫描区域', '扫描区域' + str(i)).get_list_pos())

        for roipos in list_posx:

            list_x.append(int(max(0, int((roipos[0] + roipos[2]) / 2 - nmiddleoffset)) * self._BUILD_MODEL_SCALE_FACTOR / StaticConfigParam.DIS2PIXEL + nStartX))

            list_y.append(int(roipos[3]) * self._BUILD_MODEL_SCALE_FACTOR)

        # list_y记录y轴移动距离，但是需要考虑图像每次拍摄取整以及可能丢步的问题

        for nindex, y in enumerate(list_y):

            n_build_imgnum = int(y / imgH) + 1

            ndisY = min(int(n_build_imgnum * imgH / StaticConfigParam.DIS2PIXEL) + StaticConfigParam.RUN_MORE_DIS, StaticConfigParam.MAX_Y_LEN)

            n_build_imgnum = int(ndisY * StaticConfigParam.DIS2PIXEL / imgH) #当取到最长的时候这个地方就起作用

            list_y[nindex] = ndisY

            list_buildimgnum.append(n_build_imgnum)

            list_bigimgH.append(int(n_build_imgnum * int(imgH / self._BUILD_MODEL_SCALE_FACTOR)))

            self.p.param("扫描区域", "扫描区域%d图像数量"%nindex).setValue(n_build_imgnum)

        self.h_mergelistobj.clear()

        self.h_mergelistobj.initinfo(list_buildimgnum, int(imgW / self._BUILD_MODEL_SCALE_FACTOR), list_bigimgH)

        self.build_status = BuildStatus.STATUS_SECOND

        ipc_tool.getqueue_processedData().put((-1, imc_msg.MSG_BUILD_MODEL_SECOND, [list_x, list_y]))


    def call2back2getcaptureinfo(self):

        nXtimes = int(self.p.param('全局拍摄控制', '拍摄组数').value())

        nStartX = int(self.p.param('全局拍摄控制', '起拍位置').value())

        imgH = int(self.p.param('全局拍摄控制', '相机纵向像素数').value())

        nmiddleoffset = int(int(self.p.param('全局拍摄控制', '相机横向像素数').value()) / 2 / self._BUILD_MODEL_SCALE_FACTOR)

        # 二次建模，对一次建模的roi进行隐藏，只需记录结果即可
        for n_i in range(int(nXtimes)):
            self.p.param('扫描区域', '扫描区域' + str(n_i)).isShow(False)

        list_posx = []

        list_x = []

        list_y = []

        for i in range(nXtimes):
            list_posx.append(self.p.param('扫描区域', '扫描区域' + str(i)).get_list_pos())

        for roipos in list_posx:
            list_x.append(int(int(
                max(0, (roipos[0] + roipos[
                    2]) / 2 - nmiddleoffset)) * self._BUILD_MODEL_SCALE_FACTOR / StaticConfigParam.DIS2PIXEL + nStartX))

            list_y.append(int(roipos[3]) * self._BUILD_MODEL_SCALE_FACTOR)

        # list_y记录y轴移动距离，但是需要考虑图像每次拍摄取整以及可能丢步的问题

        for nindex, y in enumerate(list_y):
            n_build_imgnum = int(y / imgH) + 1

            # ndisY = min(int((n_build_imgnum + 1) * imgH / StaticConfigParam.DIS2PIXEL), StaticConfigParam.MAX_Y_LEN)
            ndisY = min(int(n_build_imgnum * imgH / StaticConfigParam.DIS2PIXEL) + StaticConfigParam.RUN_MORE_DIS,
                        StaticConfigParam.MAX_Y_LEN)

            list_y[nindex] = ndisY

        return [list_x, list_y]


    def _calibrate_highsensor_point(self):
        """
        标定基准高度值，通过公共队列往主界面传输点坐标，点坐标收到数据之后启动线程进行测量扫描，
        扫描之后记录点值回调 callback2set_highsensor_point 将测量点结果送回
        :return:
        """
        nnum = int(self.p.param('防呆防错设置', '基准点数量').value())

        if nnum > 0:

            list_pos = []

            for nindex in range(nnum):

                x = int(self.p.param('防呆防错设置', '防呆防错点%d'%nindex, 'X位置').value())

                y = int(self.p.param('防呆防错设置', '防呆防错点%d'%nindex, 'Y位置').value())

                list_pos.append([x, y])

            ipc_tool.getqueue_processedData().put((-1, imc_msg.MSG_GET_BASE_POINT_HIGH, list_pos))


    def callback2set_highsensor_point(self, list_param):

        for nindex, data in enumerate(list_param):

            self.p.param('防呆防错设置', '防呆防错点%d' % nindex, '基准值').setValue(str(int(data)))



    def recmsg(self, n_stationid, n_msgtype, tuple_data):
        '''
        接收子站发送过来的消息
        '''
        if n_msgtype == imc_msg.MSG_BUILD_MODEL_IMG:
            if self.build_status == BuildStatus.STATUS_ALL:
                self._ReceiveBuildModelImg(tuple_data)
            elif self.build_status == BuildStatus.STATUS_SECOND:
                self._ReceiveBuilModelImgSecond(tuple_data)


    def loadparameters(self):
        self._disconnectlog()

        super(GuleParam, self).loadparameters()

        self.list_img = []

        list_path = []

        nXtimes = int(self.p.param('全局拍摄控制', '拍摄组数').value())

        for i in range(nXtimes):

            list_path.append(self.p.param('主站设置', '底板路径' + str(i)).value())

        for imgpath in list_path:

            if os.path.isfile(imgpath):

                with open(imgpath, 'rb') as curfp:

                    imageori = copy.copy(Image.open(curfp))

                    self.list_img.append(np.array(imageori))
            else:
                self.list_img.append(None)

        for nindex, img in enumerate(self.list_img):

            if img is not None:

                self.h_imgitem.setImage(img, autoLevels=False)

                self.p.param('显示图像').setValue(nindex)

        self.n_checkarea = int(self.p.param('检测区域数量').value())

        self._connectlog()


    def saveparameters(self):
        self._sortcheckpos()
        self._saveMapColRow()
        self._setbaseimgpath()
        for nindex, img in enumerate(self.list_img):
            if img is not None:
                image = np.array(img)
                imagesave = Image.fromarray(image)
                szSaveDir = self.p.param("主站设置", "底板路径" + str(nindex)).value()
                imagesave.save(szSaveDir)
        super(GuleParam, self).saveparameters()


    def _sortcheckpos(self):
        """
        同一扫描列的检测框，必须按照检测框号小的在上，检测框号大的在下
        :return:
        """
        nchecknum = int(self.p.param('检测区域数量').value())
        list_pos = []
        list_obj = []
        ncurcol = -1
        for i in range(nchecknum):

            ncol = int(self.p.param('检测区域' + str(i), '扫描组号').value())

            if ncurcol != ncol:

                ncurcol = ncol

                if list_pos != []:

                    array = np.array(list_pos).T

                    print (list_pos)

                    print(array)

                    list_sortindex = sorted(range(len(array[1])), key=lambda k: array[0][k], reverse=True)

                    print (list_sortindex)

                    print ('---------')

                    for nindex in range(len(list_pos)):

                        list_obj[nindex].set_list_pos(list_pos[list_sortindex[nindex]])

                    list_pos = []

                    list_obj = []

            list_pos.append(self.p.param('检测区域'+ str(i), '检测区域' ).get_list_pos())

            list_obj.append(self.p.param('检测区域'+ str(i), '检测区域' ))






    def _saveMapColRow(self):
        """
        根据扫描列数以及检测框中属于当前列的数量，得到一张全地图应该是N*M的组成，并且取最大的ROI作为图像大小
        :return:
        """
        # 1. 确定图像最大
        nchecknum = int(self.p.param('检测区域数量').value())
        if nchecknum ==0 :
            return
        ncolnum = int(self.p.param('全局拍摄控制', '拍摄组数').value())
        list_col = [0 for i in range(ncolnum)]
        list_h = []
        for i in range(nchecknum):
            roipos = self.p.param('检测区域'+ str(i), '检测区域' ).get_list_pos()
            list_h.append(roipos[3] - roipos[1] + 1)
            ncol = int(self.p.param('检测区域' + str(i), '扫描组号').value())
            if ncol < len(list_col):
                list_col[ncol] += 1

        nmaxrow = max(list_col)
        print (list_h)
        nmaxh = max(list_h) * self._BUILD_MODEL_SCALE_FACTOR

        self.p.param('拼图信息', '行数').setValue(nmaxrow)
        self.p.param('拼图信息', '列数').setValue(ncolnum)
        self.p.param('拼图信息', '最大检测框高').setValue(nmaxh)

        #2. 设置每个检测区域的组ID，表示这个roi在当前组中属于第几个ID，便于显示检测结果的时候拼图
        ncurcol = -1
        ncurid = 0
        for i in range(nchecknum):
            ncol = int(self.p.param('检测区域' + str(i), '扫描组号').value())
            if ncurcol != ncol:
                ncurcol = ncol
                ncurid = 0
            self.p.param('检测区域' + str(i), '当前组ID').setValue(ncurid)
            ncurid += 1

    def _changeshowimg(self, *even):
        nindex = int(even[1])

        if nindex < len(self.list_img):

            self.h_imgitem.setImage(self.list_img[nindex])

            for i in range(self._MAX_SCAN_NUM):

                self.p.param('扫描区域', "匹配位置" + str(i)).isShow(False)

            self.p.param('扫描区域', "匹配位置" + str(nindex)).isShow(True)

            nchecknum = int(self.p.param('检测区域数量').value())

            for i in range(nchecknum):

                if int(self.p.param('检测区域' + str(i), '扫描组号').value()) != nindex:

                    self.p.param('检测区域' + str(i), '检测区域').isShow(False)

                else:

                    self.p.param('检测区域' + str(i), '检测区域').isShow(True)


    def _setbaseimgpath(self):
        list_str = self.str_filedirectory.split('\\')
        for i in range(self._MAX_SCAN_NUM):
            list_str[-1] = str(i) + '.bmp'
            imgpath = "\\".join(list_str)
            self.p.param('主站设置', '底板路径' + str(i)).setValue(imgpath)


    def _ReceiveBuildModelImg(self, tuple_data):
        dict_result = json.loads(tuple_data)
        img = self._getimage(dict_result)
        newsize = (int(img.shape[1] / self._BUILD_MODEL_SCALE_FACTOR), int(img.shape[0] / self._BUILD_MODEL_SCALE_FACTOR))
        resizeimg = cv2.resize(img, newsize)
        self.h_mergeobj.merge(resizeimg)


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


    def _ReceiveBuilModelImgSecond(self, tuple_data):
        """
        接收图像进行二次建模。且图像根据扫描组数变为N张，可进行界面选择切换
        :param tuple_data:
        :return:
        """
        dict_result = json.loads(tuple_data)
        img = self._getimage(dict_result)
        newsize = (int(img.shape[1] / self._BUILD_MODEL_SCALE_FACTOR), int(img.shape[0] / self._BUILD_MODEL_SCALE_FACTOR))
        resizeimg = cv2.resize(img, newsize)
        self.h_mergelistobj.merge(resizeimg)


    def callback2changecol(self):
        self.h_mergeobj.IncreaseCol()

        # self.threadWaitDialog.setProcessBarVal(20)



    def callback2showbigimg(self):
        self.h_imgitem.setImage(self.h_mergeobj.bigimg)

        self.h_bigimage = self.h_mergeobj.bigimg

        nXtimes = int(self.p.param('全局拍摄控制', '拍摄组数').value())

        for n_i in range(int(nXtimes)):

            self.p.param('扫描区域', '扫描区域' + str(n_i)).isShow(True)

        #QtCore.QCoreApplication.processEvents(QtCore.QEventLoop.AllEvents)
        # print("self.threadWaitDialog.setProcessBarVal(60)")
        #
        # self.threadWaitDialog.setProcessBarVal(60)
        #
        # self.threadWaitDialog.close()


    def callback2judgeisfull(self):
        return self.h_mergeobj.IsFull()


    def callback2changecol_second(self):
        self.h_mergelistobj.IncreaseCol()

        #self.threadWaitDialog.setProcessBarVal(20)


    def callback2showbigimg_second(self):
        self.list_img = self.h_mergelistobj.list_bigimg

        self.h_imgitem.setImage(self.list_img[0])

        self.p.param("显示图像").setValue(0)

        self.p.param('扫描区域', "匹配位置0").isShow(True)



        #self.threadWaitDialog.close()



    def callback2judgeisfull_second(self):
        return self.h_mergelistobj.IsFull()


    def _adjustbigimgsize(self):
        if self.h_bigimage is not None:
            nXtimes = int(self.p.param('全局拍摄控制', '拍摄组数').value())
            nOversize = int(self.p.param('全局拍摄控制', '横向重叠区域').value())
            h, w, c = self.h_bigimage.shape
            nsingleimgw = int(w / nXtimes)
            list_w = []
            list_offset_src = []
            list_offset_dst = []
            for i in range(nXtimes):
                if i == 0:
                    list_offset_src.append(0)
                    list_offset_dst.append(0)
                else:
                    list_offset_src.append(nsingleimgw * i + nOversize)
                    ncurw = int(np.sum(np.array(list_w)))
                    list_offset_dst.append(ncurw)

                if i == 0 or i == nXtimes - 1:
                    list_w.append(int(nsingleimgw - nOversize))
                else:
                    list_w.append(int(nsingleimgw - nOversize * 2 ))
            nbigw = int(np.sum(np.array(list_w)))
            newimg = np.zeros((h, nbigw, c), np.uint8)
            for i in range(nXtimes):
                newimg[:, list_offset_dst[i]:list_offset_dst[i] + list_w[i]] \
                    = self.h_bigimage[:, list_offset_src[i]:list_offset_src[i] + list_w[i]]
            self.h_imgitem.setImage(newimg, autoLevels=False)

    def callback2getrowcol(self):
        nrow = self.p.param('拼图信息', '行数').value()
        ncol = self.p.param('拼图信息', '列数').value()
        return nrow, ncol

    def callback2getlisthead(self):
        nchecknum = int(self.p.param('检测区域数量').value())

        list_shead = []

        for i in range(nchecknum):

            list_shead.append("检测区域%d"%i)

        return list_shead



    # def loadimg(self):
    #     file_name = QtWidgets.QFileDialog.getOpenFileName(self, "open file dialog", "D://card",
    #                                                       "bmp files(*.bmp)")
    #     print(file_name)
    #     image = cv2.imread(file_name[0], 1)
    #     self.h_imgitem.setImage(image, autoLevels=False)


registerkxwidget(name='GuleParam', cls=GuleParam, override=True)


if __name__ == '__main__':
    app = QtWidgets.QApplication([])
    w = GuleParam(None, 1, 2 ,3)
    w.show()
    app.exec_()