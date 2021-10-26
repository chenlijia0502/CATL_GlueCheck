# -*- coding: utf-8 -*-
import os
import sys
from PyQt5 import QtCore, QtGui,QtWidgets
from PyQt5.QtWidgets import QWidget, QDialog
from collections import OrderedDict
from pyqtgraph.parametertree import ParameterTree
from kxpyqtgraph.kxparameterTree.KxParameter import KxParameter
from UI.ui_editconfigwidget import Ui_editconfigwidget
import xmltodict


MAINSTATION_CONFIG = "ApplicationSetting.xml"  # 主站配置名
SUBSTATION_CONFIG = "Checkstation.xml"  # 子站配置名
"""
Date: 2019.11.04
author:	HYH
Description:	实现从xml文件中读取数据。具体操作为打开ReadConfigWidget，自动导入一份配置文件进行解析，刷新到参数树上；
				以上为一级管理，用户可默认配置。二级管理为按下‘alt’键，会弹出一个修改内容界面，修改完成点保存数据会刷
				新到一级管理界面上。
"""

def readYamlInfo(path):
	dict_param = None
	with open(path, 'r') as f:
		cont = f.read()
		dict_param = yaml.load(cont)
	return dict_param


def readXmlInfo(path):
	'''
    读取xml，以字典返回
    '''
	import os
	import xmltodict
	global logger
	try:
		sysparamsfilename = path
		dict_sysparams = None
		if os.path.isfile(sysparamsfilename):
			with open(sysparamsfilename, "r") as fd:
				obj = xmltodict.parse(fd.read())['root']
			dict_sysparams = obj
		return dict_sysparams
	except Exception:
		logger.error('', exc_info=True)
		return None

def readconfig(configpath=None):
	if configpath is None:
		configpath = MAINSTATION_CONFIG  # 主站配置
	list_word = configpath.split('.')
	if list_word[-1] == 'xml':
		return readXmlInfo(configpath)
	elif list_word[-1] == 'yaml':
		return readYamlInfo(configpath)

def createparam(dict_sysparams):
	simpledict = {'name':"" ,'type':""}
	result = []
	for key in dict_sysparams:
		obj = dict_sysparams[key]
		if not isinstance(obj, dict):
			# print key
			cursimpledict = simpledict.copy()
			cursimpledict['name'] = key
			cursimpledict['type'] = 'str'
			cursimpledict['value'] = obj
			result.append(cursimpledict)
		else:
			list_next = createparam(obj)
			cursimpledict = simpledict.copy()
			cursimpledict['name'] = key
			cursimpledict['type'] = 'group'
			cursimpledict['value'] = obj
			cursimpledict['children'] = list_next
			result.append(cursimpledict)
	return result

def getdictfromparamtree(dict_paramtree):
	#参数树得到的字典太过复杂，加一层解析,让它跟读进来的形式一致
	if 'children' not in dict_paramtree.keys():
		return None
	dict_result = OrderedDict()
	for key in dict_paramtree['children'].keys():
		if 'children' in  dict_paramtree['children'][key].keys():
			dict_result[key] = OrderedDict()
			dict_result[key] = getdictfromparamtree(dict_paramtree['children'][key])
		else:
			dict_result[key] = dict_paramtree['children'][key]['value']
	return dict_result


class ReadConfigDialog(QDialog):
	"""
	可读取多份配置，以参数树形式呈现，alt键触发另一窗口弹出
	"""
	def __init__(self, configpath):
		super(ReadConfigDialog, self).__init__()
		self.layout = QtWidgets.QVBoxLayout(self)
		self.tabwidget = QtWidgets.QTabWidget(self)
		self.layout.addWidget(self.tabwidget)
		self.resize(1000,600)
		self.setWindowTitle(u"参数设置")
		self.b_initstatus = True
		self.list_configpath = []
		if isinstance(configpath, str):
			self.list_configpath.append(configpath)
		elif isinstance(configpath, list):
			self.list_configpath = configpath
		self.list_tabwidget = []
		for nindex, path in enumerate(self.list_configpath):
			if not os.path.isfile(path):
				QtWidgets.QMessageBox.warning(self, u"警告", u"配置不存在："+path, QtWidgets.QMessageBox.Ok)
				self.b_initstatus = False
				return
			smallwidget = ReadConfigWidget(path)
			self.list_tabwidget.append(smallwidget)
			self.tabwidget.addTab(smallwidget, str(nindex + 1))


	def closeEvent(self, QCloseEvent):
		warnwindow = QtWidgets.QMessageBox()
		respond = warnwindow.warning(self, u"警告", u"是否保存数据？",
									 QtWidgets.QMessageBox.Ok | QtWidgets.QMessageBox.Cancel)
		if respond == QtWidgets.QMessageBox.Cancel:
			pass
		else:
			for tabwidget in self.list_tabwidget:
				tabwidget.save()


