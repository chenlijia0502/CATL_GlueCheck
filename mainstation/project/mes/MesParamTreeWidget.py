from PyQt5 import QtCore, QtWidgets, QtGui
import xmltodict
import logging
import pyqtgraph as pg
import numpy as np
from suds.client import Client, HttpAuthenticated
from library.common.globalparam import tabWidgetStyle
from library.common.readconfig import readXmlInfo
from kxpyqtgraph.kxparameterTree.KxParameter import KxParameter
from pyqtgraph.parametertree import ParameterTree
from kxpyqtgraph.kxparameterTree.KxCustomWidget import *
from library.ipc import  ipc_tool


def writeXmlInfo(dict_data, path):
	try:
		xmldict = {u'root': dict_data}
		xml_sz = xmltodict.unparse(xmldict, pretty=True, encoding='gbk')
		file_object = open(path, 'w', encoding='gbk')
		file_object.write(xml_sz)
		file_object.close()
	except Exception as e:
		ipc_tool.kxlog("MES", logging.ERROR, "MES writexml ERROR" + str(e))
		return None
	return True



class CMesParamTreeWidget(QtWidgets.QWidget):
    """
    配置mes的参数树， 其中必须包含两部分最重要的键值：
    BASE 以及 PARAM
    BASE 由：username、password、url组成，缺一不可
    PARAM 由各组需要上传的参数组成，可在xml中添加

    TYPE代表当前mes是哪个接口，分为首件、进站、出站，接口不同，上传参数也不同
    """
    def __init__(self, FILE_PATH, TYPE=0):
        super(CMesParamTreeWidget, self).__init__()
        self.file_path = FILE_PATH
        self.dict_param = readXmlInfo(FILE_PATH)
        self._initui()
        self._initparam()
        self._initmes()
        self.pushbutton_test.clicked.connect(self.click2senddata)


    def _initui(self):
        self.h_parameterTree = ParameterTree(self, False)
        self.h_parameterTree.setStyleSheet("QTreeWidget::item{height:40px;}")
        self.verlayout1 = QtWidgets.QVBoxLayout(self)
        self.verlayout1.addWidget(self.h_parameterTree)
        self.pushbutton_test = QtWidgets.QPushButton(self)
        self.pushbutton_test.setText("上传测试")
        self.verlayout1.addWidget(self.pushbutton_test)


    def _initparam(self):
        #print (self.dict_param.keys(),)
        if self.dict_param == None or 'BASE' not in self.dict_param.keys() or 'PARAM' not in self.dict_param.keys():
            QtWidgets.QMessageBox.warning(self, self.tr('Warning'), u'MES xml 参数缺失，请查看%s 配置是否正确'%self.file_path,
                                          QtWidgets.QMessageBox.Ok)
            return

        params1 = self.__getparam(self.dict_param['BASE'])
        params2 = self.__getparam(self.dict_param['PARAM'])
        params = [{'name': 'BASE', 'type': 'group', 'children': params1},
                  {'name': 'PARAM', 'type': 'group', 'children': params2}]
        self.p = KxParameter.create(name='params', type='group', children=params)
        self.h_parameterTree.setParameters(self.p, showTop=False)


    def getbool(self, svalue):
        if svalue.lower() == "false" or svalue == "0":
            return False
        else:
            return True


    def __getparam(self, dict_param):
        params = []
        for key in dict_param:
            name = key
            # ischangeable = self.getbool(dict_param[key]["readonly"])
            # isvisible = self.getbool(dict_param[key]["isvisible"])
            readonly = 0
            isvisible = 1
            if 'values' not in dict_param[key]:
                stype = "str"
                value = dict_param[key]['value']
                dict_single = {'name': name, 'type': stype, 'visible': isvisible, 'readonly': readonly,
                               'value': value}
            else:
                valuedata = dict_param[key]['values']
                stype = "list"
                value = dict_param[key]['value']
                dict_single = {'name': name, 'type': stype, 'visible': isvisible, 'readonly': readonly,
                               'values': valuedata, 'value':value}
            params.append(dict_single)
        return params

    def _initmes(self):
        try:
            username = self.p.param("BASE", "username").value()
            password = self.p.param("BASE", "password").value()
            url = self.p.param("BASE", "url").value()
            #print(username, password, url)
            t = HttpAuthenticated(username=username, password=password)
            self.client = Client(url, transport=t)
            self.machineIntegrationParametricData = []
        except Exception as e:
            ipc_tool.kxlog("MES", logging.ERROR, " MES error " + str(e))


    def saveparam(self):
        try:
            for key in self.dict_param['BASE']:
                value = self.p.param('BASE', key).value()
                self.dict_param['BASE'][key]['value'] = value
            for key in self.dict_param['PARAM']:
                value = self.p.param('PARAM', key).value()
                self.dict_param['PARAM'][key]['value'] = value
            self._initmes()# 这是考虑mes如果发生改变，所以全部重新来一遍
            return writeXmlInfo(self.dict_param, self.file_path)
        except Exception as e:
            ipc_tool.kxlog("MES", logging.ERROR, " MES error " + str(e))
            return None

    def click2senddata(self):
        self.senddata()

    def senddata(self, sfc=None, data=None):
        try:
            if sfc != None:

                self.p.param('PARAM', 'sfc').setValue(sfc)
            if data == None:
                data = 1
            # TODO 下面几句话传输结果，将结果按照格式放入self.machineIntegrationParametricData
            self.machineIntegrationParametricData = []
            self.machineIntegrationParametricData.append(
                self.client.factory.create('tns:machineIntegrationParametricData'))
            self.machineIntegrationParametricData[0].name = "JSGJJCJG"
            self.machineIntegrationParametricData[0].value = data#
            self.machineIntegrationParametricData[0].dataType = "NUMBER"

            dict_senddata = {}
            for key in self.dict_param['PARAM']:
                value = self.p.param('PARAM', key).value()
                dict_senddata[key] = value
            dict_senddata["parametricDataArray"] = self.machineIntegrationParametricData

            payloads = dict_senddata
            print(payloads)
            result = self.client.service.dataCollectForSfcEx(payloads)  # 出站api

            print('上传回复结果：', result)
            if result[0] == 0:
                # warnwindows = QtWidgets.QMessageBox()
                # respond = warnwindows.information(self, "上传成功", "数据已成功上传", QtWidgets.QMessageBox.Ok)
                ipc_tool.kxlog("MES", logging.INFO, "MES 上传成功！")
            else:
                #self.SIG_CHUZHAN.emit()
                errorwindow = QtWidgets.QMessageBox()
                respond = errorwindow.warning(self, "警告，数据上传失败", result[1], QtWidgets.QMessageBox.Ok)
                ipc_tool.kxlog("MES", logging.ERROR, "！！！MES 上传失败！！！")

        except Exception as e:
            ipc_tool.kxlog("MES", logging.ERROR, " MES SEND ERROR " + str(e))



if __name__ == "__main__":
    a = QtWidgets.QApplication([])
    w = CMesParamTreeWidget("MesChuZhan.xml")
    w.show()
    a.exec_()
