#coding:utf-8

from pyqtgraph import ViewBox
from PyQt5 import QtGui, QtWidgets, QtCore

class kxViewBox(ViewBox):
    sigClicked = QtCore.pyqtSignal()
    def __init__(self,parent=None, border=None, lockAspect=False, enableMouse=True, invertY=False, enableMenu=True, name=None, invertX=False):
        super(kxViewBox, self).__init__(parent, border, lockAspect, enableMouse, invertY, enableMenu, name, invertX)


    def mouseClickEvent(self, ev):
        ViewBox.mouseClickEvent(self, ev)
        self.sigClicked.emit()
