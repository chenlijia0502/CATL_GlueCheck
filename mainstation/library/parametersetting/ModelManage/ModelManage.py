#coding:utf-8
# -*- coding: utf-8 -*-
""" use for manage the model"""

import os
import shutil
import logging
import time
import copy
import xml.dom.minidom
import numpy as np
from PIL import Image
from PyQt5 import QtCore, QtGui, QtWidgets

import pyqtgraph as pg
from .KxAddNewModelDialog import KxAddNewModelDialog
# from .ui_ModelManage import ParamSetGraphic
from UI.ui_modelmanage import Ui_ParamSetGraphic
from library.common.readsysparams import *
from library.common.globalparam import ImportantName
from UI.ui_modelmanage_lunzhuan import Ui_ParamSetGraphic_lunzhuan


pg.setConfigOptions(imageAxisOrder='row-major')



class ModelManage(QtWidgets.QWidget):
    """
        date       2017.07 — 2020.02.15
        author     HYH
        desc       模板类，创建于2017年，其实应该更早，当前版本是于2017年根据卡机
                   的模板管理改过来，彼时技术不熟练里面有些写法我现在也不太认可，
                   但好在时间证明了这个类足够稳定，所以对于后续的开发者只要知道
                   这个类的作用是管理模板——新建、复制、保存、删除。而对于类间
                   调用只要关注h_paramchanges这个信号，当执行每个操作时都会触发
                   这个信号。
                   另外，这个类只管理到模板名那个文件夹，模板名再往下便是其余类的事
    """

    h_paramchanges = QtCore.pyqtSignal(object, object)


    def __init__(self, dict_config):
        super(ModelManage, self).__init__()
        if int(dict_config['developer']['Ui_stytle']) == 0:
            self.h_ui = Ui_ParamSetGraphic()
            self.h_ui.setupUi(self)
        else:
            self.h_ui = Ui_ParamSetGraphic_lunzhuan()
            self.h_ui.setupUi(self)
        #参数路径，读配置
        self.s_dirpath = 'd:\\Param\\'
        #模板管理文件
        self.s_configfilepath = 'd:\\Param\\TemplateName.xml'
        # 已经存在的模板名列表
        self.list_allmodelname = []
        # 当前已经挂入tableview的数量
        self.n_currentlistrow = 0
        # 当前被载入模板的索引
        self.n_curmodelindex = None
        # 用于搜索框的变量
        self.list_matchlist = []
        # 存放TemplateName.yaml文件的数据
        self.list_data = []
        #
        self.n_maxrowcount = 2000
        # 自动补全
        self.completer = None
        self.h_stringlistmodel = None
        self.list_completerstr = []
        # 读系统配置xml文件，并转化为字典
        self.dict_sysparams = dict_config
        # 往模板列表添加模板的字体类型
        self.h_tablefont = QtGui.QFont()
        self.h_tablefont.setFamily("Times")
        self.h_tablefont.setPointSize(15)

        # 设置logger在终端输出
        # self.setlogger()
        # self,createdir()
        # 初始自动补全
        self.setcompleter()
        # 创建文件夹
        self.createdirpath()
        self.__inituirest()

        # if not os.path.isdir(self.s_dirpath)):
        #     try:
        #         os.makedirs(self.s_dirpath + s_modelname.decode('utf-8'))
        self.h_ui.h_addnewbtn.clicked.connect(self.slotaddnewmodel)
        self.h_ui.h_clonebtn.clicked.connect(self.slotclonebtnclicked)
        self.h_ui.h_loadbtn.clicked.connect(self.slotloadbtnclicked)
        self.h_ui.h_delbtn.clicked.connect(self.slotdelbtnclicked)
        self.h_ui.h_searchedit.textChanged.connect(self.slotsearch)
        self.h_ui.h_searchedit.returnPressed.connect(self.slotsearch_enterkey)
        #self.h_ui.h_modeltableview.clicked.connect(self.slottableviewindexchange)



    def lock(self):
        """
        外层界面的需要，断开信号
        """
        self.h_ui.h_addnewbtn.setEnabled(False)
        self.h_ui.h_clonebtn.setEnabled(False)
        self.h_ui.h_loadbtn.setEnabled(False)
        self.h_ui.h_delbtn.setEnabled(False)
        self.h_ui.h_searchedit.setEnabled(False)
        self.h_ui.h_modeltableview.setEnabled(False)


    def unlock(self):
        """
        外层界面的需要，断开信号之后重接信号
        """
        self.h_ui.h_addnewbtn.setEnabled(True)
        self.h_ui.h_clonebtn.setEnabled(True)
        self.h_ui.h_loadbtn.setEnabled(True)
        self.h_ui.h_delbtn.setEnabled(True)
        self.h_ui.h_searchedit.setEnabled(True)
        self.h_ui.h_modeltableview.setEnabled(True)


    def lockload(self):
        self.h_ui.h_loadbtn.setEnabled(False)
        self.h_ui.h_searchedit.setEnabled(False)
        self.h_ui.h_modeltableview.setEnabled(False)

    def unlockload(self):
        self.h_ui.h_loadbtn.setEnabled(True)
        self.h_ui.h_searchedit.setEnabled(True)
        self.h_ui.h_modeltableview.setEnabled(True)

    def createdirpath(self):
        """
        从ApplicationSetting.xml获取文件路径，包括存取模板的位置、记录模板信息的xml位置
        :return:
        """
        if self.dict_sysparams is not None:
            self.s_dirpath = self.dict_sysparams[u'paramsdir']
            if not os.path.isdir(self.s_dirpath):
                os.mkdir(self.s_dirpath)
            self.s_configfilepath = self.s_dirpath + '\TemplateName.xml'
            self.n_substationcount = len(self.dict_sysparams[u'substation'])

            self.list_stationname_english = []# 站点名称，也即d:\\param\\0 指的就是0这个文件夹名
            for nindex, str_station in enumerate(list(self.dict_sysparams[u'substation'])):
                self.list_stationname_english.append(str(nindex))

    def __inituirest(self):
        """
        会命名这个函数的原因是因为我希望将UI界面的图形构造与逻辑尽量分开，而modelmanageui.py
        是用qtdesigner生成，如果直接修改该文件及其不方便维护，所以在该类中放置一个函数完成ui文件
        未完成的工作
        """
        # self.h_ui.h_label.setPixmap(QtGui.QPixmap("res\lookfor.ico").scaled(
        #     self.h_ui.h_label.width() / 2, self.h_ui.h_label.height()))
        # self.h_ui.h_progressBar.setVisible(False)

        self.__initmodellist()
        #self.__initimageview()

    def __initmodellist(self):
        """
        这个函数是初始模板列表，在setupUI中只是初始了列表类，这个函数是初始列表头名以及项，
        函数使用的是旧版代码，我只修改了变量名
        而因为tableview因为程序的需要经常需要添加删除模板等，所以暂时不确定要不要设为私有成员
        """

        self.h_ui.h_modeltableview.setEditTriggers(QtWidgets.QAbstractItemView.NoEditTriggers)
        self.h_ui.h_modeltableview.setSelectionBehavior(QtWidgets.QAbstractItemView.SelectRows)
        s_headertitle = (self.tr(u"模板名"), self.tr(u"创建者"), self.tr(u"创建时间"), self.tr(u"备注"))

        n_columncount = len(s_headertitle)
        # print n_columnwidth
        self.h_modellistmodel = QtGui.QStandardItemModel()

        self.h_modellistmodel.setRowCount(self.n_maxrowcount)

        self.h_modellistmodel.setColumnCount(n_columncount)
        for index in range(0, n_columncount):
            self.h_modellistmodel.setHeaderData(index, QtCore.Qt.Horizontal, s_headertitle[index])
            self.h_modellistmodel.horizontalHeaderItem(index).setFont(QtGui.QFont("Times", 18, QtGui.QFont.Black))

        self.h_ui.h_modeltableview.setModel(self.h_modellistmodel)

        self.h_ui.h_modeltableview.verticalHeader().setHidden(True)

        self.h_ui.h_modeltableview.setAlternatingRowColors(True)
        self.h_ui.h_modeltableview.setStyleSheet(
            "background-color: rgb(255, 255, 255); "
            "alternate-background-color: rgb(237, 244, 253);"
            #"alternate-background-color: rgb(164, 221, 254);"
        )
        # hyh 2018/08/16 设置表头的颜色
        if int(self.dict_sysparams['developer']['Ui_stytle']) == 0:
            self.h_ui.h_modeltableview.verticalHeader().setStyleSheet(
                "QHeaderView::section{background:rgb(159, 200, 193);}")
            self.h_ui.h_modeltableview.horizontalHeader().setStyleSheet(
                "QHeaderView::section{background:rgb(159, 200, 193);}")
        self.h_ui.h_modeltableview.horizontalHeader().setDefaultAlignment(QtCore.Qt.AlignCenter)
        for index in range(0, n_columncount):
            self.h_ui.h_modeltableview.horizontalHeader().setResizeMode(index, QtWidgets.QHeaderView.Fixed)
        for i in range(self.n_maxrowcount):
            self.h_ui.h_modeltableview.setRowHeight(i, 50)
        self.h_ui.h_modeltableview.horizontalHeader().setStretchLastSection(True)
        self.h_ui.h_modeltableview.horizontalHeader().setResizeMode(QtWidgets.QHeaderView.Stretch)

        self.__loadmodellist()

    def __initimageview(self):
        """
        初始化显图的widget，当前跟旧版UI差别比较大便是这部分，在widget上挂上的是pyqtgraph的
        显图界面;
        另外，变量大部分为私有成员，原因是该函数本身就是为补充本类的UI类的图形而添加的，
        该函数及该函数的新建的成员变量不想被外界包括本类的子类所使用（留下self.m_himg是需要修改其显示的图片）
        """

        self.__list_graphicsView = []
        self.__list_view = []
        self.list_baseimg = []
        if len(self.list_stationname_english) > 2:
            self.list_showimg_stationname_english = self.list_stationname_english
        else:
            self.list_showimg_stationname_english = self.list_stationname_english
        for stationname_english in self.list_showimg_stationname_english:
            graphicsView = pg.GraphicsView(self.h_ui.h_widgetview)
            self.__list_graphicsView.append(graphicsView)
            self.h_ui.layout_imageview.addWidget(graphicsView)

            view = pg.ViewBox(invertY=True, enableMenu=False)
            self.__list_view.append(view)
            graphicsView.setCentralItem(view)
            view.setAspectLocked(True)

            img = pg.ImageItem()
            self.list_baseimg.append(img)
            view.addItem(img)

    def __loadmodellist(self):
        """
        由ApplicationSetting.xml读取存放数据的路径，然后根据该路径下的TemplateName.xml读取已经创建的模板，
        并把他们加载进来
        """
        #TODO 转python3这里可能会有问题
        if os.path.isfile(self.s_configfilepath):
            # 解决xml读取中文模板失败的问题
            f = open(self.s_configfilepath, 'r')
            r = f.read()
            text = str(r.encode("utf-8"), encoding='utf-8')
            dom1 = xml.dom.minidom.parseString(text)

            root = dom1.documentElement
            booknode = root.getElementsByTagName('work')
            self.list_allmodelname = []
            for booklist in booknode:
                dict_dict = {}
                index = int(booklist.getAttribute('index'))
                s_creator = str(booklist.getElementsByTagName('Creator')[0].childNodes[0].nodeValue).strip()
                s_modelnames = str(booklist.getElementsByTagName('Modelname')[0].childNodes[0].nodeValue).strip()
                s_remark = str(booklist.getElementsByTagName('Remark')[0].childNodes[0].nodeValue).strip()
                s_time = str(booklist.getElementsByTagName('Time')[0].childNodes[0].nodeValue).strip()
                dict_dict = {'index': index, 'Creator': s_creator, 'Modelname': s_modelnames, 'Time': s_time,
                             'Remark': s_remark}
                # self.list_allmodelname.append(s_modelnames.decode('utf-8'))
                self.list_allmodelname.append(s_modelnames)
                self.list_data.append(dict_dict)
            # print self.list_data
            # 首项是载入项重复了要删除
            if len(self.list_allmodelname) != 0:
                del self.list_allmodelname[0]
        else:
            self.writeconfigfile()

        for n_i in range(1, len(self.list_data)):
            s_modelname = self.list_data[n_i]['Modelname']
            s_creatorname = self.list_data[n_i]['Creator']
            s_createtime = self.list_data[n_i]['Time']
            s_exinfo = self.list_data[n_i]['Remark']
            # 下面四组都是往表上添加对应的项目
            h_modelnameitem = QtGui.QStandardItem(s_modelname)
            h_modelnameitem.setTextAlignment(QtCore.Qt.AlignCenter)
            h_modelnameitem.setFont(self.h_tablefont)
            self.h_modellistmodel.setItem(self.n_currentlistrow, 0, h_modelnameitem)

            h_creatornameitem = QtGui.QStandardItem(s_creatorname)
            h_creatornameitem.setTextAlignment(QtCore.Qt.AlignCenter)
            h_creatornameitem.setFont(self.h_tablefont)
            self.h_modellistmodel.setItem(self.n_currentlistrow, 1, h_creatornameitem)

            h_createtimeitem = QtGui.QStandardItem(s_createtime)
            h_createtimeitem.setTextAlignment(QtCore.Qt.AlignCenter)
            h_createtimeitem.setFont(self.h_tablefont)
            self.h_modellistmodel.setItem(self.n_currentlistrow, 2, h_createtimeitem)

            h_exinfoitem = QtGui.QStandardItem(s_exinfo)
            h_exinfoitem.setTextAlignment(QtCore.Qt.AlignCenter)
            h_exinfoitem.setFont(self.h_tablefont)
            self.h_modellistmodel.setItem(self.n_currentlistrow, 3, h_exinfoitem)
            self.n_currentlistrow += 1

    def setfirst(self):
        """
        这是为了程序一开始启动的时候外部调用载入当前模板项
        （之所以需要外部调用是因为一开始该类并未跟参数项的类挂上联系）
        """
        if self.list_data != []:
            s_row = int(self.list_data[0]['index'])
            self.slotloadbtnclicked(s_row)
            # self.setcompleter()
        else:
            # 告诉界面现在没有模板，现在参数项界面为空
            self.h_paramchanges.emit(None, False)

    def getcurmodelnum(self):
        """获取当前模板数量，目的是确定是否显示模板"""
        return len(self.list_data)

    def create_element(self, doc, tag, attr):
        """
        创建xml所需
        """
        # 创建一个元素节点
        elementnode = doc.createElement(tag)
        # 创建一个文本节点
        textnode = doc.createTextNode(attr)
        # 将文本节点作为元素节点的子节点
        elementnode.appendChild(textnode)
        return elementnode

    def writeconfigfile(self):
        """
        函数设计的用途是将更新的数据写入self.list_data并保存到TemplateName.yaml
        """
        dom1 = xml.dom.getDOMImplementation()  # 创建文档对象，文档对象用于创建各种节点。
        doc = dom1.createDocument(None, "info", None)
        top_element = doc.documentElement  # 得到根节点
        # print self.list_data
        for n_i in range(len(self.list_data)):
            snode = doc.createElement('work')
            snode.setAttribute('index', str(self.list_data[n_i]["index"]))
            modelnode = self.create_element(doc, 'Modelname', self.list_data[n_i]['Modelname'])
            snode.appendChild(modelnode)
            creatornode = self.create_element(doc, 'Creator', self.list_data[n_i]["Creator"])
            snode.appendChild(creatornode)
            marknode = self.create_element(doc, 'Remark', self.list_data[n_i]["Remark"])
            snode.appendChild(marknode)
            timenode = self.create_element(doc, 'Time', self.list_data[n_i]["Time"])
            snode.appendChild(timenode)
            top_element.appendChild(snode)
        # print self.list_data
        # if not os.path.isfile(self.s_configfilepath):
            # os.mkdir(self.s_configfilepath)
            # fd = open(self.s_configfilepath, mode="w", encoding="utf-8")
            # fd.close()
        xmlfile = open(self.s_configfilepath, 'w')#当文件不存在时也可自动生成初始化文件 20200203
        doc.writexml(xmlfile, addindent=' ' * 4, newl='\n', encoding='utf-8')
        xmlfile.close()


        # #TODO:yaml版本
        # h_stream = file(self.s_configfilepath, 'w')
        # yaml.dump(self.list_data, h_stream)
        # h_stream.close()

    # def outsidetoupdateimageview(self):
    #     if self.n_curmodelindex is not None:
    #         self.updateimageview(self.n_curmodelindex)

    def updateimageview(self, n_index=None):
        """
        函数用来供内外部调用更新图片
        """
        if n_index is not None:
            s_modelname = str(self.h_modellistmodel.item(n_index, 0).text()).strip()

            #self.choosepicture(self.s_dirpath + s_modelname)
            for i in range(len(self.list_baseimagepath)):
                s_baseimagepath = self.list_baseimagepath[i]
                if os.path.isfile(s_baseimagepath):
                    h_img = np.array(Image.open(s_baseimagepath))
                    self.list_baseimg[i].setImage(h_img)
                else:
                    self.__list_view[i].removeItem(self.list_baseimg[i])
                    self.list_baseimg[i] = pg.ImageItem()
                    self.__list_view[i].addItem(self.list_baseimg[i])

    def choosepicture(self, s_templatepath):
        '''
        得到底板图路径列表
        '''
        self.list_baseimagepath = []
        list_curtemplate_station_path = []
        for stationname_english in self.list_showimg_stationname_english:
            s_curtemplate_station_path = s_templatepath + '\\' + stationname_english
            if not os.path.isdir(s_curtemplate_station_path):
                os.mkdir(s_curtemplate_station_path)
            list_curtemplate_station_path.append(s_curtemplate_station_path)
            s_imagepath = s_curtemplate_station_path + '\\BaseImg.bmp'
            self.list_baseimagepath.append(s_imagepath)
        return self.list_baseimagepath

    def resizeEvent(self, event):
        """
        函数重载了QtGui.QWidget.resizeEvent，使界面大小变化的时候h_modellistmodel会随着变化
        """
        pass
        # n_columncount = self.h_modellistmodel.columnCount()
        # n_columnwidth = self.h_ui.h_modeltableview.geometry().width() / n_columncount * 0.96
        # for index in range(0, n_columncount):
        #     self.h_ui.h_modeltableview.horizontalHeader().setResizeMode(index, QtGui.QHeaderView.Fixed)
        #     self.h_ui.h_modeltableview.setColumnWidth(index, 100)
        # QtGui.QWidget.resizeEvent(self, event)

    def slotaddnewmodel(self):
        """
        对应于新建按钮的信号槽，
        功能：在self.h_ui.h_modeltableview上添加相应的模板，并根据相应的默认目录（程序初始化时应给出）
        新建默认的.dat文件
        """
        h_addmodeldialog = KxAddNewModelDialog(self)
        h_addmodeldialog.setWindowTitle(self.tr('New/Import offline template'))
        if h_addmodeldialog.exec_() == QtWidgets.QDialog.Accepted:
            s_modelname = str(h_addmodeldialog.getModelNameText()).strip()
            s_creatorname = h_addmodeldialog.getCreatorNameText()
            s_exinfo = h_addmodeldialog.getExInfoText()
        else:
            return
        if s_modelname == '':
            return
        if s_modelname in self.list_allmodelname:
            QtWidgets.QMessageBox.warning(self, self.tr('Warning'),
                                      self.tr('There is a template with the same name local!'), QtGui.QMessageBox.Ok)
            return

        if s_creatorname == '':
            s_creatorname = 'Undefined'
        if s_exinfo == '':
            s_exinfo = 'Nothing'
        s_createtime = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()))

        if self.createdir_and_initdate(s_modelname):
            # 下面四组都是往表上添加对应的项目
            h_modelnameitem = QtGui.QStandardItem(s_modelname)
            h_modelnameitem.setTextAlignment(QtCore.Qt.AlignCenter)
            h_modelnameitem.setFont(self.h_tablefont)
            self.h_modellistmodel.setItem(self.n_currentlistrow, 0, h_modelnameitem)

            h_creatornameitem = QtGui.QStandardItem(s_creatorname)
            h_creatornameitem.setTextAlignment(QtCore.Qt.AlignCenter)
            h_creatornameitem.setFont(self.h_tablefont)
            self.h_modellistmodel.setItem(self.n_currentlistrow, 1, h_creatornameitem)

            h_createtimeitem = QtGui.QStandardItem(s_createtime)
            h_createtimeitem.setTextAlignment(QtCore.Qt.AlignCenter)
            h_createtimeitem.setFont(self.h_tablefont)
            self.h_modellistmodel.setItem(self.n_currentlistrow, 2, h_createtimeitem)

            h_exinfoitem = QtGui.QStandardItem(s_exinfo)
            h_exinfoitem.setTextAlignment(QtCore.Qt.AlignCenter)
            h_exinfoitem.setFont(self.h_tablefont)
            self.h_modellistmodel.setItem(self.n_currentlistrow, 3, h_exinfoitem)

            # 加上新建模板的名字
            self.list_allmodelname.append(s_modelname)
            # 将上一段底色置白
            if self.n_curmodelindex is not None:
                for i in range(4):
                    self.h_modellistmodel.item(self.n_curmodelindex, i).setBackground(
                        QtGui.QColor(0, 0, 0, 0))
            # 添加完模板便载入，故为其本身
            self.n_curmodelindex = self.n_currentlistrow
            # 将成功载入的模板项置为黄色
            for i in range(4):
                self.h_modellistmodel.item(self.n_curmodelindex, i).setBackground(
                    QtGui.QColor(238, 207, 161))
            #self.updateimageview(self.n_curmodelindex)
            self.n_currentlistrow = self.n_currentlistrow + 1
            dict_newmodeinfo = {'Modelname': s_modelname, 'Creator': s_creatorname, 'Time': s_createtime,
                                'Remark': s_exinfo, 'index': self.n_curmodelindex}
            # todict = dict_newmodeinfo
            todict = copy.deepcopy(dict_newmodeinfo)
            if self.list_data == []:
                self.list_data.append(dict_newmodeinfo)
            else:
                self.list_data[0] = dict_newmodeinfo
            self.list_data.append(todict)
            self.writeconfigfile()
            # self.setcompleter()

    def slotclonebtnclicked(self):
        """
        对应于复制按钮的信号槽，
        功能：在self.h_ui.h_modeltableview上复制选中的模板，并根据相应的默认目录（程序初始化时应给出）
        新建默认的.dat文件
        """
        n_row = self.h_ui.h_modeltableview.currentIndex().row()
        if n_row < 0 or n_row >= self.n_currentlistrow:
            logging.debug("user choose the row out of range")
            return
        s_modelname = str(self.h_modellistmodel.item(n_row, 0).text()).strip()
        self.clonemodel(s_modelname, n_row)

    def clonemodel(self, s_modelnames=None, n_rows=None):
        """
        复制模板，被复制键的槽函数调用，所做的事与self.slotaddnewmodel()接近
        """
        if s_modelnames and n_rows is not None:
            s_oldmodelname = s_modelnames
            n_row = n_rows
        else:
            return
        # 当前欲复制模板为载入的模板，提示一下
        if self.n_curmodelindex == n_row:
            QtWidgets.QMessageBox.warning(self, self.tr('Warning'),
                self.tr('The template you are currently trying to copy is loading!'),
                                      QtWidgets.QMessageBox.Ok)

        h_addmodeldialog = KxAddNewModelDialog(self)
        # addModelDialog.setStationAssignmentStatusList(self.stationAssignmentStatusWidgetList[row].StationAssignmentStatusList)
        h_addmodeldialog.setModelNameText('copy_' + s_oldmodelname)
        h_addmodeldialog.setWindowTitle(self.tr('New/Import offline template'))
        if h_addmodeldialog.exec_() == QtWidgets.QDialog.Accepted:
            s_modelcopyname = str(h_addmodeldialog.getModelNameText()).strip()
            s_creatorname = h_addmodeldialog.getCreatorNameText()
            s_exinfo = h_addmodeldialog.getExInfoText()
        else:
            return
        if s_modelcopyname == '':
            return
        if s_modelcopyname in self.list_allmodelname:
            QtWidgets.QMessageBox.warning(self, self.tr('Warning'),
                                      self.tr('There is a template with the same name local!'), QtWidgets.QMessageBox.Ok)
            return

        if s_creatorname == '':
            s_creatorname = 'Undefined'
        if s_exinfo == '':
            s_exinfo = 'Nothing'
        s_createtime = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()))

        if self.copydir_and_date(s_oldmodelname, s_modelcopyname):
            # 下面四组都是往表上添加对应的项目
            h_modelnameitem = QtGui.QStandardItem(s_modelcopyname)
            h_modelnameitem.setTextAlignment(QtCore.Qt.AlignCenter)
            h_modelnameitem.setFont(self.h_tablefont)
            self.h_modellistmodel.setItem(self.n_currentlistrow, 0, h_modelnameitem)

            h_creatornameitem = QtGui.QStandardItem(s_creatorname)
            h_creatornameitem.setTextAlignment(QtCore.Qt.AlignCenter)
            h_creatornameitem.setFont(self.h_tablefont)
            self.h_modellistmodel.setItem(self.n_currentlistrow, 1, h_creatornameitem)

            h_createtimeitem = QtGui.QStandardItem(s_createtime)
            h_createtimeitem.setTextAlignment(QtCore.Qt.AlignCenter)
            h_createtimeitem.setFont(self.h_tablefont)
            self.h_modellistmodel.setItem(self.n_currentlistrow, 2, h_createtimeitem)

            h_exinfoitem = QtGui.QStandardItem(s_exinfo)
            h_exinfoitem.setTextAlignment(QtCore.Qt.AlignCenter)
            h_exinfoitem.setFont(self.h_tablefont)
            self.h_modellistmodel.setItem(self.n_currentlistrow, 3, h_exinfoitem)

            # 加上新建模板的名字
            self.list_allmodelname.append(s_modelcopyname)
            # 将上一段底色置白
            if self.n_curmodelindex is not None:
                for i in range(4):
                    self.h_modellistmodel.item(self.n_curmodelindex, i).setBackground(
                        QtGui.QColor(0, 0, 0, 0))
            # 添加完模板便载入，故为其本身
            self.n_curmodelindex = self.n_currentlistrow
            # 将成功载入的模板项置为黄色
            for i in range(4):
                self.h_modellistmodel.item(self.n_curmodelindex, i).setBackground(
                    QtGui.QColor(238, 207, 161))
            self.updateimageview(self.n_curmodelindex)
            self.n_currentlistrow = self.n_currentlistrow + 1
            dict_newmodeinfo = {'Modelname': s_modelcopyname, 'Creator': s_creatorname, 'Time': s_createtime,
                                'Remark': s_exinfo, 'index': self.n_curmodelindex}
            # todict = dict_newmodeinfo
            todict = copy.deepcopy(dict_newmodeinfo)
            if self.list_data == []:
                self.list_data.append(dict_newmodeinfo)
            else:
                self.list_data[0] = dict_newmodeinfo

            self.list_data.append(todict)
            self.writeconfigfile()
            # self.setcompleter()

    def msgloadmodel(self, s_mdoelname):
        self.list_msgloadmodel = []
        self.list_msgloadmodel.extend(
            self.h_modellistmodel.findItems(s_mdoelname, QtCore.Qt.MatchContains, 0))
        if len(self.list_msgloadmodel) != 0:
            n_row = self.list_msgloadmodel[0].row()
            if self.h_modellistmodel.item(n_row, 0).text() == s_mdoelname:
                self.slotloadbtnclicked(n_row)
                return True

    def slotloadbtnclicked(self, n_rows=None):
        """
        对应于载入按钮的信号槽，但是函数不只为它设计，在搜索框enter时也会调用这个函数，它可以当做一个载入函数使用
        功能：将.dat数据载入到参数项界面中去
        """
        "槽函数传递过来的参数为False = 0，所以载入第一个模板"
        if n_rows is not None and n_rows is not False:
            # print("--------------", n_rows)
            n_row = n_rows
        else:
            n_row = self.h_ui.h_modeltableview.currentIndex().row()

        if n_row < 0 or n_row >= self.n_currentlistrow:
            logging.debug("user choose the row out of range")
            return
        # 如果载入项为当前项，则无视
        if n_row == self.n_curmodelindex:
            return
        else:
            #self.h_ui.h_progressBar.setVisible(True)
            s_modelname = str(self.h_modellistmodel.item(n_row, 0).text())
            s_creatorname = str(self.h_modellistmodel.item(n_row, 1).text())
            s_createtime = str(self.h_modellistmodel.item(n_row, 2).text())
            s_exinfo = str(self.h_modellistmodel.item(n_row, 3).text())

            self.h_paramchanges.emit(self.s_dirpath + s_modelname + '\\', True)

            # 将上一段底色置白
            if self.n_curmodelindex is not None:
                for i in range(4):
                    self.h_modellistmodel.item(self.n_curmodelindex, i).setBackground(
                        QtGui.QColor(0, 0, 0, 0))
            self.n_curmodelindex = n_row
            for i in range(4):
                self.h_modellistmodel.item(self.n_curmodelindex, i).setBackground(
                    QtGui.QColor(238, 207, 161))
            dict_newmodeinfo = {'Modelname': s_modelname, 'Creator': s_creatorname, 'Time': s_createtime,
                                'Remark': s_exinfo, 'index': self.n_curmodelindex}
            #             todict = dict_newmodeinfo
            # todict = copy.deepcopy(dict_newmodeinfo)
            # todict1 = copy.deepcopy(dict_newmodeinfo)
            # list_data首项为当前加载项
            if self.list_data == []:
                self.list_data.append(dict_newmodeinfo)
            else:
                self.list_data[0] = dict_newmodeinfo
                # print 'here'
            # print dict_newmodeinfo
            self.writeconfigfile()
            # self.list_data.append(todict)
            #self.updateimageview(self.n_curmodelindex)
            #self.h_ui.h_progressBar.setVisible(False)
            # self.setcompleter()

    def slotdelbtnclicked(self):
        """
        对应于删除按钮的信号槽，
        功能：在self.h_xui.h_modeltableview上删除选中的模板，并删除该模板对应的文件
        """
        n_row = self.h_ui.h_modeltableview.currentIndex().row()
        if n_row < 0 or n_row >= self.n_currentlistrow:
            logging.debug("user choose the row out of range")
            return
        # 要删除的模板为当前载入模板，不可删
        if n_row == self.n_curmodelindex:
            QtWidgets.QMessageBox.warning(self, self.tr('Warning'), self.tr('The current template cannot be deleted!'),
                                      QtWidgets.QMessageBox.Ok)
            return

        h_rb =  QtWidgets.QMessageBox.question(self, self.tr('Warning'), self.tr('Ensure to delete the template?'),
                                          QtWidgets.QMessageBox.Ok | QtWidgets.QMessageBox.Cancel)
        if h_rb == QtWidgets.QMessageBox.Cancel:
            return
        else:
            if self.delmodel(n_row):
                # print n_row
                self.n_currentlistrow -= 1
                if n_row < self.n_curmodelindex:
                    self.n_curmodelindex -= 1
                # 因为首项是载入项，所以加一
                # del self.list_data[n_row + 1]
                self.changeindex(n_row)
                # self.list_data[0] -= 1
                self.writeconfigfile()

    def delmodel(self, n_row=None):
        """
        根据索引行来删除模板
        """
        if n_row != None:
            n_row = n_row
        else:
            logging.warning("the n_row is None, check")
            return False
        s_modelname = str(self.h_modellistmodel.item(n_row, 0).text()).strip()

        # del self.stationAssignmentStatusWidgetList[row]
        # 删掉文件，成功就把逻辑上的东西也删了
        if self.deletedir(s_modelname):
            self.h_modellistmodel.removeRow(n_row)
            # self.setcompleter()
            for n_name in range(len(self.list_allmodelname)):
                if s_modelname == self.list_allmodelname[n_name]:
                    del self.list_allmodelname[n_name]
                    return True
        else:
            return False

    def changeindex(self, n_row=None):
        """
        当删除一个模板之后，因为前期写的保存模板数据不够全面问题，所以该函数的存在就是修正
        删除一个模板之后其他模板的各项索引不变化的问题
        """
        if n_row is not None:
            if n_row < int(self.list_data[0]['index']):
                self.list_data[0]['index'] = str(int(self.list_data[0]['index']) - 1)
            for n_i in range(n_row + 1 + 1, len(self.list_data)):
                self.list_data[n_i]['index'] = str(int(self.list_data[n_i]['index']) - 1)
            # 因为首项是载入项，所以加一
            del self.list_data[n_row + 1]

    def slotsearch(self):
        """
        对应于搜索框的信号槽，
        功能：搜索框中文字变化时查找文字对应的模板，并将搜索到的模板底色置蓝
        """
        self.list_selecteditemlist = []
        # print self.n_curmodelindex
        # 把除了载入项刷黄其他项全部重新刷白
        for i in range(len(self.list_matchlist)):
            if self.list_matchlist[i].row() != self.n_curmodelindex:
                self.h_modellistmodel.item(self.list_matchlist[i].row(), self.list_matchlist[i].column()).setBackground(
                    QtGui.QBrush(QtCore.Qt.white))
            else:
                self.h_modellistmodel.item(self.list_matchlist[i].row(), self.list_matchlist[i].column()).setBackground(
                    QtGui.QBrush(QtCore.Qt.yellow))

        if self.h_ui.h_searchedit.text() != '':
            self.list_matchlist = []
            self.list_matchlist.extend(
                self.h_modellistmodel.findItems(self.h_ui.h_searchedit.text(), QtCore.Qt.MatchContains, 0))

            self.list_completerstr = []
            for i in range(len(self.list_matchlist)):
                self.h_modellistmodel.item(self.list_matchlist[i].row(), self.list_matchlist[i].column()).setBackground(
                    QtGui.QBrush(QtCore.Qt.blue))
                self.list_completerstr.append(self.h_modellistmodel.item(self.list_matchlist[i].row(),
                                                                         self.list_matchlist[i].column()).text())
            self.setcompleter()
            # bool_flag = False
            # for i in range(len(self.list_matchlist)):
            #     if self.list_matchlist[i].text() == self.h_ui.h_searchedit.text():
            #         bool_flag = True
            #         self.list_selecteditemlist.append((self.list_matchlist[i].row(), self.list_matchlist[i].column()))
            #         self.h_modellistmodel.item(self.list_matchlist[i].row(), self.list_matchlist[i].column()).setBackground(
            #             QtGui.QBrush(QtCore.Qt.blue))

            # if bool_flag is False:
            #     for i in range(len(self.list_matchlist)):
            #         self.list_selecteditemlist.append((self.list_matchlist[i].row(), self.list_matchlist[i].column()))
            #         self.h_modellistmodel.item(self.list_matchlist[i].row(), self.list_matchlist[i].column()).setBackground(
            #             QtGui.QBrush(QtCore.Qt.blue))

    def slotsearch_enterkey(self):
        """
        对应于搜索框输入文字进行时用户按了“enter”键
        功能：如果输入文字有对应的模板，则立刻载入该模板，也即调用self.loadbtnclicked()

        """
        flag = False
        # for i in range(len(self.list_matchlist)):
        #     if self.list_matchlist[i].text() == self.h_ui.h_searchedit.text():
        #         flag = True
        #         n_row = self.list_matchlist[i].row()
        #         print "there"
        #         self.slotloadbtnclicked(n_row)
        if flag is False:
            for i in range(len(self.list_matchlist)):
                if i == 0:
                    n_row = self.list_matchlist[i].row()
                    self.slotloadbtnclicked(n_row)

    def slottableviewindexchange(self):
        """
        当鼠标点击模板项目的时候，图层会显示该模板的照片
        """
        n_row = self.h_ui.h_modeltableview.currentIndex().row()
        if n_row >= 0 and n_row < self.n_currentlistrow:
            self.updateimageview(n_row)

    def setcompleter(self):
        """
        设置自动补全供能
        """
        if self.completer is not None:
            self.h_stringlist.clear()
            for s_name in self.list_completerstr:
                self.h_stringlist.append(s_name)
            self.h_stringlistmodel.setStringList(self.h_stringlist)
            self.completer.setModel(self.h_stringlistmodel)
        else:
            self.completer = QtWidgets.QCompleter(self.list_allmodelname)
            self.h_stringlistmodel = QtCore.QStringListModel()
            self.h_stringlist = []
            self.completer.setCompletionMode(QtWidgets.QCompleter.UnfilteredPopupCompletion)
            self.completer.setCaseSensitivity(QtCore.Qt.CaseSensitive)
            self.h_ui.h_searchedit.setCompleter(self.completer)

    def setcomplte_activated_slot(self):
        self.slotsearch()
        self.slotsearch_enterkey()

    def createdir_and_initdate(self, s_modelname=None):
        """
        根据师兄说的xml格式读取文件，然后相应的根据参数项的界面项来创建文件并保存数据，这里创建的文件
        的路径是由self变量确定。
        """
        # 创建文件夹
        if not os.path.isdir(self.s_dirpath + s_modelname):
            try:
                os.makedirs(self.s_dirpath + s_modelname)
            except:
                QtWidgets.QMessageBox.warning(self, self.tr('Warning'), self.tr('Template name has invalid characters!'),
                                          QtWidgets.QMessageBox.Ok)
                return False

        else:
            QtWidgets.QMessageBox.warning(self, self.tr('Warning'),
                                      self.tr('There is a directory with the same name local!'), QtWidgets.QMessageBox.Ok)
            return False
        # 送出信号，告知模板已变
        self.h_paramchanges.emit(self.s_dirpath + s_modelname + '\\', True)
        return True

    def copydir_and_date(self, oldmodelname, newmodelname):
        if not os.path.isdir(self.s_dirpath + newmodelname):
            try:
                shutil.copytree(self.s_dirpath + oldmodelname, self.s_dirpath + newmodelname)
            except:
                QtWidgets.QMessageBox.warning(self, self.tr('Warning'), self.tr('Template name has invalid characters!'),
                                          QtWidgets.QMessageBox.Ok)
                return False
        else:
            QtWidgets.QMessageBox.warning(self, self.tr('Warning'),
                                      self.tr('There is a directory with the same name local!'), QtWidgets.QMessageBox.Ok)
            return False
        # 送出信号，告知模板已变
        self.h_paramchanges.emit(self.s_dirpath + newmodelname + '\\', True)
        return True

    def deletedir(self, s_modelname=None):
        """
        删除模板所在位置的文件夹
        """
        if s_modelname is not None:
            if os.path.isdir(self.s_dirpath + s_modelname):
                try:
                    shutil.rmtree(self.s_dirpath + s_modelname)
                except:
                    QtWidgets.QMessageBox.warning(self, self.tr('Warning'),
                                              self.tr('delete fail!'),
                                              QtWidgets.QMessageBox.Ok)
                    return False
            else:
                QtWidgets.QMessageBox.warning(self, self.tr('Warning'),
                                          self.tr('There have no model directory!'),
                                          QtWidgets.QMessageBox.Ok)
            return True


            # class Completer(QtGui.QCompleter):
            #     def __init__(self, stringlist, parent=None):
            #         super(Completer, self).__init__(parent)
            #         self.stringlist = stringlist
            #         self.setModel(QtGui.QStringListModel())
            #
            #     def update(self, completionText):
            #         h_stringlist = QtCore.QStringList()
            #         for str in completionText:
            #             h_stringlist << str
            #         filtered = self.stringlist.filter(h_stringlist, QtCore.Qt.CaseInsensitive)
            #         self.model().setStringList(filtered)
            #         self.popup().setCurrentIndex(self.model().index(0, 0))



