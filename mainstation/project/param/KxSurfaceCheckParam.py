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
from PIL import Image
# from pyqtgraph.imageview.ImageView import ImageView


class KxSurfacecheckParam(KxBaseParamWidget):
    """
              表面检查
    """
    def __init__(self, h_parentwidget, n_uid, n_areanum, n_stationid):
        KxBaseParamWidget.__init__(self,n_uid, n_areanum, n_stationid)
        self.h_parent = h_parentwidget
        self.ui = Ui_ParamPYLoadWidget()
        self.ui.setupUi(self)
        self._initui()
        self._initparam()
        self._initsignal()



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



    def _initparam(self):
        self.s_imgpath = None
        self.n_qualitytreenum = 0#当前已显示质量检查组数
        self.params.extend([
            {'name': u'主站设置', 'type': 'group', 'visible':False, 'children': [
                {'name': u'图像信息', 'type': 'imageinfo',
                 'value': {"isShow": True}, "infovisible": True},
                {'name': '底板路径', 'type':'str'},
            ]},

            {'name': u'检查设置', 'type': 'group', 'children': [
                {'name': u'学习', 'type': 'bool', 'value': 0},
                {'name': u'学习张数', 'type': 'int', 'value': 30},
                {'name': u'点灵敏度', 'type': 'slider', 'value': 1, 'limits': [0, 10]},
                {'name': u'线灵敏度', 'type': 'slider', 'value': 1, 'limits': [0, 10]},
                {'name': u'划伤灵敏度', 'type': 'slider', 'value': 1, 'limits': [0, 10]},
                {'name': u'灰度高灵敏度', 'type': 'slider', 'value': 1, 'limits': [0, 10]},
                {'name': u'灰度低灵敏度', 'type': 'slider', 'value': 1, 'limits': [0, 10]},
                {'name': u'缺失灵敏度', 'type': 'slider', 'value': 1, 'limits': [0, 10]},
                {'name': u'灵敏度基值', 'type': 'slider', 'value': 1, 'limits': [0, 10]}, ]
             },

            {'name': '定位设置', 'type': 'group', 'children': [
                {'name': u'检测框', 'type': 'roiwithtext', 'value': {"isShow": True, "pos": u"0,0,100,100"},
                 "roi_opt": {"word": u'检测框', "scaleable": True}, "infovisible": False},
                {'name': u'定位核个数', 'type': 'int', 'value': 0, 'step': 1, 'limits': (0, 4)},
                {'name': u'搜索方向', 'type': 'list', 'values': {u"垂直水平": 0, u'垂直': 1, u'水平': 2}, 'value': 0},
                {'name': u'搜索范围', 'type': 'int', 'value': 100, 'step': 10, 'limits': (10, 800), },
                {'name': u'算法类型', 'type': 'list', 'values': {u'投影变换': 0, u'线性变换': 1}, 'value': 0, 'visible': False},
                {'name': u'定位核', 'type': 'group', 'expanded': False, 'visible': False, 'children': [
                    {'name': u'定位核1', 'type': 'roiwithtext', 'value': {"isShow": False, "pos": u"0,0,100,100"},
                     "roi_opt": {"word": u'定位框1', "scaleable": True, 'pen':3}, "infovisible": False},
                    {'name': u'定位核2', 'type': 'roiwithtext', 'value': {"isShow": False, "pos": u"0,0,100,100"},
                     "roi_opt": {"word": u'定位框2', "scaleable": True, 'pen':3}, "infovisible": False},
                    {'name': u'定位核3', 'type': 'roiwithtext', 'value': {"isShow": False, "pos": u"0,0,100,100"},
                     "roi_opt": {"word": u'定位框3', "scaleable": True, 'pen':3}, "infovisible": False},
                    {'name': u'定位核4', 'type': 'roiwithtext', 'value': {"isShow": False, "pos": u"0,0,100,100"},
                     "roi_opt": {"word": u'定位框4', "scaleable": True, 'pen':3}, "infovisible": False}]}]
             }

        ])
        self.appendqualityinspectionstandards(self.params)

        self.p = KxParameter.create(name='params', type='group', children=self.params)
        self.h_parameterTree.setParameters(self.p, showTop=False)

        self.p.param(u'主站设置', u'图像信息').add2view(self.ui.h_gVShowRealImg, self.h_imgitem)
        self.p.param(u'定位设置', u'检测框').add2view(self.view)
        nmaxnum = self.p.param(u'定位设置', u'定位核个数').opts['limits'][1]
        for nindex in range(nmaxnum):
             self.p.param(u'定位设置', u'定位核', u'定位核%d' % (nindex + 1)).add2view(self.view)

    def appendqualityinspectionstandards(self, dict_params):
        '''
        判废标准模块
        '''
        # tip 取决子站主站共同定义的json文件
        s_tip = u'x[0]-点数\nx[1]-能量\nx[2]-左上X\nx[3]-左上Y\nx[4]-缺陷宽\nx[5]-缺陷高\n'
        self.n_maxstandardnum = 12
        list_standardschildrenitems = [
            {'name': '检查标准组数', 'type': 'int', 'value': self.n_qualitytreenum, 'step': 1,
             'limits': (0, self.n_maxstandardnum)}
        ]
        for i in range(0, self.n_maxstandardnum):
            list_standardschildrenitems.append(
            {'name': '质量检查标准' + str(i), 'type': 'group', 'expanded': False, 'visible': False, 'children': [
            {'name': '缺陷名', 'type': 'str', 'value': 'defectErr', 'visible': True},
            # {'name': '判废数', 'type': 'int', 'value': 1, 'step': 1, 'limits': (0, 4), 'visible': True},
            {'name': '表达式', 'type': 'kxtext', 'value': 'x[1]>100', 'tip': s_tip, 'visible': True},]}
        )


        dict_standardsitem = {'name': '质量检查标准', 'type': 'group',
                              'children': list_standardschildrenitems}
        dict_params.append(dict_standardsitem)


    def _initsignal(self):
        self.p.param(u'定位设置', u'定位核个数').sigValueChanged.connect(self._addkernroi)
        self.ui.h_pBtLoad.clicked.connect(self._loadpicture)
        self.ui.h_pBtSave.clicked.connect(self.saveparameters)
        self.p.param('质量检查标准', '检查标准组数').sigValueChanged.connect(self._addqualdetectslot)

    def _addqualdetectslot(self, *even):
        if int(even[1]) > self.n_qualitytreenum:
            for n_i in range(int(even[1])):
                self.p.param('质量检查标准','质量检查标准' + str(n_i)).show()
        else:
            for n_i in range(int(even[1]), self.n_qualitytreenum):
                self.p.param('质量检查标准', '质量检查标准' + str(n_i)).hide()
        self.n_qualitytreenum = int(even[1])


    def _addkernroi(self):
        nkernel = int(self.p.param(u'定位设置', u'定位核个数').value())
        nmaxkernel = self.p.param(u'定位设置', u'定位核个数').opts['limits'][1]
        if self.h_imgitem.image is None:
            return
        for nindex in range(nmaxkernel):
            self.p.param(u'定位设置', u'定位核', u'定位核%d' % (nindex + 1)).isShow(False)
        for nindex in range(nkernel):
            self.p.param(u'定位设置', u'定位核', u'定位核%d' % (nindex + 1)).isShow(True)

    def _loadpicture(self):
        file_name = QtWidgets.QFileDialog.getOpenFileName(self, "open file dialog", "d:\\",
                                                      "bmp files(*.bmp)")
        if file_name != '':
            file_name = file_name[0]
            with open(file_name, 'rb') as curfp:
                self.imagepil = copy.copy(Image.open(curfp))
            self.h_image = np.array(self.imagepil)
            self.h_imagecopy = self.imagepil
            self.h_imgitem.setImage(self.h_image, autoLevels=False)

    def setmodeldirectory(self, strpath):#overwrite
        super(KxSurfacecheckParam, self).setmodeldirectory(strpath)
        list_str = strpath.split('\\')
        list_str[-1] = "baseimg.bmp"
        self.s_imgpath = "\\".join(list_str)


    def saveparameters(self):#overwrite
        self.p.param('主站设置', '底板路径').setValue(self.s_imgpath)
        if self.h_imgitem.image is not None:
            image = np.array(self.h_image)
            imagesave = Image.fromarray(image)
            imagesave.save(self.s_imgpath)
        super(KxSurfacecheckParam, self).saveparameters()

    def loadparameters(self):#overwrite
        super(KxSurfacecheckParam, self).loadparameters()
        path = self.p.param('主站设置', '底板路径').value()
        if os.path.isfile(path):
            with open(path, 'rb') as curfp:
                self.imagepil = copy.copy(Image.open(curfp))
            self.h_image = np.array(self.imagepil)
            self.h_imagecopy = self.imagepil
            self.h_imgitem.setImage(self.h_image, autoLevels=False)

    def recmsg(self, n_stationid, n_msgtype, tuple_data):
        '''
        接收子站发送过来的消息
        '''
        # import imc_msg
        # if n_msgtype == imc_msg.MSG_SEND_REAL_TIME_IMAGE:
        #     self.ReceiveImages(tuple_data[0])

    def setlearnstatus(self, value):
        self.p.param('检查设置', '学习').setValue(value)
        self.saveparameters()





registerkxwidget(name='KxSurfacecheckParam', cls=KxSurfacecheckParam, override=True)


if __name__ == '__main__':
    app = QtWidgets.QApplication([])
    w = KxSurfacecheckParam(None, 1, 2 ,3)
    w.show()
    app.exec_()