# -*-coding:utf-8-*-
from project.monitoring import * #这行import保证实时界面能够挂载
import time
import tkinter.messagebox #弹窗库
from PyQt5.QtGui import *
from ui_adduser import Ui_adduser
import codecs
from PyQt5.QtCore import Qt
from kxpyqtgraph.kxItem.DoubleListParameterItem import *
import csv
import logging
import tkinter
import tkinter.messagebox #弹窗库
import operator
from subuserwidget import subuserwidget
import datetime

#管理用户界面
class adduserdialog(QtWidgets.QDialog):
    def __init__(self, mode=0, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.logger = logging.getLogger('UI.%s' % self.__class__.__name__)
        self.window = 0
        self.count = 0
        self.ui = Ui_adduser()
        self.ui.setupUi(self)
        self.mode = mode
        self.newfile = 0
        self.setWindowFlags(Qt.WindowCloseButtonHint)
        self.window = 0
        self.str_passwordpath = 'd:\\'
        ###### 设置界面控件
        self.ui.userlineEdit.setEchoMode(QtWidgets.QLineEdit.Password)
        self.userlist = self.getUserlist()
        ###### 绑定按钮事件
        self.ui.pushButton.clicked.connect(self.adduser)
        self.ui.pushButton_2.clicked.connect(self.close)
        self.ui.pushButton_sub.clicked.connect(self.subuser)

    #获得用户列表
    def getUserlist(self):
       userlist = []
       try:
           csvreader = csv.reader(open(self.str_passwordpath + '/userlist.csv','r'))
           if csvreader.line_num == 1:
               pass
           else:
               for item in csvreader:
                   append_idem=self.decrypt(item[0])
                   userlist.append([append_idem.decode('utf-8'),item[1],item[2]])
       except Exception as e:
           self.logger.info('用户文件不存在,已初始化')
           self.newfile = 1
           self.userlist = [['root',2,'2021-11-29 20:13:49']]
           self.saveUserlist()
       return userlist

    def now(self):
        return time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()))

    def str_to_time(self,date,load=0):
        if load == 1:
            date = date.replace("/", "-")
            date = date+':00'
        return datetime.datetime.strptime(date, "%Y-%m-%d %H:%M:%S")

    #保存用户列表
    def saveUserlist(self):
        with codecs.open(self.str_passwordpath + '/userlist.csv','w','utf-8') as csvfile:
            #指定csv文件指定头部
            fieldnames = ['ID','permission','time']
            writer = csv.DictWriter(csvfile,fieldnames=fieldnames)
            writer.writeheader()
            for i in range(0,len(self.userlist)):
                try:
                    append_idem = self.encrypt(self.userlist[i][0])
                    writer.writerow({'ID':append_idem.decode('utf-8'),'permission':self.userlist[i][1],'time':self.userlist[i][2]})
                except Exception as e:
                    self.logger.error('', exc_info=True)

    def adduser(self):
        if self.newfile == 1 or operator.eq(self.userlist, [['root','2','2021-11-29 20:13:49']]):
            self.newfile = 0
            self.userlist = []
        idex = [self.ui.userlineEdit.text(),self.ui.comboBox.currentIndex(),self.now()]
        for i in range(len(self.userlist)):
            if self.userlist[i][0] == idex[0]:
                root = tkinter.Tk()
                root.withdraw()
                tkinter.messagebox.showinfo('提示', '该用户已存在！')
                return
        self.userlist.append(idex)
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
        self.subwidght = subuserwidget(self,self.userlist)
        # 必须加这句
        self.subwidght.setWindowModality(Qt.ApplicationModal)
        self.subwidght.exec_()


if __name__ == "__main__":
    app = QtWidgets.QApplication([])
    w = adduserdialog()
    w.show()
    app.exec_()