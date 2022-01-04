from library.common.readconfig import readXmlInfo
from PyQt5.QtCore import Qt
from suds.client import Client, HttpAuthenticated
from library.common.readconfig import readconfig
from kxpyqtgraph.kxItem.DoubleListParameterItem import *
import os
import csv
import time
from library.ipc.ipc_tool import getqueue_queue_kxlog
import logging


class MesParamTreeWidget(QtWidgets.QDialog):
    def __init__(self):
        super(MesParamTreeWidget, self).__init__()
        self.h_parameterTree = ParameterTree(self, False)
        self._initui()
        self.dict_param = readXmlInfo("MesParam.xml")
        self.setWindowTitle("MES")
        self.setWindowIcon(QtGui.QIcon("res//mes.jpg"))
        self._initparam()
        self._initmes()
        self.pushButton.clicked.connect(self.sendmes)
        self.ptbNotSend.clicked.connect(self.notSend)
        self.pushButton_chuzhan.clicked.connect(self.chuzhan)
        self.pushButton_jinzhan.clicked.connect(self.jinzhan)
        self.setWindowModality(Qt.ApplicationModal)
        self.setWindowFlag(QtCore.Qt.WindowCloseButtonHint)

    def _initmes(self):
        try:
            dictconfig = readconfig()
            #print (dictconfig)
            print(dictconfig["MES"]['username'],dictconfig["MES"]['password'], dictconfig["MES"]['url'])
            t = HttpAuthenticated(username=dictconfig["MES"]['username'], password=dictconfig["MES"]['password'])
            self.client = Client(dictconfig["MES"]['url'], transport=t)
            self.machineIntegrationParametricData = []
        except Exception as e:
            s_msg = " MES error "
            print (s_msg, e)
            #getqueue_queue_kxlog().put(('mes', logging.ERROR, s_msg))

    def _initui(self):
        self.setMinimumWidth(600)
        self.setMinimumHeight(800)
        self.verlayout = QtWidgets.QVBoxLayout(self)
        self.h_parameterTree.setStyleSheet("QTreeWidget::item{height:40px;}")
        self.verlayout.addWidget(self.h_parameterTree)
        self.widget_13 = QtWidgets.QWidget(self)
        self.widget_13.setObjectName("widget_13")
        self.horizontalLayout_13 = QtWidgets.QHBoxLayout(self.widget_13)
        self.horizontalLayout_13.setObjectName("horizontalLayout_13")
        self.pushButton = QtWidgets.QPushButton(self.widget_13)
        self.ptbNotSend = QtWidgets.QPushButton(self.widget_13)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.pushButton.sizePolicy().hasHeightForWidth())
        self.pushButton.setSizePolicy(sizePolicy)
        self.ptbNotSend.setSizePolicy(sizePolicy)
        font = QtGui.QFont()
        font.setFamily("Arial")
        font.setPointSize(14)
        font.setBold(False)
        font.setItalic(False)
        font.setWeight(9)
        self.pushButton.setFont(font)
        self.pushButton.setStyleSheet("font: 75 14pt \"Arial\";")
        self.pushButton.setObjectName("pushButton")
        self.pushButton.setText("确认上传")
        self.ptbNotSend.setFont(font)
        self.ptbNotSend.setStyleSheet("font: 75 14pt \"Arial\";")
        self.ptbNotSend.setObjectName("pushButton")
        self.ptbNotSend.setText("不上传")
        self.horizontalLayout_13.addStretch()
        self.horizontalLayout_13.addWidget(self.pushButton)
        self.horizontalLayout_13.addStretch()
        self.horizontalLayout_13.addWidget(self.ptbNotSend)
        self.horizontalLayout_13.addStretch()

        self.pushButton_jinzhan = QtWidgets.QPushButton()
        self.pushButton_jinzhan.setText("进站")
        self.pushButton_chuzhan = QtWidgets.QPushButton(self)
        self.pushButton_chuzhan.setText("出站")

        # 卷针数据添加
        # self.widget_juanzhen = QtWidgets.QWidget(self)
        # self.horizontalLayout = QtWidgets.QHBoxLayout(self.widget_juanzhen)
        # self.label = QtWidgets.QLabel(self.widget_juanzhen)
        # self.label.setStyleSheet("font: 14pt \"Arial Rounded MT Bold\";\n"
        #                          "border:1px solid rgb(0, 0, 0);")
        # self.label.setText("卷针号(JZBH)")
        # self.horizontalLayout.addWidget(self.label)
        # self.juanzhencombox = QtWidgets.QComboBox(self.widget_juanzhen)
        # self.juanzhencombox.addItem("1")
        # self.juanzhencombox.addItem("2")
        # sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding)
        # sizePolicy.setHorizontalStretch(0)
        # sizePolicy.setVerticalStretch(0)
        # sizePolicy.setHeightForWidth(self.juanzhencombox.sizePolicy().hasHeightForWidth())
        # self.juanzhencombox.setSizePolicy(sizePolicy)
        # self.juanzhencombox.setStyleSheet("font: 18pt \"Arial\";")
        # self.horizontalLayout.addWidget(self.juanzhencombox)
        # self.horizontalLayout.setStretch(0, 1)
        # self.horizontalLayout.setStretch(1, 1)
        # self.verlayout.addWidget(self.widget_juanzhen)
        #self.verlayout.addWidget(self.widget_13)
        #self.verlayout.addWidget(self.pushButton)
        #self.verlayout.addWidget(self.pushButton_jinzhan)
        self.verlayout.addWidget(self.pushButton_chuzhan)
        self.verlayout.setStretch(0, 16)

        self.verlayout.setStretch(1, 1)
        #self.verlayout.setStretch(2, 1)

    def _initparam(self):
        params = []
        for key in self.dict_param:
            name = self.dict_param[key]['chinesename'] + '(' + key + ")"
            ischangeable = self.getbool(self.dict_param[key]["readonly"])
            isvisible = self.getbool(self.dict_param[key]["isvisible"])
            valuedata = self.dict_param[key]['value']
            if isinstance(self.dict_param[key]['value'], str) or valuedata is None:
                stype = "str"
                value = self.dict_param[key]['value']
                dict_single = {'name': name, 'type': stype, 'visible': isvisible, 'readonly': ischangeable,
                               'value': value}
            else:
                stype = "list"
                value = {}
                for v in valuedata:
                    value[v] = valuedata[v]
                dict_single = {'name': name, 'type': stype, 'visible': isvisible, 'readonly': ischangeable,
                               'values': value}
            params.append(dict_single)
        self.p = KxParameter.create(name='params', type='group', children=params)
        self.h_parameterTree.setParameters(self.p, showTop=False)



    def getbool(self, svalue):
        if svalue.lower() == "false" or svalue == "0":
            return False
        else:
            return True


    def setdata(self, list_tuple_data):
        try:
            self.measuredata = list_tuple_data
            datatype = "NUMBER"
            self.machineIntegrationParametricData = []
            self.machineIntegrationParametricData.append(
                self.client.factory.create('tns:machineIntegrationParametricData'))
            self.machineIntegrationParametricData[0].name = "JZBH"
            self.machineIntegrationParametricData[0].value = self.juanzhencombox.currentText()
            self.machineIntegrationParametricData[0].dataType = datatype
            for i in range(len(list_tuple_data)):
                self.machineIntegrationParametricData.append(
                    self.client.factory.create('tns:machineIntegrationParametricData'))
                self.machineIntegrationParametricData[i + 1].name = list_tuple_data[i][0]
                self.machineIntegrationParametricData[i + 1].value = list_tuple_data[i][1]
                self.machineIntegrationParametricData[i + 1].dataType = datatype

        except:
            s_msg = " MES ERROR "
            getqueue_queue_kxlog().put(('mes', logging.ERROR, s_msg))

    def save_measure_result(self):
        try:
            name = []
            result = []
            dayname = time.strftime('%Y-%m-%d', time.localtime())
            timename = time.strftime('%H-%M-%S', time.localtime())
            name.append('时间')
            result.append(timename)
            sn_name = 'sn'
            sn_result = self.p.param("当前电芯号(sfc)").value()
            name.append(sn_name)
            result.append(sn_result)
            for item in self.measuredata:
                name.append(item[0])
                result.append(item[1])
            file_path = 'D:\\measureresult'
            file_path = file_path.strip()
            file_path = file_path.rstrip("\\")
            isExists = os.path.exists(file_path)
            if not isExists:
                os.makedirs(file_path)
            csv_file_name = file_path + '\\' + dayname + '.csv'
            with open(csv_file_name, 'a+', newline='') as csvfile:
                writer = csv.writer(csvfile)
                writer.writerow(name)
                writer.writerow(result)
                writer.writerow([''])
        except Exception as e:
            s_msg = " writer csv error "
            getqueue_queue_kxlog().put(('mes', logging.ERROR, s_msg))

    def notSend(self):
        self.save_measure_result()
        self.close()





            # getqueue_queue_kxlog().put(('mes', logging.ERROR, s_msg))



    def jinzhan(self):
        print("进站命令")
        try:
            # TODO 下面几句话传输结果，将结果按照格式放入self.machineIntegrationParametricData
            self.machineIntegrationParametricData = []
            # self.machineIntegrationParametricData.append(
            #     self.client.factory.create('tns:machineIntegrationParametricData'))
            # self.machineIntegrationParametricData[0].name = "JZBH"
            # self.machineIntegrationParametricData[0].value = self.juanzhencombox.currentText()
            # self.machineIntegrationParametricData[0].dataType = datatype
            #
            dict_senddata = {}
            print(self.client.service)
            for key in self.dict_param:
                name = self.dict_param[key]['chinesename'] + '(' + key + ")"
                value = self.p.param(name).value()
                dict_senddata[key] = value
            dict_senddata["parametricDataArray"] = self.machineIntegrationParametricData
            payloads = {"resourceRequest": [dict_senddata]}
            print(payloads)
            result = self.client.service.dataCollectForSfcEX(**payloads)  # 出站api
            print('上传回复结果：', result)
            if result[0] == 0:
                warnwindows = QtWidgets.QMessageBox()
                respond = warnwindows.information(self, "上传成功", "数据已成功上传", QtWidgets.QMessageBox.Ok)
                self.close()
            else:
                errorwindow = QtWidgets.QMessageBox()
                respond = errorwindow.warning(self, "警告，数据上传失败", result[1], QtWidgets.QMessageBox.Ok)
        except:
            s_msg = " MES SEND ERROR "
            print(s_msg)


