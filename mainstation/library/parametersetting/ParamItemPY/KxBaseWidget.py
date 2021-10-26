# -*- coding: utf-8 -*-
import sys, os
sys.path.append("../")
import  xmltodict
from PyQt5 import QtCore, QtGui, QtWidgets
from library.common.globalfun import ischildof
import pyqtgraph as pg
from kxpyqtgraph.kxparameterTree.KxParameter import KxParameter

#这里两个变量虽为全局变量，但是请不要直接调用
KXPARAM_TYPES = {}
KXPARAM_NAMES = {}

def registerkxwidget(name, cls, override=False):
    global KXPARAM_TYPES
    if name in KXPARAM_TYPES and not override:
        raise Exception("Parameter type '%s' already exists (use override=True to replace)" % name)
    if not ischildof(cls, KxBaseParamWidget):
        raise Exception(name + u"非KxBaseMonitoringWidget的继承类")
    KXPARAM_TYPES[name] = cls
    KXPARAM_NAMES[cls] = name

pg.setConfigOptions(imageAxisOrder='row-major')#确定行为主，否则图像显示会列为主




class KxBaseParamWidget(QtWidgets.QWidget):
    def __init__(self, n_uid, n_areanum, n_stationid):
        """
        参数界面基类，继承类必须将n_id、n_areanum写到参数中去，不然会造成读取错误
        (我为了后面方便大家开发，其实把系统写的很死，反而限制住了一些东西。)
        Parameters
        ----------
        n_uid           界面号，作用是区分参数界面，相当于界面id
        n_areanum       区域数，也即一个检查站有几种检查方法，这个只在global界面有被子站读取，但为方便管理，每个界面都有
        n_stationid     站点号


        PS:
            如果界面挂载在科信的Framework中，那么流程是：
            （1）类被实例化，初始__init__
            （2）被外部调用 self.setmodeldirectory()，设置保存参数的路径
            （3）被

        """
        super(KxBaseParamWidget, self).__init__()
        self.str_filedirectory = "d:\\model.xml"
        self.n_uid = n_uid
        self.n_stationid = n_stationid
        self.n_areanum = n_areanum
        self.dict_state = None#字典用来存储上次载入的数据
        self.params = [{'name': u'全局设置', 'type': 'group','visible':False,  'children': [
                {'name': u'界面号', 'type': 'int', 'value': self.n_uid, 'readonly':True},
                {'name': u'站点号', 'type': 'int', 'value': self.n_stationid, 'step': 1, 'limits':(0, 20), 'readonly':True},
                {'name': u'区域数', 'type': 'int', 'value': self.n_areanum, 'step': 1, 'limits': (0, 20), 'readonly': True},
        ]}]
        self.p = None#要在基类实现它





    @staticmethod
    def create(**kwargs):
        s_classname = kwargs.get('name', None)
        if s_classname is None:
            return
        else:
            cls = KXPARAM_TYPES[s_classname]
        return cls(kwargs.get('h_parent', None), kwargs.get('n_uid', None),
                   kwargs.get('n_areanum', None),kwargs.get('n_stationid', None))


    def recmsg(self, n_stationid, n_msgtype, tuple_data):
        '''
        接收子站发送过来的消息
        '''
        pass

    def setmodeldirectory(self, strpath):
        """
        模板管理界面向这传递文件保存路径
        """
        self.str_filedirectory = strpath



    def saveparameters(self):
        """
                            保存参数，写xml参数文件
        """
        if isinstance(self.p, KxParameter):
            self.dict_state = self.p.saveState(filter='user')
            xmldict = {u'root':self.dict_state}
            xml_sz = xmltodict.unparse(xmldict,pretty = True, encoding='gbk')
            file_object = open(self.str_filedirectory, 'w', encoding='gbk')
            file_object.write(xml_sz)
            file_object.close()


    def loadparameters(self):
        """
            从当前模板路径中载入参数，并刷新界面
        """
        if isinstance(self.p, KxParameter):
            str_filename = self.str_filedirectory
            if os.path.exists(str_filename):
                with open(str_filename, "r", encoding='gbk') as fd:
                    obj = xmltodict.parse(fd.read())['root']
                self.dict_state = obj
                self.p.restoreState(obj, removeChildren=False)
            else:
                self.saveparameters()

    def lock(self):
        self.setEnabled(False)

    def unlock(self):
        """
        解锁界面，包括解锁参数树以及ROI,解锁参数树（调用基类widget的锁）
        """
        self.setEnabled(True)

    def DataPipeline(self, npipelinetype, list_data):
        """
        数据管道，其他类经常需要获取参数，直接走管道
        :param npipelinetype:   管道类型，传输哪种类型数据
        :param list_data: 传输的数据
        :return: 默认1
        """
        pass

    def cancelparameter(self):
        """
            取消参数修改，刷新界面，恢复到最后一次保存的参数状态
        """
        self.p.restoreState(self.dict_state)
