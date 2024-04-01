import os
import xmltodict

import logging

def readXmlInfo(path):
	'''
    读取xml，以字典返回
    '''

	try:
		sysparamsfilename = path
		dict_sysparams = None
		if os.path.isfile(sysparamsfilename):
			with open(sysparamsfilename, "r") as fd:
				obj = xmltodict.parse(fd.read())['root']
			dict_sysparams = obj
		return dict_sysparams
	except Exception:
		logging.error('', exc_info=True)
		return None

def writeXmlInfo(dict_data, path):
	try:
		xmldict = {u'root': dict_data}
		xml_sz = xmltodict.unparse(xmldict, pretty=True, encoding='gbk')
		file_object = open(path, 'w', encoding='gbk')
		file_object.write(xml_sz)
		file_object.close()
	except Exception:
		logging.error("", exc_info = True)
		return None
	return True


dict_read_data = readXmlInfo("MesParam.xml")

data1 = readXmlInfo("save.xml")

print(dict_read_data == data1)


# dict_read_data['site']['chinesename'] = "测试"
#
