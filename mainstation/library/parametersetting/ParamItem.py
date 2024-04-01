# -*- coding: utf-8 -*-
"""use for manage subwidget"""
import os
import sys
# sys.path.append("../")
import copy
import logging
import xml.dom.minidom
from PyQt5 import QtGui, QtCore, QtWidgets
# from pyqtgraph.ordereddict import *
from collections import OrderedDict
import imc_msg

from library.parametersetting.ParamItemPY import *
from library.parametersetting.ParamItemPY.KxBaseWidget import *
from library.common.readsysparams import *
from library.common.ZCTabwidget import ZCTabwidget
from library.common.globalparam import DataPipeLineType
from library.common.globalparam import ImportantName

class ParamItem(QtWidgets.QWidget):
    """
    2020.02.20 这个类主要作用是挂参数树的界面，特别主要的是默认一定会存在一份global的配置，
    也即呈现 global、check1、check2.....等等，界面id从0开始索引，在初始化这些参数py的时候，
    会传入两个参数：
    n_uid       (1)参数界面号，作用是给参数界面唯一的标识符，当需要发送信号给子站时与其他参数界面区分
    n_areanum   (2)设置区域数量，注意，这个区域号其实只有global参数才会用到，对于其他的界面来说作用不大,但也要加；
                    这个参数表示一共用多少份check参数，每份参数其实都是一种检查方法，至于使用“区域”这个名称是传统
    每次更换模板触发时，会触发两个函数:
    1.setmodeldirectory()：设置其保存参数的路径，一般为配置中的模板路径+模板名+参数名+.xml
    2.loadparameters()：载入模板，初始化好参数以及设置完参数路径之后，载入模板

    """
    def __init__(self, h_parentwidget, n_stationid, dict_config):
        super(ParamItem, self).__init__()
        self.h_parentwidget = h_parentwidget
        self.n_stationid = n_stationid
        self.dict_sysparams = dict_config
        self.s_stationname = str(n_stationid)#存放global.xmg、check1.xml这些文件的文件夹名
        self.dict_alltabwidget = OrderedDict()#字典的键值为 界面号：类
        self.list_alltabwidget = []#每个方法的类名
        self.list_prename = []#每个tabwidget的名称
        self.list_paramfilename = []#参数文件名，后来保存用,一般是global0，check1，check2.....
        self.s_dirpath = dict_config[u'paramsdir']#参数路径
        self.s_positionofmodel = None#模板路径，相当于s_dirpath加上模板名
        self.bool_lock = False#界面是否加锁
        self.s_xmlpath = None# 存放global.xmg、check1.xml这些文件的文件夹路径名
        self.__initgraphic()

    
    def recmsg(self, n_stationid, n_msgtype, tuple_data):
        '''
        接收子站发送过来的消息
        '''   
        # if n_msgtype in imc_msg.list_params_module:
        #     self.dict_alltabwidget[n_uiid].recmsg(n_stationid, n_msgtype, tuple_data[1:])
        for n_classname in self.dict_alltabwidget:
            self.dict_alltabwidget[n_classname].recmsg(n_stationid, n_msgtype, tuple_data)

    def setlearnstatus(self, b_status=False):
        '''
        为了统一接口，所有界面都有这个函数
        ''' 
        for s_classname in self.dict_alltabwidget:
            self.dict_alltabwidget[s_classname].setlearnstatus(b_status)

    def __initgraphic(self):
        """
        这个类的作用是承接参数项未知py，也就是让这个类作为接口将py文件纳入参数设置的界面，
        所以便没必要专门为这个类开一个ui类，该函数即是代替ui类
        """
        self.h_verticallayout = QtWidgets.QVBoxLayout(self)
        # self.h_verticallayout.setSpacing(0)
        self.h_verticallayout.setContentsMargins(10, 10, 0, 0)
        self.h_tabwidget = QtWidgets.QTabWidget(self)
        self.h_verticallayout.addWidget(self.h_tabwidget)
        self.__inittabwidget()

    def __inittabwidget(self):
        """
        在参数项界面挂上ApplicationSetting指示的py（包括实例化类以及挂上界面）
        :return:
        """
        if self.imreadxml():
            pass
        else:
            QtWidgets.QMessageBox.warning(self, u"警告", u"配置中不存在参数界面相关的内容，请重新设置配置",
                                          QtWidgets.QMessageBox.Ok)
        for n_index, s_name in enumerate(self.list_alltabwidget):
            # -1 是因为global不算检查方法(区域数跟检查方法是一个意思)
            self.dict_alltabwidget[n_index] = KxBaseParamWidget.create(name=s_name, h_parent=self,
                                                                       n_stationid=self.n_stationid, n_uid=n_index,
                                                                       n_areanum = len(self.list_alltabwidget) - 1)
            self.h_tabwidget.addTab(self.dict_alltabwidget[n_index], self.list_prename[n_index])
        #目的显示check模板
        if len(self.dict_alltabwidget) > 1:
            self.h_tabwidget.setCurrentWidget(self.dict_alltabwidget[1])

    
    def imreadxml(self):
        """
        从xml里面读进所有的类名, 另外记得在开头将所有类对应的位置import进来，否则resig无法运行
        2020.02.18 第一份配置是global，第二份开始才是check + 1、2、3,并且，保存的xml名称也是这个名称（英文）。
        注意哈，global那份参数界面是特殊的，其余的都是检查参数，按照固定名称 + id
        """
        if (self.dict_sysparams is not None) and (len(self.list_alltabwidget) == 0):
            s_substation = "substation" + str(self.n_stationid + 1)
            substation_ui_parameter_terms = self.dict_sysparams[u'substation'][s_substation][u'ui_parameter_terms']
            self.list_alltabwidget = substation_ui_parameter_terms.keys()
            list_englishname = [ImportantName.XML_GLOBALCONFIGNAME]
            list_chinesename = [ImportantName.XML_GLOBALCONFIGNAME_CH]
            list_englishname.extend([ImportantName.XML_MODELCONFIGNAME + str(nindex + 1) for nindex in range(len(self.list_alltabwidget) -1)])
            list_chinesename.extend([ImportantName.XML_MODELCONFIGNAME_CH + str(nindex + 1) for nindex in range(len(self.list_alltabwidget)-1)])
            if int(self.dict_sysparams[u'istranslate']):
                self.list_prename = list_chinesename
            else:
                self.list_prename = list_englishname
            self.list_paramfilename  = list_englishname
            return True
        else:
            if len(self.list_alltabwidget) == 0:
                return False
            else:
                return True

    def updateparam(self, s_positions=None, bool_creatnew=True):
        """
        功能与模板管理的信号相挂，接收到信号便更新后面界面的数据，bool_creatnew 决定模板是新建，复制操作还是单纯的载入
        """
        if bool_creatnew:
            self.h_tabwidget.clear()
            for s_name in self.dict_alltabwidget:
                self.dict_alltabwidget[s_name] = None
            self.__inittabwidget()
        else:
            pass
        if s_positions != None:
            self.s_positionofmodel = s_positions
            self.s_xmlpath = self.s_positionofmodel+self.s_stationname+'\\'
            if not os.path.exists(self.s_xmlpath):
                os.makedirs(self.s_xmlpath)
        else:
            logging.warning("update have no position")
            return
        for n_classname in self.dict_alltabwidget:
            self.dict_alltabwidget[n_classname].setmodeldirectory(self.s_xmlpath +
                                                                  self.list_paramfilename[n_classname] + ".xml")
            self.dict_alltabwidget[n_classname].loadparameters()

        if bool_creatnew:
            self.checkload()


    def lock(self):
        for h_classname in self.dict_alltabwidget:
            self.dict_alltabwidget[h_classname].lock()
        self.bool_lock = True

    def unlock(self):
        for h_classname in self.dict_alltabwidget:
            self.dict_alltabwidget[h_classname].unlock()
        self.bool_lock = False

    def checkload(self):
        """ensure PARAMITEMPY was locked or unlock """
        if self.bool_lock:
            self.lock()

    def getpositionofmodel(self):
        """检测时需要获取当前的模板路径"""
        if self.s_xmlpath is not None:
            return self.s_xmlpath
        else:
            return None


    def str2fun(self, uid, strfun, *param):
        """
        通过输入函数名str来进行调用参数集界面的函数，实现ParmItem对下接口的开发
        Parameters
        ----------
        uid         界面号，确定是需要哪个界面的数据
        strfun      欲调用函数名称
        *param      调用函数时输入的参数，可以有多个

        Returns
        -------

        """
        if uid in self.dict_alltabwidget:
            classobj = self.dict_alltabwidget[uid]
            method_list = [func for func in dir(classobj) if callable(getattr(classobj, func))]
            if strfun in method_list:
                curfunctions = getattr(classobj, strfun)
                return curfunctions(*param)
            else:
                return None
        else:
            return None
