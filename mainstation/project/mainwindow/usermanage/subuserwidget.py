# -*-coding:utf-8-*-
from PyQt5.QtWidgets import QTableWidget, QTableWidgetItem

from kxpyqtgraph.kxItem.DoubleListParameterItem import *
from PyQt5.QtCore import Qt
import tkinter
import tkinter.messagebox #弹窗库

from library.common.readconfig import readconfig, createparam
from ui_submeswidget import Ui_submeswigget

class subuserwidget(QtWidgets.QDialog):
    def __init__(self,parent,userlist):
        super(subuserwidget, self).__init__()
        self.ui = Ui_submeswigget()
        self.ui.setupUi(self)
        self.parent = parent
        self.userlist = userlist
        self.updatetablewidget()
        self.ui.pbt_submespro.clicked.connect(self.click_sub)
        self.ui.pbt_cancel.clicked.connect(self.eventreturn)
    #更新列表
    def updatetablewidget(self):
        dict = {'0':'操作员','1':'工程师','2':'管理员'}
        self.ui.tableWidget.setColumnCount(2)
        self.ui.tableWidget.setRowCount(len(self.userlist))
        #self.ui.tableWidget = QTableWidget(len(self.userlist), 2)
        self.ui.tableWidget.setHorizontalHeaderLabels(['用户', '权限'])
        for item in range(len(self.userlist)):
            newItem = QTableWidgetItem(self.userlist[item][0])
            self.ui.tableWidget.setItem(item, 0, newItem)
            newItem = QTableWidgetItem(dict[self.userlist[item][1]])
            self.ui.tableWidget.setItem(item, 1, newItem)
            self.ui.tableWidget.setColumnWidth(item,200)
        self.ui.tableWidget.setEditTriggers(QtWidgets.QAbstractItemView.NoEditTriggers)

    def click_sub(self):
        try:
            row = self.ui.tableWidget.selectedItems()[0].row()
            if self.userlist[row][1] == '2':
                root = tkinter.Tk()
                root.withdraw()
                tkinter.messagebox.showinfo('提示', '该用户为管理员,不可删除！')
                return
            else:
                del self.userlist[row]
            self.parent.saveUserlist()
            root = tkinter.Tk()
            root.withdraw()
            tkinter.messagebox.showinfo('提示', '删除完成！')
            self.updatetablewidget()
        except:
            pass
    def eventreturn(self):
        self.close()