class ReadConfigWidget(QWidget):
	"""
	读取单份配置，以参数树形式呈现，alt键触发另一窗口弹出
	"""
	def __init__(self, config_path):
		super(ReadConfigWidget, self).__init__()
		self.s_configpath = config_path
		self.__initui()

	def __initui(self):
		self.layout = QtWidgets.QHBoxLayout(self)
		self.dict_sysparam = readconfig(self.s_configpath)
		self.dict_param = createparam(self.dict_sysparam)
		self.param = KxParameter.create(name='params', type='group', children=self.dict_param)
		self.paramtree = ParameterTree()
		self.paramtree.setParameters(self.param, showTop=False)
		self.layout.addWidget(self.paramtree)
		self.layout.setContentsMargins(2, 2, 2, 2)
		# self.resize(1000,600)
		# self.setWindowTitle(u"参数设置")


	def keyPressEvent(self, event):
		print ('here')
		if event.key() == QtCore.Qt.Key_Alt:# 按下Alt才会来到这里(之后要加上密码才能打开)
			widget = EditConfigWidget()
			widget.set_editword(self.s_configpath)
			widget.Signal_paramchange.connect(self.__resetparam)
			widget.Signal_close.connect(self.__reenable)
			self.setEnabled(False)
			widget.show()
			widget.exec_()

	def __reenable(self):#防止冲突
		self.setEnabled(True)

	def __resetparam(self):
		#收到修改界面的重设，则进行重设
		self.dict_sysparam = readconfig(self.s_configpath)
		self.dict_param = createparam(self.dict_sysparam)
		self.param = KxParameter.create(name='params', type='group', children=self.dict_param)
		self.paramtree.setParameters(self.param, showTop=False)

	def save(self):
		dict_state = self.param.saveState(filter='user')
		dict_newstate = getdictfromparamtree(dict_state)
		xmldict = {u'root': dict_newstate}
		xml_sz = xmltodict.unparse(xmldict, pretty=True)
		with open(self.s_configpath, 'w') as file_object:
			file_object.write(xml_sz)
			file_object.close()



class EditConfigWidget(QDialog):
	"""
	编辑配置类，比如说配置中需要新添一个站的内容时，就需要启动这个界面进行自定义设置
	"""
	Signal_paramchange = QtCore.pyqtSignal()
	Signal_close = QtCore.pyqtSignal()
	def __init__(self):
		super(EditConfigWidget, self).__init__()
		self.ui = Ui_editconfigwidget()
		self.ui.setupUi(self)
		self.__initconnect()
		self.ui.pushButton.setEnabled(False)

	def __initconnect(self):
		self.ui.pushButton.clicked.connect(self.__saveconfig)
		self.ui.pushButton_2.clicked.connect(self.close)
		self.ui.pushButton_reset.clicked.connect(self.__reset)
		self.ui.textEdit.textChanged.connect(self.__reEnable)


	def set_editword(self, config_path):
		self.s_configpath = config_path
		dict_result = readconfig(config_path)
		xmldict = {u'root': dict_result}
		self.config_sz = xmltodict.unparse(xmldict, pretty=True)
		self.ui.textEdit.setText(self.config_sz)



	def __reset(self):
		self.ui.textEdit.clear()
		self.ui.textEdit.setText(self.config_sz)

	def __reEnable(self):
		self.ui.pushButton.setEnabled(True)

	def __saveconfig(self):
		newword = self.ui.textEdit.toPlainText()
		with open(self.s_configpath, 'w') as file_object:
			file_object.write(newword)
			file_object.close()
		self.ui.pushButton.setEnabled(False)
		self.Signal_paramchange.emit()  # 触发通知上一层更新

	def keyPressEvent(self, QKeyEvent):
		if (QKeyEvent.key() == QtCore.Qt.Key_S and (QKeyEvent.modifiers() & QtCore.Qt.ControlModifier)):
			self.__saveconfig()

	def closeEvent(self, QCloseEvent):
		self.Signal_close.emit()

if __name__ == '__main__':
	file_name = 'Checkstation.xml'
	app = QtWidgets.QApplication([])
	obj = ReadConfigWidget(file_name)
	obj.show()
	app.exec_()

