#coding=utf-8
'''
Created on 2015年8月5日

@author: ljj
'''
from PyQt5 import QtGui, QtCore, QtWidgets
from .ui_KxAddNewModelDialog import Ui_KxAddNewModelDialog


class KxAddNewModelDialog(QtWidgets.QDialog):
    '''
    classdocs
    '''
    def __init__(self, parent = None):
        '''
        Constructor
        '''
        super(KxAddNewModelDialog, self).__init__(parent)
        self.ui = Ui_KxAddNewModelDialog()
        self.ui.setupUi(self)
        self.setWindowFlags(QtCore.Qt.Window | QtCore.Qt.WindowTitleHint | QtCore.Qt.WindowCloseButtonHint)
        #self.setCreatorNameText(KxGlobalParams.getGlobalParams().m_UserManage.GetCurUsername())#2017
        
    def setModelNameText(self, text):
        self.ui.lineEdit.setText(text)

    #     def getStationAssignmentStatusList(self):
#         return  self.ui.stationAssignmentStatusWidget.StationAssignmentStatusList
    
#     def setStationAssignmentStatusList(self, StationAssignmentStatusList):  
#         self.ui.stationAssignmentStatusWidget.StationAssignmentStatusList = copy.copy(StationAssignmentStatusList)
#         self.ui.stationAssignmentStatusWidget.updateUI()
        
       
    def setCreatorNameText(self, text):
        self.ui.lineEdit1.setText(text)

    def setExInfoText(self, text):
        self.ui.lineEdit2.setText(text)

    
    def getModelNameText(self):
        return self.ui.lineEdit.text()
    
    def getCreatorNameText(self):
        return self.ui.lineEdit1.text()
    
    def getExInfoText(self):
        return self.ui.lineEdit2.text()
    
    def showEvent(self, event):
        if event.type() == QtGui.QShowEvent.Show:
            self.ui.lineEdit.setFocus()
            # self.ui.lineEdit.setSelection(0, self.ui.lineEdit.text().length())
        return QtWidgets.QDialog.showEvent(self, event)