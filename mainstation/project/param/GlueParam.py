# -*- coding: utf-8 -*-
import sys
#为了文件的读写设置，python2不加这两句保存或者读文件时会出现解码编码问题

sys.path.append("../../../")
import json
import cv2
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
from project.other.WaitDialogWithText import WaitDialogWithText
import  logging
import threading
from library.common.readconfig import readconfig, MAINSTATION_CONFIG
#节拍  分析
#

class BuildStatus:#建模状态
    STATUS_INIT = 0
    STATUS_ALL = 1
    STATUS_SECOND = 2
    STATUS_SIMULATE = 3#载入图像进行的模拟建模状态

class GuleParam(KxBaseParamWidget):
    """
              涂胶项目
    """
    _SCAN_MAP_TIMES = 3 #全地图扫描次数
    _MAX_ROI_NUM = 42
    _MAX_SCAN_NUM = 6
    _LIST_GROUPNAME = ['组数一', '组数二', '组数三', '组数四', '组数五', '组数六']
    _MAX_MASK_NUM = 50
    _BUILD_MODEL_SCALE_FACTOR = 4#建模的时候对图像进行压缩，不然会卡顿

    _SIG_CAPTUREBIGIMG_DONE = QtCore.pyqtSignal()# 大图采集建模完成

    SIG_LOAD2MODEL1 = QtCore.pyqtSignal(object)  # 单张图像采集
    SIG_LOAD2MODEL2 = QtCore.pyqtSignal()  # 切换下一列
    SIG_LOAD2MODEL3 = QtCore.pyqtSignal()  # 采集完成
    SIG_LOAD2MODEL_ERROR = QtCore.pyqtSignal(str)  # 发生错误

    def __init__(self, h_parentwidget, n_uid, n_areanum, n_stationid):
        KxBaseParamWidget.__init__(self,n_uid, n_areanum, n_stationid)
        self.h_parent = h_parentwidget
        self.ui = Ui_ParamPYLoadWidget()
        self.ui.setupUi(self)
        self.dict_config = readconfig(MAINSTATION_CONFIG)
        StaticConfigParam.DIS2PIXEL = 1.0 / float(self.dict_config['resolution'])
        #print(StaticConfigParam.DIS2PIXEL)
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
        self._SIG_CAPTUREBIGIMG_DONE.connect(self._closewaitdialog)


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
        self.ui.h_pBtLoad.setText("模拟建模")
        self.ui.h_pBtLoad.clicked.connect(self._simulate2buildmodel)
        # self.ui.h_pBtLoad.setText("载入图像")
        # self.ui.h_pBtLoad.clicked.connect(self.loadimg)



    def _initparam(self):
        self.s_imgpath = None
        self.n_qualitytreenum = 0#当前已显示质量检查组数
        self.n_measurehighnum = 0#测量高度的点
        #self.n_checkarea = 0#当前检测区域
        self.b_connectstatus = False# 参数改变信号，是否连接状态，是则不调用disconnect
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
                {'name': '相机横向分辨率', 'type': 'float', 'value': 0.1, 'limits': [0, 1], 'visible':False},
                {'name': '相机纵向像素数', 'type': 'int', 'value': 2000, 'limits': [200, 6600]},
                {'name': '拍摄长度', 'type': 'int', 'value': 2000, 'limits': [0, StaticConfigParam.MAX_Y_LEN]},
                {'name': '起拍位置', 'type': 'int', 'value': 0, 'limits':[0, 2000], 'visible':False},
                {'name': '拍摄组数', 'type': 'int', 'value': 3, 'limits': [1, self._MAX_SCAN_NUM]},
                {'name': '横向重叠区域', 'type': 'int', 'value': 1, 'limits': [1, 2000]},
                {'name': '全局取图', 'type': 'action'},
                {'name': '扫描区域取图', 'type': 'action'},
            ]},
            {'name': '建模图像缩放系数', 'type': 'str', 'value':str(self._BUILD_MODEL_SCALE_FACTOR),'visible':False,
             'readonly':True },
            #{'name': u'检测区域数量', 'type': 'int', 'value': 0, 'step': 1, 'limits': (0, self._MAX_ROI_NUM)},
            {'name': u'显示图像', 'type':'list', 'values': {'组数一': 0, '组数二': 1, '组数三':2, '组数四':3,
                                                        '组数五':4, '组数六':5}},
            {'name': '检测参数', 'type': 'group', 'children': [
                {'name': '涂胶类型', 'type': 'list', 'values': {'弓形': 0, '涂满': 1}},
                {'name': '检高灵敏度', 'type': 'int', 'value': 3, 'limits': [0, 25]},
                {'name': '检低灵敏度', 'type': 'int', 'value': 3, 'limits': [0, 25]},
                {'name': '标准色调', 'type': 'int', 'value': 80, 'limits': [0, 255]},
                {'name': '色差灵敏度', 'type': 'int', 'value': 2, 'limits': [0, 25]},
                {'name': '异物最小点数', 'type': 'int', 'value': 20, 'limits': [0, 20000]},
            ]},
            {'name': '反光校正', 'type': 'group', 'children': [
                {'name': '左右校正偏移量', 'type': 'int', 'value': 300, 'limits': [0, 1500]},
                {'name': '上下校正偏移量', 'type': 'int', 'value': 3, 'limits': [0, 1500]},
                {'name': '灰度偏移系数', 'type': 'int', 'value': 1, 'limits': [0, 25]},
                {'name': '色调偏移系数', 'type': 'int', 'value': 3, 'limits': [0, 25]},
            ]},
            {'name': '拼图信息', 'type': 'group', 'visible':False,'children': [
                {'name': '行数', 'type': 'int', 'value': 0, 'limits': [0, self._MAX_SCAN_NUM]},
                {'name': '列数', 'type': 'int', 'value': 0, 'limits': [0, self._MAX_SCAN_NUM]},
                {'name': '最大检测框高', 'type': 'str', 'value': ''},
            ]},
        ])

        self._append_scan_area(self.params)
        #self._append_checkarea_param(self.params)
        self._appendcheckparam(self.params)
        self.p = KxParameter.create(name='params', type='group', children=self.params)
        self.h_parameterTree.setParameters(self.p, showTop=False)
        self.p.param(u'主站设置', u'图像信息').add2view(self.ui.h_gVShowRealImg, self.h_imgitem)
        # for nindex in range(self._MAX_ROI_NUM):
        #      self.p.param(u'检测区域' + str(nindex), u'检测区域').add2view(self.view)
        for nindex in range(self._MAX_SCAN_NUM):
             self.p.param('扫描区域', '扫描区域' + str(nindex)).add2view(self.view)
        for nindex in range(self._MAX_SCAN_NUM * 2):
             self.p.param('扫描区域', '匹配位置' + str(nindex)).add2view(self.view)

        for i in range(self._MAX_SCAN_NUM):
            for j in range(int(self._MAX_ROI_NUM / self._MAX_SCAN_NUM)):
                self.p.param(self._LIST_GROUPNAME[i], "检测区域"+str(j+1), "检测区域").add2view(self.view)

            for j in range(int(self._MAX_MASK_NUM)):
                self.p.param(self._LIST_GROUPNAME[i], "高灰度掩膜区域"+str(j)).add2view(self.view)

            for j in range(int(self._MAX_MASK_NUM)):
                self.p.param(self._LIST_GROUPNAME[i], "低灰度掩膜区域"+str(j)).add2view(self.view)

        self._initsignal()


    def _appendcheckparam(self, dict_params):
        list_groupname = self._LIST_GROUPNAME
        dict_checkareanum = {'name': '检测区域数量', 'type': 'int', 'visible': True, 'value': 0, 'limits':[0, int(self._MAX_ROI_NUM / self._MAX_SCAN_NUM)]}
        dict_highmaskareanum = {'name': '高灰度掩膜数量', 'type': 'int', 'visible': True, 'value': 0, 'limits':[0, self._MAX_MASK_NUM]}
        dict_lowmaskareanum = {'name': '低灰度掩膜数量', 'type': 'int', 'visible': True, 'value': 0, 'limits':[0, self._MAX_MASK_NUM]}
        list_checkchildrenitems = []
        for i in range(0, int(self._MAX_ROI_NUM / self._MAX_SCAN_NUM)):
            list_checkchildrenitems.append(
                {'name': '检测区域' + str(i + 1), 'type': 'group', 'expanded': False, 'visible': False, 'children': [
                    {'name': u'检测区域', 'type': 'roiwithtext', 'value': {"isShow": False, "pos": u"0,0,100,100"},
                      "roi_opt": {"word": '检测区域'+ str(i + 1), "scaleable": True, 'pen':1}, "infovisible": False},
                    {'name': '屏蔽当前检测区域', 'type': 'bool', 'value':False},
                ]}
            )

        for i in range(0, self._MAX_MASK_NUM):
            list_checkchildrenitems.append(
                    {'name': u'高灰度掩膜区域'+str(i), 'type': 'roiwithtext', 'value': {"isShow": False, "pos": u"0,0,100,100"},
                     "roi_opt": {"word": '高灰度掩膜区域' + str(i), "scaleable": True, 'pen': 4}, "infovisible": False, 'visible':False}
            )

        for i in range(0, self._MAX_MASK_NUM):
            list_checkchildrenitems.append(
                    {'name': u'低灰度掩膜区域'+str(i), 'type': 'roiwithtext', 'value': {"isShow": False, "pos": u"0,0,100,100"},
                     "roi_opt": {"word": '低灰度掩膜区域' + str(i), "scaleable": True, 'pen': 5}, "infovisible": False, 'visible':False}
            )
        list_one_info = [dict_checkareanum, dict_highmaskareanum, dict_lowmaskareanum]
        list_one_info.extend(list_checkchildrenitems)

        list_dict_group = []
        for groupname in list_groupname:
            dict_group = {'name': groupname, 'type': 'group', 'expanded': False, 'visible': True, 'children':list_one_info}
            list_dict_group.append(dict_group)

        dict_params.extend(list_dict_group)




    def _connectlog(self):
        """连接日志信号槽，改动可显示到日志中"""
        self.b_connectstatus = True
        self.p.param("全局拍摄控制").sigTreeStateChanged.connect(self._logparamchange)
        self.p.param("检测参数").sigTreeStateChanged.connect(self._logparamchange)
        #self.p.param("检测区域数量").sigTreeStateChanged.connect(self._logparamchange)
        self.p.param("防呆防错设置").sigTreeStateChanged.connect(self._logparamchange)
        for i in range(self._MAX_SCAN_NUM):
            self.p.param(self._LIST_GROUPNAME[i]).sigTreeStateChanged.connect(self._logparamchange)



    def _disconnectlog(self):
        """取消连接日志信号槽，因为参数树的一个bug，程序刚启动的时候会有些"""
        if self.b_connectstatus:
            self.p.param("全局拍摄控制").sigTreeStateChanged.disconnect(self._logparamchange)
            self.p.param("检测参数").sigTreeStateChanged.disconnect(self._logparamchange)
            #self.p.param("检测区域数量").sigTreeStateChanged.disconnect(self._logparamchange)
            self.p.param("防呆防错设置").sigTreeStateChanged.disconnect(self._logparamchange)
            for i in range(self._MAX_SCAN_NUM):
                self.p.param(self._LIST_GROUPNAME[i]).sigTreeStateChanged.disconnect(self._logparamchange)
            self.b_connectstatus = False


    def _logparamchange(self, *args):
        s_log = args[0].name() + "--" + args[1][0][0].name() +\
                " 由 " + str(args[1][0][0].defaultValue()) + "改为 " + str(args[1][0][2])
        ipc_tool.kxlog("main", logging.INFO, s_log)


    def _initsignal(self):
        """初始化时负责连接关键信号槽"""
        #self.p.param('检测区域数量').sigValueChanged.connect(self._add_checkarea)
        self.p.param('全局拍摄控制', '全局取图').sigActivated.connect(self._captureimg)
        self.p.param('全局拍摄控制', '扫描区域取图').sigActivated.connect(self._captureimg_second)
        self.p.param('显示图像').sigValueChanged.connect(self._changeshowimg)
        self.p.param('全局拍摄控制', '横向重叠区域').sigValueChanged.connect(self._adjustbigimgsize)
        self.p.param('防呆防错设置', '基准点数量').sigValueChanged.connect(self._addpointparam)
        self.p.param('防呆防错设置', '标定基准高度').sigActivated.connect(self._calibrate_highsensor_point)

        for i in range(self._MAX_SCAN_NUM):
            self.p.param(self._LIST_GROUPNAME[i], "检测区域数量").sigValueChanged.connect(self._addcheckarea)

            self.p.param(self._LIST_GROUPNAME[i], "高灰度掩膜数量").sigValueChanged.connect(self._addmaskarea1)

            self.p.param(self._LIST_GROUPNAME[i], "低灰度掩膜数量").sigValueChanged.connect(self._addmaskarea2)

        self.SIG_LOAD2MODEL1.connect(self._slot_simulate_recimg)
        self.SIG_LOAD2MODEL2.connect(self._slot_simulate_changcol)
        self.SIG_LOAD2MODEL3.connect(self._slot_simulate_builddone)
        self.SIG_LOAD2MODEL_ERROR.connect(self._slot_error)


    def _addcheckarea(self, *args):
        ncol = 0
        for nindex, groupname in enumerate(self._LIST_GROUPNAME):
            if args[0] == self.p.param(groupname, "检测区域数量"):
                ncol = nindex
                break
        ncurimg = int(self.p.param('显示图像').value())
        if ncol == ncurimg:
            for n_i in range(0, int(args[1])):
                self.p.param(self._LIST_GROUPNAME[ncol], '检测区域' + str(n_i + 1), '检测区域').isShow(True)
            for n_i in range(int(args[1]), int(self._MAX_ROI_NUM / self._MAX_SCAN_NUM)):
                self.p.param(self._LIST_GROUPNAME[ncol], '检测区域' + str(n_i + 1), '检测区域').isShow(False)


    def _addmaskarea1(self, *args):
        ncol = 0
        for nindex, groupname in enumerate(self._LIST_GROUPNAME):
            if args[0] == self.p.param(groupname, "高灰度掩膜数量"):
                ncol = nindex
                break
        ncurimg = int(self.p.param('显示图像').value())
        if ncol == ncurimg:
            for n_i in range(0, int(args[1])):
                self.p.param(self._LIST_GROUPNAME[ncol], '高灰度掩膜区域' + str(n_i)).isShow(True)
            for n_i in range(int(args[1]), int(self._MAX_MASK_NUM)):
                self.p.param(self._LIST_GROUPNAME[ncol], '高灰度掩膜区域' + str(n_i)).isShow(False)


    def _addmaskarea2(self, *args):
        ncol = 0
        for nindex, groupname in enumerate(self._LIST_GROUPNAME):
            if args[0] == self.p.param(groupname, "低灰度掩膜数量"):
                ncol = nindex
                break
        ncurimg = int(self.p.param('显示图像').value())
        if ncol == ncurimg:
            for n_i in range(0, int(args[1])):
                self.p.param(self._LIST_GROUPNAME[ncol], '低灰度掩膜区域' + str(n_i)).isShow(True)
            for n_i in range(int(args[1]), int(self._MAX_MASK_NUM)):
                self.p.param(self._LIST_GROUPNAME[ncol], '低灰度掩膜区域' + str(n_i)).isShow(False)


    def _addpointparam(self, *even):
        """显示或隐藏防呆防错信号槽"""
        if int(even[1]) > self.n_measurehighnum:
            for n_i in range(self.n_measurehighnum, int(even[1])):
                self.p.param('防呆防错设置','防呆防错点' + str(n_i)).show()
        else:
            for n_i in range(int(even[1]), self.n_measurehighnum):
                self.p.param('防呆防错设置', '防呆防错点' + str(n_i)).hide()
        self.n_measurehighnum = int(even[1])


    # def _add_checkarea(self, *even):
    #     """显示或隐藏检测区域"""
    #     if int(even[1]) > self.n_checkarea:
    #         for n_i in range(self.n_checkarea, int(even[1])):
    #             self.p.param('检测区域' + str(n_i)).show()
    #             self.p.param('检测区域' + str(n_i), '检测区域').isShow(True)
    #     else:
    #         for n_i in range(int(even[1]), self.n_checkarea):
    #             self.p.param('检测区域' + str(n_i)).hide()
    #             self.p.param('检测区域' + str(n_i), '检测区域').isShow(False)
    #     self.n_checkarea = int(even[1])


    # def _append_checkarea_param(self, dict_params):
    #     """增加检测区域的参数"""
    #     list_standardschildrenitems = []
    #     for i in range(0, self._MAX_ROI_NUM):
    #         list_standardschildrenitems.append(
    #             {'name': '检测区域' + str(i), 'type': 'group', 'expanded': False, 'visible': False, 'children': [
    #                 {'name': u'检测区域', 'type': 'roiwithtext', 'value': {"isShow": False, "pos": u"0,0,100,100"},
    #                   "roi_opt": {"word": '检测区域'+ str(i), "scaleable": True, 'pen':1}, "infovisible": False},
    #                 {'name': u'扫描组号', 'type': 'list', 'values': {'组数一': 0, '组数二': 1, '组数三':2, '组数四':3,
    #                                                     '组数五':4, '组数六':5}},
    #                 {'name': u'当前组ID', 'type': 'int', 'value': 0, 'limits':[0, 6], 'readonly':True},
    #                 {'name': '屏蔽当前检测区域', 'type': 'bool', 'value':False},
    #             ]}
    #         )
    #     dict_params.extend(list_standardschildrenitems)


    def _append_scan_area(self, dict_params):
        """增加扫描区域的参数"""
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

        for i in range(0, self._MAX_SCAN_NUM * 2):
            list_standardschildrenitems.append(
                {'name': u'匹配位置' + str(i), 'type': 'roiwithtext', 'value': {"isShow": False, "pos": u"0,0,100,100"},
                 "roi_opt": {"word": '匹配位置' + str(i), "scaleable": True, 'pen': 2}, "infovisible": False},
            )

        dict_scan = {'name': '扫描区域', 'type': 'group', 'visible':False, 'children':list_standardschildrenitems}

        dict_params.append(dict_scan)


    def _append_fangdaifangcuo_point(self):
        """给参数树增加防呆防错的参数"""
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
        self.threadWaitDialog = WaitDialogWithText('正在建模，请勿点击...')
        self.threadWaitDialog.clear()
        self.threadWaitDialog.setProcessBarRange(0, 100)
        self.threadWaitDialog.show()
        QtCore.QCoreApplication.processEvents(QtCore.QEventLoop.ExcludeUserInputEvents)
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

        n_firstbuild_imgnum = int(ndisY * StaticConfigParam.DIS2PIXEL) / imgH + 1

        ndisY = min(int((n_firstbuild_imgnum + 1) * imgH /  StaticConfigParam.DIS2PIXEL) + StaticConfigParam.RUN_MORE_DIS, StaticConfigParam.MAX_Y_LEN )#当取到最长的时候这个地方就起作用

        n_firstbuild_imgnum = int(ndisY * StaticConfigParam.DIS2PIXEL / imgH)

        nbigimgH = int(n_firstbuild_imgnum * imgH / self._BUILD_MODEL_SCALE_FACTOR)

        nbigimgW = int(imgW / self._BUILD_MODEL_SCALE_FACTOR) * nXtimes

        self.h_mergeobj.clear()

        self.h_mergeobj.initinfo(n_firstbuild_imgnum, nbigimgW, nbigimgH)

        self.build_status = BuildStatus.STATUS_ALL

        ipc_tool.getqueue_processedData().put((-1, imc_msg.MSG_BUILD_MODEL, [nStartX, ndisX, ndisY, nXtimes]))


    def _captureimg_second(self):
        """
        第二次采集图像进行建模，根据roi框的位置进行拍摄
        :return:
        """
        self.threadWaitDialog = WaitDialogWithText('正在二次取图建模，请勿点击...')
        self.threadWaitDialog.clear()
        self.threadWaitDialog.setProcessBarRange(0, 100)
        self.threadWaitDialog.show()
        QtCore.QCoreApplication.processEvents(QtCore.QEventLoop.ExcludeUserInputEvents)

        imgW = int(self.p.param('全局拍摄控制', '相机横向像素数').value())

        list_x, list_y, list_buildimgnum, list_bigimgH = self._getlist_xy_movepos()

        for nindex, n_build_imgnum in  enumerate(list_buildimgnum):

            self.p.param("扫描区域", "扫描区域%d图像数量" % nindex).setValue(n_build_imgnum)

        self.h_mergelistobj.clear()

        self.h_mergelistobj.initinfo(list_buildimgnum, int(imgW / self._BUILD_MODEL_SCALE_FACTOR), list_bigimgH)

        self.build_status = BuildStatus.STATUS_SECOND

        ipc_tool.getqueue_processedData().put((-1, imc_msg.MSG_BUILD_MODEL_SECOND, [list_x, list_y]))


    def _getlist_xy_movepos(self):
        """
        获取 二次取图建模时所需要的移动距离，也是实际检测流程开始时需要的数据
        Returns
        -------
        list_x  移动x距离,单位mm
        list_y  移动y距离，单位mm
        list_buildimgnum    每个值代表扫描一列拍摄多少张图
        list_bigimgH        每一列的大图高，用于拼接图像
        """
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


        return list_x, list_y, list_buildimgnum, list_bigimgH


    def call2back2getcaptureinfo(self):
        """
            开始检测时，外部回调这个函数得到运动信息，以及标定的防呆防错基准值
        """

        list_x, list_y, list_buildimgnum, list_bigimgH = self._getlist_xy_movepos()

        # 记录判断防呆防错的点
        list_z = []
        nnum = int(self.p.param('防呆防错设置', '基准点数量').value())

        if nnum > 0:
            for nindex in range(nnum):
                x = self.p.param('防呆防错设置', '防呆防错点%d' % nindex, 'X位置').value()

                y = self.p.param('防呆防错设置', '防呆防错点%d' % nindex, 'Y位置').value()

                z = self.p.param('防呆防错设置', '防呆防错点%d' % nindex, '基准值').value()

                list_z.append([x, y, z])

        return [list_x, list_y, list_z]


    def _calibrate_highsensor_point(self):
        """
        标定基准高度值，通过公共队列往主界面传输点坐标，点坐标收到数据之后启动线程进行测量扫描，
        扫描之后记录点值回调 callback2set_highsensor_point 将测量点结果送回
        :return:
        """
        nnum = int(self.p.param('防呆防错设置', '基准点数量').value())

        if nnum > 0:
            self.threadWaitDialog = WaitDialogWithText('正在标定基准高度，请勿点击...')
            self.threadWaitDialog.clear()
            self.threadWaitDialog.setProcessBarRange(0, 100)
            self.threadWaitDialog.show()
            QtCore.QCoreApplication.processEvents(QtCore.QEventLoop.ExcludeUserInputEvents)

            list_pos = []

            for nindex in range(nnum):

                x = min(int(self.p.param('防呆防错设置', '防呆防错点%d'%nindex, 'X位置').value()), StaticConfigParam.MAX_X_LEN)

                y = min(int(self.p.param('防呆防错设置', '防呆防错点%d'%nindex, 'Y位置').value()), StaticConfigParam.MAX_Y_LEN)

                list_pos.append([x, y])

            ipc_tool.getqueue_processedData().put((-1, imc_msg.MSG_GET_BASE_POINT_HIGH, list_pos))


    def callback2set_highsensor_point(self, list_param):
        """
        外部回调进行设置基准值
        Parameters
        ----------
        list_param：高度传感器获取的基准值

        Returns
        -------

        """
        for nindex, data in enumerate(list_param):

            if data is not None:

                self.p.param('防呆防错设置', '防呆防错点%d' % nindex, '基准值').setValue(str(int(data)))

        self._SIG_CAPTUREBIGIMG_DONE.emit()



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
        """
        载入模板参数，切换模板或新建模板、刚启动程序的时候都会调用这个类
        Returns
        -------

        """
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

        #self.n_checkarea = int(self.p.param('检测区域数量').value())

        self._connectlog()


    def saveparameters(self):
        """保存参数"""
        self._disconnectlog()
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
        self.sendcheckstatus()
        self._connectlog()


    def _sortcheckpos(self):
        """
        同一扫描列的检测框，必须按照检测框左上角x、y坐标小的在上，坐标大的在下
        :return:
        """
        ncolnum = int(self.p.param('全局拍摄控制', '拍摄组数').value())

        for i in range(ncolnum):
            list_pos = []
            list_obj = []
            nroinum = self.p.param(self._LIST_GROUPNAME[i], "检测区域数量").value()
            for j in range(nroinum):
                list_pos.append(self.p.param(self._LIST_GROUPNAME[i], '检测区域' + str(j+1), '检测区域').get_list_pos())
                list_obj.append(self.p.param(self._LIST_GROUPNAME[i], '检测区域' + str(j+1), '检测区域'))
            if list_pos != []:
                array = np.array(list_pos).T
                list_sortindex = sorted(range(len(array[1])), key=lambda k: array[1][k], reverse=False)
                for nindex in range(len(list_pos)):
                    list_obj[nindex].set_list_pos(list_pos[list_sortindex[nindex]])

        # nchecknum = int(self.p.param('检测区域数量').value())
        # list_pos = []
        # list_obj = []
        # ncurcol = -1
        # for i in range(nchecknum):
        #
        #     ncol = int(self.p.param('检测区域' + str(i), '扫描组号').value())
        #
        #     if ncurcol != ncol:
        #
        #         ncurcol = ncol
        #
        #         if list_pos != []:
        #
        #             array = np.array(list_pos).T
        #
        #             list_sortindex = sorted(range(len(array[1])), key=lambda k: array[1][k], reverse=False)
        #
        #             for nindex in range(len(list_pos)):
        #
        #                 list_obj[nindex].set_list_pos(list_pos[list_sortindex[nindex]])
        #
        #             list_pos = []
        #
        #             list_obj = []
        #
        #     list_pos.append(self.p.param('检测区域'+ str(i), '检测区域' ).get_list_pos())
        #
        #     list_obj.append(self.p.param('检测区域'+ str(i), '检测区域' ))


    def _saveMapColRow(self):
        """
        根据扫描列数以及检测框中属于当前列的数量，得到一张全地图应该是N*M的组成
        :return:
        """
        # 1. 确定图像最大
        # nchecknum = int(self.p.param('检测区域数量').value())
        # if nchecknum ==0 :
        #     return
        ncolnum = int(self.p.param('全局拍摄控制', '拍摄组数').value())
        list_col = [0 for i in range(ncolnum)]
        list_h = []
        for i in range(ncolnum):
            nroinum = self.p.param(self._LIST_GROUPNAME[i], "检测区域数量").value()
            list_col[i] = nroinum
            for j in range(nroinum):
                roipos = self.p.param(self._LIST_GROUPNAME[i], '检测区域'+ str(i+1), '检测区域' ).get_list_pos()
                list_h.append(roipos[3] - roipos[1] + 1)

        nmaxrow = max(list_col)
        #nmaxh = max(list_h) * self._BUILD_MODEL_SCALE_FACTOR

        self.p.param('拼图信息', '行数').setValue(nmaxrow)
        self.p.param('拼图信息', '列数').setValue(ncolnum)
        #self.p.param('拼图信息', '最大检测框高').setValue(nmaxh)

        #2. 设置每个检测区域的组ID，表示这个roi在当前组中属于第几个ID，便于显示检测结果的时候拼图
        # TODO 2022-03-17 觉得不是很必要，所以取消
        # ncurcol = -1
        # ncurid = 0
        # for i in range(nchecknum):
        #     ncol = int(self.p.param('检测区域' + str(i), '扫描组号').value())
        #     if ncurcol != ncol:
        #         ncurcol = ncol
        #         ncurid = 0
        #     self.p.param('检测区域' + str(i), '当前组ID').setValue(ncurid)
        #     ncurid += 1


    def _changeshowimg(self, *even):
        """改变显示图像，选择组号几的时候进行切换"""
        nindex = int(even[1])

        if nindex < len(self.list_img):

            # 1. 显示图像
            self.h_imgitem.setImage(self.list_img[nindex])

            # #2. 显示匹配框
            for i in range(self._MAX_SCAN_NUM * 2):
                self.p.param('扫描区域', "匹配位置" + str(i)).isShow(False)
            self.p.param('扫描区域', "匹配位置" + str(nindex*2)).isShow(True)
            self.p.param('扫描区域', "匹配位置" + str(nindex * 2 + 1)).isShow(True)

            #
            #3. 屏蔽所有其它掩膜框, 显示当前框
            ncolnum = int(self.p.param('全局拍摄控制', '拍摄组数').value())

            for ncol in range(ncolnum):

                b_status = False

                if ncol == nindex:#

                    b_status = True

                nchecknum = int(self.p.param(self._LIST_GROUPNAME[ncol], "检测区域数量").value())

                nmasknum1 = int(self.p.param(self._LIST_GROUPNAME[ncol], "高灰度掩膜数量").value())

                nmasknum2 = int(self.p.param(self._LIST_GROUPNAME[ncol], "低灰度掩膜数量").value())

                for i in range(nchecknum):

                    self.p.param(self._LIST_GROUPNAME[ncol], "检测区域"+str(i + 1), "检测区域").isShow(b_status)

                for i in range(nmasknum1):

                    self.p.param(self._LIST_GROUPNAME[ncol], "高灰度掩膜区域"+str(i)).isShow(b_status)

                for i in range(nmasknum2):

                    self.p.param(self._LIST_GROUPNAME[ncol], "低灰度掩膜区域" + str(i)).isShow(b_status)




    def _setbaseimgpath(self):
        """设置底板路径"""
        list_str = self.str_filedirectory.split('\\')
        for i in range(self._MAX_SCAN_NUM):
            list_str[-1] = str(i) + '.bmp'
            imgpath = "\\".join(list_str)
            self.p.param('主站设置', '底板路径' + str(i)).setValue(imgpath)


    def _ReceiveBuildModelImg(self, tuple_data):
        """接收建模图像，第一次建模的图，且压缩放入拼图obj中"""
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
        """初次建模时N次扫描，进行更换列"""
        self.h_mergeobj.IncreaseCol()

        self.threadWaitDialog.setProcessBarVal(20)



    def callback2showbigimg(self):
        """初次建模完成回调显示大图"""
        self.h_imgitem.setImage(self.h_mergeobj.bigimg)

        self.h_bigimage = self.h_mergeobj.bigimg

        for n_i in range(self._MAX_SCAN_NUM):

            self.p.param('扫描区域', '扫描区域' + str(n_i)).isShow(False)

        nXtimes = int(self.p.param('全局拍摄控制', '拍摄组数').value())

        for n_i in range(int(nXtimes)):

            self.p.param('扫描区域', '扫描区域' + str(n_i)).isShow(True)

        self._SIG_CAPTUREBIGIMG_DONE.emit()




    def callback2judgeisfull(self):
        """外部回调判断当前列图像是否已满，是的话外部会执行扫描下一列的动作"""
        return self.h_mergeobj.IsFull()


    def callback2changecol_second(self):
        """"""
        self.h_mergelistobj.IncreaseCol()

        self.threadWaitDialog.setProcessBarVal(20)


    def callback2showbigimg_second(self):
        self.list_img = self.h_mergelistobj.list_bigimg

        self.h_imgitem.setImage(self.list_img[0])

        self.p.param("显示图像").setValue(0)

        self.p.param('扫描区域', "匹配位置0").isShow(True)
        self.p.param('扫描区域', "匹配位置1").isShow(True)


        self._SIG_CAPTUREBIGIMG_DONE.emit()



    def callback2judgeisfull_second(self):
        return self.h_mergelistobj.IsFull()


    def _adjustbigimgsize(self):
        if self.h_bigimage is not None:
            #nXtimes = int(self.p.param('全局拍摄控制', '拍摄组数').value())
            nXtimes = self._SCAN_MAP_TIMES
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
        nchecknum = 0

        ncolnum = int(self.p.param('全局拍摄控制', '拍摄组数').value())

        for i in range(ncolnum):
            nroinum = self.p.param(self._LIST_GROUPNAME[i], "检测区域数量").value()
            nchecknum += nroinum


        list_shead = []

        for i in range(nchecknum):

            list_shead.append("检测区域%d"%(i + 1))

        return list_shead


    def _closewaitdialog(self):
        QtCore.QCoreApplication.processEvents(QtCore.QEventLoop.AllEvents)

        self.threadWaitDialog.setProcessBarVal(20)

        self.threadWaitDialog.close()


    def getcheckareastatus(self):
        """
        程序启动的时候，外部界面需要获取检测区域屏蔽状态，通过这个函数获取。也可提供类内函数调用
        """

        # list_status = []
        #
        # nchecknum = int(self.p.param('检测区域数量').value())
        #
        # for i in range(nchecknum):
        #
        #     status = self.p.param('检测区域' + str(i), '屏蔽当前检测区域').value()
        #
        #     if status == 'True' or status == 'true' or status == True:
        #
        #         list_status.append(1)
        #
        #     else:
        #
        #         list_status.append(0)
        #
        # return list_status

        list_status = []
        ncolnum = int(self.p.param('全局拍摄控制', '拍摄组数').value())
        for i in range(ncolnum):

            nroinum = self.p.param(self._LIST_GROUPNAME[i], "检测区域数量").value()

            for j in range(nroinum):

                status = self.p.param(self._LIST_GROUPNAME[i], '检测区域' + str(j+1), '屏蔽当前检测区域').value()

                if status == 'True' or status == 'true' or status == True:

                    list_status.append(1)

                else:

                    list_status.append(0)

        return list_status



    def callback2changecheckstatus(self, list_data):
        """
        屏蔽检测区域界面点击保存之后，会将结果设置到这个位置
        """
        # for i, data in enumerate(list_data):
        #
        #     self.p.param('检测区域' + str(i), '屏蔽当前检测区域').setValue(bool(data))
        #
        # self.saveparameters()

        ncolnum = int(self.p.param('全局拍摄控制', '拍摄组数').value())

        ncurindex = 0

        for i in range(ncolnum):

            nroinum = self.p.param(self._LIST_GROUPNAME[i], "检测区域数量").value()

            for j in range(nroinum):

                self.p.param(self._LIST_GROUPNAME[i], "检测区域" + str(j+1),  '屏蔽当前检测区域').setValue(bool(list_data[ncurindex]))

                ncurindex +=1

                if ncurindex >= len(list_data):
                    break

            if ncurindex >= len(list_data):
                break
        self.saveparameters()


    def sendcheckstatus(self):
        """
        点击保存的时候将检测是否需要屏蔽的结果发送给另外的界面
        """
        ipc_tool.getqueue_processedData().put((-1, imc_msg.MSG_SET_CHECK_MASK, self.getcheckareastatus()))


    def _simulate2buildmodel(self):
        """
        模拟建模，也即载入图片进行建模
        1. 首先初始化图形列表
        2. 打开线程
        3.
        Returns
        -------

        """
        file_name = QtWidgets.QFileDialog.getExistingDirectory(self, "打开模拟取图路径")
        #print (file_name)

        self.threadWaitDialog = WaitDialogWithText('本地取图进行二次建模，请勿点击...')
        self.threadWaitDialog.clear()
        self.threadWaitDialog.setProcessBarRange(0, 100)
        self.threadWaitDialog.show()
        QtCore.QCoreApplication.processEvents(QtCore.QEventLoop.ExcludeUserInputEvents)

        imgW = int(self.p.param('全局拍摄控制', '相机横向像素数').value())

        list_x, list_y, list_buildimgnum, list_bigimgH = self._getlist_xy_movepos()

        self.h_mergelistobj.clear()

        self.h_mergelistobj.initinfo(list_buildimgnum, int(imgW / self._BUILD_MODEL_SCALE_FACTOR), list_bigimgH)

        self.build_status = BuildStatus.STATUS_SIMULATE

        threadloadimg2buildmodel = threading.Thread(target=self._threadfun_loadimg2buildmodel, args=[list_buildimgnum,
                                                                                                     file_name])
        threadloadimg2buildmodel.start()


    def _slot_simulate_recimg(self, img):
        newsize = (int(img.shape[1] / self._BUILD_MODEL_SCALE_FACTOR), int(img.shape[0] / self._BUILD_MODEL_SCALE_FACTOR))
        resizeimg = cv2.resize(img, newsize)
        self.h_mergelistobj.merge(resizeimg)


    def _slot_simulate_changcol(self):
        self.callback2changecol_second()


    def _slot_simulate_builddone(self):
        self.callback2showbigimg_second()


    def _slot_error(self, info):
        self._closewaitdialog()
        QtWidgets.QMessageBox.warning(self, u"错误", "！！！" + info + " ！！！", QtWidgets.QMessageBox.Cancel)


    def _threadfun_loadimg2buildmodel(self, *data):
        """
        线程，载入图片进行建模。图片名称有序列要求，从 0 开始不断迭代，且中间不能断，且总数量不能小于两倍sum(list_imgnum)
        Parameters
        ----------
        data    :[list_imgnum, path]
                 list_imgnum 图像列表数量，代表没一列采集多少张图像
                 path        模拟取图路径
        Returns
        -------

        """
        list_imgnum = data[0]
        path = data[1]
        if os.path.isdir(path):
            nmaxnum = sum(list_imgnum)
            nreadindex = 0  # 读取索引
            nreadimgnum = 0
            for nimgnum in list_imgnum:
                for i in range(nimgnum):
                    readpath = path + "\\" + str(nreadindex) + ".bmp"
                    nreadindex += 1
                    if os.path.isfile(readpath):
                        with open(readpath, 'rb') as curfp:
                            img = np.array(copy.copy(Image.open(curfp)))
                        nreadimgnum += 1
                        self.SIG_LOAD2MODEL1.emit(img)
                nreadindex += nimgnum  # 跳到下一列
                self.SIG_LOAD2MODEL2.emit()
            if nreadimgnum != nmaxnum:
                self.SIG_LOAD2MODEL_ERROR.emit("图像数量不足，无法完成建模")  # 返回错误
            else:
                self.SIG_LOAD2MODEL3.emit()
        else:
            self.SIG_LOAD2MODEL_ERROR.emit("路径不存在")  # 返回错误


    def loadimg(self):
        file_name = QtWidgets.QFileDialog.getOpenFileName(self, "open file dialog", "D://card",
                                                          "bmp files(*.bmp)")
        #print(file_name)
        image = cv2.imread(file_name[0], 1)
        self.h_imgitem.setImage(image, autoLevels=False)



registerkxwidget(name='GuleParam', cls=GuleParam, override=True)


if __name__ == '__main__':
    app = QtWidgets.QApplication([])
    w = GuleParam(None, 1, 2 ,3)
    w.show()
    app.exec_()