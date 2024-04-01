# -*-coding:utf-8-*-
from project.monitoring import * #这行import保证实时界面能够挂载
import time
import tkinter.messagebox #弹窗库
from PyQt5.QtGui import *
from library.common.usermanager.ui_adduser import Ui_adduser
import codecs
from PyQt5.QtCore import Qt
from kxpyqtgraph.kxItem.DoubleListParameterItem import *
import csv
import tkinter
import tkinter.messagebox #弹窗库
import operator
from library.common.usermanager.subuserwidget import subuserwidget
import datetime
from library.ipc.ipc_tool import kxlog
import logging

#管理用户界面
class Adduserdialog(QtWidgets.QDialog):
    """
    最多有八组权限，由UI决定，可根据需求修改UI，根据初始输入的list_slevel进行命名修改，
    对应权限结果保存为 1 / 0
    """
    _CSV_SAVE_HEAD = ['ID','permission','time']
    def __init__(self, list_slevel:[]):
        super(Adduserdialog, self).__init__()
        self.setWindowTitle("添加账号")
        self.ui = Ui_adduser()
        self.ui.setupUi(self)
        self.list_level = list_slevel
        self._initPermissionChoice(list_slevel)
        self.setWindowFlags(Qt.WindowCloseButtonHint)
        self.str_passwordpath = 'd:\\'
        ###### 设置界面控件
        self.ui.userlineEdit.setEchoMode(QtWidgets.QLineEdit.Password)
        self.userlist = self.getUserlist()
        self.ui.userlineEdit.setFocus()
        ###### 绑定按钮事件
        self.ui.pushButton_confirm.clicked.connect(self.adduser)
        self.ui.pushButton_cancel.clicked.connect(self.close)
        self.ui.pushButton_sub.clicked.connect(self.subuser)



    def _initPermissionChoice(self, list_slevel:[]):
        """
        初始化权限选择
        :return:
        """
        self.list_checkbox = [self.ui.checkBox_1, self.ui.checkBox_2,self.ui.checkBox_3,
                              self.ui.checkBox_4,self.ui.checkBox_5,self.ui.checkBox_6,
                              self.ui.checkBox_7,self.ui.checkBox_8, self.ui.checkBox_9, self.ui.checkBox_10]

        self.nlevelnum = min(len(list_slevel), len(self.list_checkbox))

        for nindex in range(self.nlevelnum):

            self.list_checkbox[nindex].setText(list_slevel[nindex])

        for nindex in range(self.nlevelnum, len(self.list_checkbox)):

            self.list_checkbox[nindex].hide()


    def keyPressEvent(self, a0: QtGui.QKeyEvent):
        pass#目的是避免回车输入直接确认账号，还没选权限


    #获得用户列表
    def getUserlist(self):
       userlist = []
       try:
           csvpath = self.str_passwordpath + '/userlist.csv'
           # import win32api, win32con
           # win32api.SetFileAttributes(csvpath, win32con.FILE_ATTRIBUTE_HIDDEN)
           csvreader = csv.reader(open(csvpath,'r'))
           for nindex, item in enumerate(csvreader):
               if nindex == 0:#去掉头
                   continue
               append_idem=self.decrypt(item[0])
               userlist.append([append_idem.decode('utf-8'),item[1],item[2]])
       except Exception as e:
           kxlog(self, logging.ERROR, '用户文件不存在,已初始化')
           s_all = ""
           for i in range(self.nlevelnum):
               s_all += "1"
           self.userlist = [['zs20210401', s_all, '2021-04-01 08:30:00']]
           self.saveUserlist()
           return self.userlist
       return userlist


    def now(self):
        return time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()))


    def str_to_time(self,date, load=0):
        if load == 1:
            date = date.replace("/", "-")
            date = date+':00'
        return datetime.datetime.strptime(date, "%Y-%m-%d %H:%M:%S")


    def saveUserlist(self):
        """
        保存用户列表
        :return:
        """
        csvpath = self.str_passwordpath + '/userlist.csv'
        with codecs.open(csvpath,'w','utf-8') as csvfile:
            #指定csv文件指定头部
            fieldnames = self._CSV_SAVE_HEAD
            writer = csv.DictWriter(csvfile,fieldnames=fieldnames)
            writer.writeheader()
            for i in range(0,len(self.userlist)):
                try:
                    append_idem = self.encrypt(self.userlist[i][0])
                    writer.writerow({self._CSV_SAVE_HEAD[0]:append_idem.decode('utf-8'),
                                     self._CSV_SAVE_HEAD[1]:self.userlist[i][1],
                                     self._CSV_SAVE_HEAD[2]:self.userlist[i][2]})
                except Exception as e:
                    kxlog(self, logging.ERROR, str(e))
            #print("save: ", self.userlist)
        # import win32api, win32con
        # win32api.SetFileAttributes(csvpath, win32con.FILE_ATTRIBUTE_HIDDEN)


    def _getcurlevel(self):
        s_all = ""
        for i in range(self.nlevelnum):
            if self.list_checkbox[i].isChecked():
                s_all += "1"
            else:
                s_all += "0"
        return s_all


    def adduser(self):
        idex = [self.ui.userlineEdit.text(), self._getcurlevel(), self.now()]
        #print ('add before: ', self.userlist)
        for i in range(len(self.userlist)):
            if self.userlist[i][0] == idex[0]:
                root = tkinter.Tk()
                root.withdraw()
                tkinter.messagebox.showinfo('提示', '该用户已存在！')
                return

        self.userlist.append(idex)
        print ('add after: ', self.userlist)

        self.saveUserlist()
        root = tkinter.Tk()
        root.withdraw()
        tkinter.messagebox.showinfo('提示', '添加新用户成功！')
        self.close()

    def encrypt(self,data):
        key = 'ZS_TECH'
        data = data.encode('utf-8')
        dataLen = len(data)
        keyLen = len(key)
        key = dataLen // keyLen * key + key[:dataLen % keyLen]
        result = bytearray()
        for i in range(len(key)):
            result.append(ord(key[i]) ^ data[i])
        return result

    def decrypt(self,data):
        key = 'ZS_TECH'
        data = data.encode('utf-8')
        dataLen = len(data)
        keyLen = len(key)
        key = dataLen // keyLen * key + key[:dataLen % keyLen]

        result = bytearray()
        for j in range(len(key)):
            result.append(data[j] ^ ord(key[j]))
        return result

    #删除用户权限
    def subuser(self):
        namelist = []
        self.userlist = self.getUserlist()
        self.subwidght = subuserwidget(self,self.userlist,self.list_level)
        # 必须加这句
        self.subwidght.setWindowModality(Qt.ApplicationModal)
        self.subwidght.exec_()



if __name__ == "__main__":
    app = QtWidgets.QApplication([])
    w = Adduserdialog()
    w.show()
    app.exec_()