#----------------------------------------------------------------------------------------------------------------------#



    def sendmes(self):
        try:
            # self.machineIntegrationParametricData[0].name = "JZBH"
            # self.machineIntegrationParametricData[0].value = self.juanzhencombox.currentText()
            # self.machineIntegrationParametricData[0].dataType = "NUMBER"
            dict_senddata = {}
            for key in self.dict_param:
                name = self.dict_param[key]['chinesename'] + '(' + key + ")"
                value = self.p.param(name).value()
                dict_senddata[key] = value
            dict_senddata["parametricDataArray"] = self.machineIntegrationParametricData
            payloads = {"resourceRequest": [dict_senddata]}
            result = self.client.service.dataCollectForResourceFAI(**payloads)
            print ('上传回复结果：', result)
            if result[0] == 0:
                warnwindows = QtWidgets.QMessageBox()
                respond = warnwindows.information(self, "上传成功", "数据已成功上传", QtWidgets.QMessageBox.Ok)
                self.close()
            else:
                errorwindow = QtWidgets.QMessageBox()
                respond = errorwindow.warning(self, "警告，数据上传失败", result[1], QtWidgets.QMessageBox.Ok)
        except Exception as e:
            s_msg = " MES SEND ERROR "
            print(s_msg, e)
            #getqueue_queue_kxlog().put(('mes', logging.ERROR, s_msg))




    def chuzhan(self):
        #self.sendmes()
        print('出站命令')
        try:
            # TODO 下面几句话传输结果，将结果按照格式放入self.machineIntegrationParametricData
            self.machineIntegrationParametricData = []
            self.machineIntegrationParametricData.append(
                self.client.factory.create('tns:machineIntegrationParametricData'))
            self.machineIntegrationParametricData[0].name = "JSGJJCJG"
            self.machineIntegrationParametricData[0].value = 1# 先上传结果是OK的情况
            self.machineIntegrationParametricData[0].dataType = "NUMBER"

            dict_senddata = {}
            for key in self.dict_param:
                name = self.dict_param[key]['chinesename'] + '(' + key + ")"
                value = self.p.param(name).value()
                dict_senddata[key] = value
            dict_senddata["parametricDataArray"] = self.machineIntegrationParametricData

            # TODO 需不需要"resourceRequest"修饰待定
            payloads = dict_senddata

            print(payloads)

            result = self.client.service.dataCollectForSfcEx(payloads)  # 出站api

            print('上传回复结果：', result)
            if result[0] == 0:
                warnwindows = QtWidgets.QMessageBox()
                respond = warnwindows.information(self, "上传成功", "数据已成功上传", QtWidgets.QMessageBox.Ok)
                self.close()
            else:
                errorwindow = QtWidgets.QMessageBox()
                respond = errorwindow.warning(self, "警告，数据上传失败", result[1], QtWidgets.QMessageBox.Ok)
        except Exception as e:
            s_msg = " MES SEND ERROR "
            print(s_msg, e)



if __name__ == "__main__":
    app = QtWidgets.QApplication([])
    w = MesParamTreeWidget()
    w.show()
    app.exec_()
