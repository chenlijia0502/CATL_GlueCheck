# -*-coding:utf-8-*-
from PyQt5.QtWidgets import QTableWidget, QTableWidgetItem

from kxpyqtgraph.kxItem.DoubleListParameterItem import *
from PyQt5.QtCore import Qt
import tkinter
import tkinter.messagebox #弹窗库

from library.common.readconfig import readconfig, createparam
from library.common.usermanager.ui_submeswidget import Ui_submeswigget
import  logging

class subuserwidget(QtWidgets.QDialog):
    def __init__(self,parent,userlist, list_slevel):
        super(subuserwidget, self).__init__()
        self.logger = logging.getLogger('UI.%s' % self.__class__.__name__)
        self.ui = Ui_submeswigget()
        self.ui.setupUi(self)
        self.parent = parent
        self.userlist = userlist
        self.list_slevel = list_slevel
        self.updatetablewidget()
        self.ui.pbt_submespro.clicked.connect(self.click_sub)
        self.ui.pbt_cancel.clicked.connect(self.eventreturn)


    #更新列表
    def updatetablewidget(self):
        list_head = ['用户']
        list_head.extend(self.list_slevel)
        self.ui.tableWidget.setColumnCount(len(list_head))
        self.ui.tableWidget.setRowCount(len(self.userlist))
        self.ui.tableWidget.setHorizontalHeaderLabels(list_head)
        self.ui.tableWidget.horizontalHeader().setSectionResizeMode(QtWidgets.QHeaderView.Stretch)

        for nindex in range(len(self.userlist)):
            # if nindex == 0:
            #     newItem = QTableWidgetItem("************")#管理员账号保密
            # else:
            newItem = QTableWidgetItem(self.userlist[nindex][0])
            # if len(self.userlist[nindex][0]) > 3:
            #     newItem = QTableWidgetItem(self.userlist[nindex][0][:3] + "*********")
            # else:
            #     newItem = QTableWidgetItem(self.userlist[nindex][0])
            self.ui.tableWidget.setItem(nindex, 0, newItem)
            for col in range(len(self.list_slevel)):
                if col < len(self.userlist[nindex][2]):#考虑后面可能会出现新增锁定项目的可能，所以做保护
                    item = QTableWidgetItem(self.userlist[nindex][2][col])
                    self.ui.tableWidget.setItem(nindex, col + 1, item)

        self.ui.tableWidget.setEditTriggers(QtWidgets.QAbstractItemView.NoEditTriggers)

    def click_sub(self):
        try:
            row = self.ui.tableWidget.selectedItems()[0].row()
            del self.userlist[row]
            #self.parent.saveUserlist()
            root = tkinter.Tk()
            root.withdraw()
            tkinter.messagebox.showinfo('提示', '删除完成！')
            self.updatetablewidget()
        except:
            self.logger.log(logging.ERROR, "删除用户错误")


    def eventreturn(self):
        self.close()

    def getrefreshuserlist(self):
        return self.userlist