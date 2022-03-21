from PyQt5 import QtCore, QtWidgets, QtGui
import pyqtgraph as pg
import numpy as np
from library.common.globalparam import tabWidgetStyle
from library.common.readconfig import readXmlInfo
from kxpyqtgraph.kxparameterTree.KxParameter import KxParameter
from pyqtgraph.parametertree import ParameterTree
from kxpyqtgraph.kxparameterTree.KxCustomWidget import *
from project.mes.MesParamTreeWidget import CMesParamTreeWidget
from project.other.globalparam import SYSPATH


class CMesParamWidget(QtWidgets.QWidget):
    FILE_SHOUJIAN = "MesShouJian.xml"#首件mes配置
    FILE_CHUZHAN = "MesChuZhan.xml"#出站mes配置
    FILE_JINZHAN = "MesJinZhan.xml"#进站mes配置
    def __init__(self):
        super(CMesParamWidget, self).__init__()
        self._initui()
        self.pushbutton_save.clicked.connect(self.saveparam)


    def _initui(self):
        self.tabwidget = QtWidgets.QTabWidget(self)
        self.verlayout = QtWidgets.QVBoxLayout(self)
        self.verlayout.addWidget(self.tabwidget)

        self.widget1 = CMesParamTreeWidget(self.FILE_CHUZHAN, TYPE=2,
                                           exceldir_path=SYSPATH.PATH_MESCHUZHAN_EXCEL,
                                           sheet_name="Sheet", head= ['条码', '开始时间', '结束时间',
                                                                       '耗时', '传参', 'Code', 'message', '出站模式'])
        self.widget2 = QtWidgets.QWidget()
        self.widget3 = CMesParamTreeWidget(self.FILE_SHOUJIAN, TYPE=0,
                                           exceldir_path=SYSPATH.PATH_MESSHOUJIAN_EXCEL,
                                           sheet_name="Sheet", head= ['开始时间', '结束时间',
                                                                       '耗时', '传参', 'Code', 'message'])

        self.tabwidget.addTab(self.widget1, "出站")
        self.tabwidget.addTab(self.widget2, "进站")
        self.tabwidget.addTab(self.widget3, "首件")

        self.tabwidget.setStyleSheet(tabWidgetStyle)

        self.pushbutton_save = QtWidgets.QPushButton(self)
        self.pushbutton_save.setText("保存")
        self.pushbutton_save.setMinimumHeight(60)
        self.verlayout.addWidget(self.pushbutton_save)

    def saveparam(self):
        """
        保存多个tab 的参数
        """
        if self.widget1.saveparam():
            QtWidgets.QMessageBox.information(self, "保存成功", u'保存成功',
                                          QtWidgets.QMessageBox.Ok)
        else:
            QtWidgets.QMessageBox.warning(self, self.tr('Warning'), u'保存失败',
                                          QtWidgets.QMessageBox.Ok)

    def senddata(self, sfc, data):
        self.widget1.senddata(sfc, data)

    def setchuzhansfc(self, sfc):
        self.widget1.setsfc(sfc)


if __name__ == "__main__":
    a = QtWidgets.QApplication([])
    w = CMesParamWidget()
    w.show()
    a.exec_()

