from PyQt5 import QtCore, QtWidgets, QtGui
import xmltodict
import logging
import time
import pyqtgraph as pg
import numpy as np
from suds.client import Client, HttpAuthenticated
from library.common.globalparam import tabWidgetStyle
from library.common.readconfig import readXmlInfo
from kxpyqtgraph.kxparameterTree.KxParameter import KxParameter
from pyqtgraph.parametertree import ParameterTree
from kxpyqtgraph.kxparameterTree.KxCustomWidget import *
from library.ipc import  ipc_tool
from library.common.ExcelManager import CExcelManager


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

class MESTYPE:
    SHOUJIAN = 0
    JINZHAN = 1
    CHUZHAN = 2



class CMesParamTreeWidget(QtWidgets.QWidget):
    """
    配置mes的参数树， 其中必须包含两部分最重要的键值：
    BASE 以及 PARAM
    BASE 由：username、password、url组成，缺一不可
    PARAM 由各组需要上传的参数组成，可在xml中添加

    TYPE代表当前mes是哪个接口，分为首件（0）、进站（1）、出站（2），接口不同，上传参数也不同。

    file_name, sheet_name, head
    """
    def __init__(self, FILE_PATH, exceldir_path, sheet_name, head, TYPE=0, ):
        super(CMesParamTreeWidget, self).__init__()
        self.file_path = FILE_PATH
        self.dict_param = readXmlInfo(FILE_PATH)
        self.type = TYPE
        self._initui()
        self._initparam()
        self._initmes()
        self._setexcelinfo(exceldir_path, sheet_name, head)
        self.pushbutton_test.clicked.connect(self.click2senddata)

    def _setexcelinfo(self, exceldir_path, sheet_name, head):
        self.basedir = exceldir_path
        self.basesheet_name = sheet_name
        self.basehead = head
        self.curpath = time.strftime('%Y-%m-%d', time.localtime(time.time())) + ".xlsx"
        self.excel_log = CExcelManager(self.basedir + "\\" + self.curpath, self.basesheet_name, self.basehead)
        self.timer = QtCore.QTimer(self)
        self.timer.timeout.connect(self.__timeout2createnewexccel)
        self.timer.start(1000 * 60 * 5)


    def __timeout2createnewexccel(self):
        """
        定时器五分钟更新一次当前日期，如果日期更新则改变文件名
        """
        curpath = time.strftime('%Y-%m-%d', time.localtime(time.time())) + ".xlsx"
        if curpath != self.curpath:
            if not self.excel_log.b_iswriting:#只要不是正在写入
                self.excel_log = CExcelManager(self.basedir + "\\" + self.curpath, self.basesheet_name, self.basehead)


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
        if 'UPLOAD' in self.dict_param:
            params3 = self.__getparam(self.dict_param['UPLOAD'])
            params = [{'name': 'BASE', 'type': 'group', 'children': params1},
                      {'name': 'PARAM', 'type': 'group', 'children': params2},
                      {'name': 'UPLOAD', 'type': 'group', 'children': params3}]
        else:
            params = [{'name': 'BASE', 'type': 'group', 'children': params1},
                      {'name': 'PARAM', 'type': 'group', 'children': params2},]
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
            t = HttpAuthenticated(username=username, password=password, timeout=2)
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
            if 'UPLOAD' in self.dict_param:
                for key in self.dict_param['UPLOAD']:
                    value = self.p.param('UPLOAD', key).value()
                    self.dict_param['UPLOAD'][key]['value'] = value
            self._initmes()# 这是考虑mes如果发生改变，所以全部重新来一遍
            return writeXmlInfo(self.dict_param, self.file_path)
        except Exception as e:
            ipc_tool.kxlog("MES", logging.ERROR, " MES error " + str(e))
            return None

    def setsfc(self, sfc):
        if self.type == MESTYPE.CHUZHAN:
            self.p.param('PARAM', 'sfc').setValue(sfc)

    def click2senddata(self):
        self.senddata()


    def senddata(self, sfc=None, data=1):
        try:
            if self.type == MESTYPE.CHUZHAN:
                if sfc != None:
                    self.p.param('PARAM', 'sfc').setValue(sfc)
                # 旧版本
                # self.machineIntegrationParametricData = []
                # self.machineIntegrationParametricData.append(
                #     self.client.factory.create('tns:machineIntegrationParametricData'))
                # self.machineIntegrationParametricData[0].name = "JSGJJCJG"
                # self.machineIntegrationParametricData[0].value = data#
                # self.machineIntegrationParametricData[0].dataType = "NUMBER"

                # 新版本
                self.machineIntegrationParametricData = []

                name = self.p.param('UPLOAD', 'name').value()
                self.machineIntegrationParametricData.append({'name':name, 'value':data, 'dataType': "NUMBER"})

                dict_senddata = {}
                for key in self.dict_param['PARAM']:
                    value = self.p.param('PARAM', key).value()
                    dict_senddata[key] = value
                dict_senddata["parametricDataArray"] = self.machineIntegrationParametricData

                payloads = dict_senddata

                uploadtime = time.time()
                strtime1 = time.strftime('%Y-%m-%d %X', time.localtime(uploadtime))
                strtime1 = strtime1 + "." + str(uploadtime).split('.')[1][0:3]

                result = self.client.service.dataCollectForSfcEx(payloads)  # 出站api

                uploaddonetime = time.time()
                strtime2 = time.strftime('%Y-%m-%d %X', time.localtime(uploaddonetime))
                strtime2 = strtime2 + "." + str(uploaddonetime).split('.')[1][0:3]
                ndiff = str(int((uploaddonetime - uploadtime) * 1000)) + " ms"
                list_data = [self.p.param('PARAM', 'sfc').value(), strtime1, strtime2, ndiff, payloads, result[0], result[1], "自动"]
                self.excel_log.writeExcel([list_data])# 写入log
                if result[0] == 0:
                    ipc_tool.kxlog("MES", logging.INFO, "MES出站 上传成功！")
                else:
                    errorwindow = QtWidgets.QMessageBox()
                    respond = errorwindow.warning(self, "警告，数据上传失败", str(result[0]) + " " + result[1], QtWidgets.QMessageBox.Ok)
                    ipc_tool.kxlog("MES", logging.ERROR, "！！！MES出站 上传失败！！！" + result[1])

            elif self.type == MESTYPE.SHOUJIAN:

                # 版本 1
                # self.machineIntegrationParametricData = []
                # self.machineIntegrationParametricData.append(
                #     self.client.factory.create('tns:machineIntegrationParametricData'))
                # self.machineIntegrationParametricData[0].name = "JSGJJCJG"
                # self.machineIntegrationParametricData[0].value = data  #
                # self.machineIntegrationParametricData[0].dataType = "NUMBER"

                #版本2 待测试
                self.machineIntegrationParametricData = []
                self.machineIntegrationParametricData.append({'name':'JSGJJCJG', 'value':data, 'dataType': "NUMBER"})

                dict_senddata = {}
                for key in self.dict_param['PARAM']:
                    value = self.p.param('PARAM', key).value()
                    dict_senddata[key] = value
                dict_senddata["parametricDataArray"] = self.machineIntegrationParametricData

                uploadtime = time.time()
                strtime1 = time.strftime('%Y-%m-%d %X', time.localtime(uploadtime))
                strtime1 = strtime1 + "." + str(uploadtime).split('.')[1][0:3]

                result = self.client.service.dataCollectForResourceFAI(dict_senddata)  # 出站api

                uploaddonetime = time.time()
                strtime2 = time.strftime('%Y-%m-%d %X', time.localtime(uploaddonetime))
                strtime2 = strtime2 + "." + str(uploaddonetime).split('.')[1][0:3]
                ndiff = str(int((uploaddonetime - uploadtime) * 1000)) + " ms"
                list_data = [strtime1, strtime2, ndiff, dict_senddata, result[0], result[1]]
                self.excel_log.writeExcel([list_data])# 写入log

                if result[0] == 0:
                    ipc_tool.kxlog("MES", logging.INFO, "MES首件 上传成功！")
                else:
                    errorwindow = QtWidgets.QMessageBox()
                    respond = errorwindow.warning(self, "警告，数据上传失败", str(result[0]) + " " + result[1], QtWidgets.QMessageBox.Ok)
                    ipc_tool.kxlog("MES", logging.ERROR, "！！！MES首件 上传失败！！！" + result[1])

        except Exception as e:
            ipc_tool.kxlog("MES", logging.ERROR, " MES SEND ERROR " + str(e))

#

if __name__ == "__main__":
    a = QtWidgets.QApplication([])
    w = CMesParamTreeWidget("MesChuZhan.xml")
    w.show()
    a.exec_()
