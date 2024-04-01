#coding=utf-8
'''
Created on 2015年10月26日

@author: huber yao
'''
import os
import xmltodict
def readsysparams():
        '''
        读取系统配置参数
        '''
        sysparamsfilename = 'ApplicationSetting.xml'
        dict_sysparams = None
        if os.path.isfile(sysparamsfilename):
            with open(sysparamsfilename, "rb") as fd:
                obj = xmltodict.parse(fd.read())['root']
            dict_sysparams = obj
        return dict_sysparams
    
 


    
