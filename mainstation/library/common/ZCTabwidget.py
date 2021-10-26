#coding:utf-8
from PyQt5 import QtGui,QtCore, QtWidgets

import pyqtgraph as pg
import sip
from UI.ui_tabwidget import Ui_ZCTabwidget


"""
    代替QtGui.QTabWidget,原因是它的背景设置不了，布局极其难弄
    
    2020.02.19 已经放弃，成功找到设置tabwidget stylesheet的办法
"""

class ZCTabwidget(QtWidgets.QWidget):
    currentChanged = QtCore.pyqtSignal()
    def __init__(self):
        super(QtWidgets.QWidget, self).__init__()
        self.ui = Ui_ZCTabwidget()
        self.ui.setupUi(self)
        self.setMinimumSize(QtCore.QSize(1700, 800))
        self.npbtwidth = 80
        self.npbtheight = 35
        self.list_ptb = []
        self.list_widget = []
        self.list_widgetname = []
        self.layout_show = QtWidgets.QHBoxLayout(self.ui.botwidget)
        self.layout_show.setSpacing(0)
        self.ncurrentindex = 0


    def addTab(self, widget, name):
        self.list_widget.append(widget)
        self.list_widgetname.append(name)
        self.list_ptb.append(self.__initonebutton(name))
        #可不用去在意lambda在这里是什么，只需知道这会让控件被触发时同时发送了一个参数（少用，调试麻烦）
        index = len(self.list_ptb) - 1
        self.list_ptb[-1].clicked.connect(lambda: self.__showspecficwidget(index))
        self.layout_show.addWidget(self.list_widget[-1])
        self.setCurrentWidget(self.list_widget[-1])

    def __initonebutton(self, name):#固定了控件大小，没有排版
        nxpos = len(self.list_ptb) * self.npbtwidth
        pushButton = QtWidgets.QPushButton(self.ui.topwidget)
        pushButton.setGeometry(QtCore.QRect(nxpos+1, 0, self.npbtwidth, 35))
        pushButton.setStyleSheet("background-color: rgba(255, 250, 205, 30%);")
        pushButton.setText(name)
        pushButton.setCheckable(True)
        pushButton.setChecked(True)
        return pushButton

    def clear(self):
        num = len(self.list_ptb)
        for n in range(num):
            sip.delete(self.list_ptb[n])
            self.layout_show.removeWidget(self.list_widget[n])
        self.list_widget = []
        self.list_widgetname = []
        self.list_ptb = []
        self.ncurrentindex = 0

    def __showspecficwidget(self, nindex):
        if nindex == self.ncurrentindex:
            self.list_ptb[nindex].setChecked(True)
            return
        else:
            for n in range(len(self.list_widget)):
                self.list_widget[n].hide()
                self.list_ptb[n].setChecked(False)
            self.list_ptb[nindex].setChecked(True)
            self.list_widget[nindex].show()
            self.ncurrentindex = nindex

    def setCurrentWidget(self, widget):
        if widget in self.list_widget:
            nindex = self.list_widget.index(widget)
            self.__showspecficwidget(nindex)

    def currentwidgetchange(self):
        self.currentChanged.emit()

    def count(self):
        return len(self.list_widget)



# if __name__ == '__main__':
#     app = QtGui.QApplication([])
#
#     w = Mais()
#     w.showMaximized()
#     app.exec_()