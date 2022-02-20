import os
import openpyxl
import logging
import time
import tkinter
import tkinter.messagebox #弹窗库
import copy
from PyQt5 import QtCore, QtWidgets, QtGui
from library.common.usermanager.ui_addusernew import Ui_addusernew
from library.common.usermanager.subuserwidget import subuserwidget
from library.common.ExcelManager import CExcelManager




class CAddUserDialogNew(QtWidgets.QDialog):
    STR_PASSWORDSAVE_PATH = "d:\\software\\system\\userlist.xlsx"
    LEVEL_SHEET_NAME = "Sheet1"# 保存用户权限列表的sheet名
    MODULE_SHEET_NAME = "Sheet2" #保存等级列表的sheet名
    _CSV_SAVE_HEAD = ['name', 'ID','permission','time']

    def __init__(self, list_modulename, list_levelname=['一级权限', '二级权限','三级权限', '四级权限','五级权限', '六级权限']):
        super(CAddUserDialogNew, self).__init__()
        self.ui = Ui_addusernew()
        self.ui.setupUi(self)
        self.list_level = list_modulename
        self.__initui(list_levelname)
        self.excel_ssh = CExcelManager(self.STR_PASSWORDSAVE_PATH, 'Sheet', self._CSV_SAVE_HEAD)
        self.h_levelmanager = CmanageLevelTool(list_modulename, list_levelname)
        self._initinfo()
        #self.setWindowFlags(QtCore.Qt.WindowStaysOnTopHint)#全屏

        ###### 绑定按钮事件
        self.ui.pushButton_confirm.clicked.connect(self.adduser)
        self.ui.pushButton_cancel.clicked.connect(self.close)
        self.ui.pushButton_sub.clicked.connect(self._subuser)
        self.ui.pushButton_level.clicked.connect(self._showmanageleveltool)


    def __initui(self, list_levelname):
        self.ui.comboBox.setMaximumHeight(30)  ## set to match height of spin box and line edit
        self.ui.comboBox.setStyleSheet("font: 13pt \"Arial\";")
        for name in list_levelname:
            self.ui.comboBox.addItem(name)


    def _initinfo(self):
        list_list_status = self.excel_ssh.read_data_line_for_sheet(self.MODULE_SHEET_NAME)
        self.h_levelmanager.setinfo(list_list_status)
        self.userlist = self._loaduserlist()


    def _showmanageleveltool(self):
        self.h_levelmanager.show()
        self.h_levelmanager.exec_()

        list_list_status = self.h_levelmanager.getinfo2save()

        self.excel_ssh.writeExcelinOverwrite(list_list_status, self.MODULE_SHEET_NAME)


    #删除用户权限
    def _subuser(self):
        self.userlist = self._loaduserlist()
        self.subwidght = subuserwidget(self, self.userlist, self.list_level)
        # 必须加这句
        self.subwidght.setWindowModality(QtCore.Qt.ApplicationModal)
        self.subwidght.exec_()
        self.userlist = self.subwidght.getrefreshuserlist()
        self.saveUserlist()


    def _loaduserlist(self):
        userlist = []
        try:
            list_data = self.excel_ssh.read_data_line_for_sheet(self.LEVEL_SHEET_NAME)
            #print ("showall:", list_data)
            if list_data != []:
                for nindex, item in enumerate(list_data):
                    if nindex == 0:  # 去掉头
                        continue
                    #print ("item", self.decrypt(item[2]).decode('utf-8'))
                    userlist.append([item[0], item[1], self.decrypt(item[2]).decode('utf-8'), item[3]])
            else:
                # from 。。。。import 。。。。。
                # kxlog(self, logging.ERROR, '用户文件不存在,已初始化')#TODO 记得要取消注释
                s_all = ""
                for i in range(self.h_levelmanager.MAX_MODULE_NUM):
                    s_all += "1"
                self.userlist = [['出厂用户', 'zs20210401', s_all, '2021-04-01 08:30:00']]
                self.saveUserlist()
                return self.userlist
            return userlist
        except Exception as e:
            print("exception:", e)


    def getUserlist(self):
        return self.userlist

            #kxlog(self, logging.ERROR, '读取用户列表的时候发生错误，错误日志%s'%str(e))


    def saveUserlist(self):
        """
        保存用户列表，保存的时候是包括head一起覆盖保存
        :return:
        """
        list_list_userlist = []
        for list_user in self.userlist:
            list_data = copy.copy(list_user)
            if len(list_user) > 3: #保护
                list_data[2] = self.encrypt(list_data[2]).decode('utf-8')
            list_list_userlist.append(list_data)
        list_list_data = []
        list_list_data.append(self._CSV_SAVE_HEAD)
        list_list_data.extend(list_list_userlist)
        self.excel_ssh.writeExcelinOverwrite(list_list_data, self.LEVEL_SHEET_NAME)


    def adduser(self):
        if self.ui.userlineEdit_name.text() != "" and self.ui.userlineEdit.text() != "":
            curlevele = self.h_levelmanager.getlevellist(self.ui.comboBox.currentIndex())
            idex = [self.ui.userlineEdit_name.text(), self.ui.userlineEdit.text(), curlevele, self.now()]
            for i in range(len(self.userlist)):
                if self.userlist[i][0] == idex[0]:
                    root = tkinter.Tk()
                    root.withdraw()
                    tkinter.messagebox.showinfo('提示', '该用户名已存在！')
                    return
                elif self.userlist[i][1] == idex[1]:
                        root = tkinter.Tk()
                        root.withdraw()
                        tkinter.messagebox.showinfo('提示', '该卡号已存在！')
                        return

            self.userlist.append(idex)

            self.saveUserlist()
            root = tkinter.Tk()
            root.withdraw()
            tkinter.messagebox.showinfo('提示', '添加新用户成功！')
            self.ui.userlineEdit_name.clear()
            self.ui.userlineEdit.clear()
            self.close()
        else:
            root = tkinter.Tk()
            root.withdraw()
            tkinter.messagebox.showinfo('提示', '用户名以及键盘必须非空')
            return


    def now(self):
        return time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()))


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

class CmanageLevelTool(QtWidgets.QDialog):
    """
    比如一共有N个模块的权限，这个类把这N个模块分为了六类，每一类里的模块也可设置
    """
    MAX_LEVEL_NUM = 6
    MAX_MODULE_NUM = 15
    def __init__(self, list_modulename, list_levelname=[]):
        super(CmanageLevelTool, self).__init__()
        self.list_modulename = list_modulename
        self.list_levelname = list_levelname
        self._initui()
        # self.setMinimumWidth(1100)
        # self.setMaximumWidth(1100)
        self._initlabelname(list_modulename, list_levelname)
        self._hidecheckbox_label(len(list_modulename))
        self.pushbutton_save.clicked.connect(self.close)


    def _initui(self):
        self.list_label = []

        self.list_checkbox = []

        self.list_labellevel = []

        nminw = 150

        font = QtGui.QFont()

        font.setPointSizeF(15)

        font.setBold(True)

        widget1 =QtWidgets.QWidget()

        horlayout1 = QtWidgets.QHBoxLayout(widget1)

        label = QtWidgets.QLabel(self)

        label.setMinimumWidth(nminw)

        label.setFont(font)

        label.setAlignment(QtCore.Qt.AlignCenter)

        horlayout1.addWidget(label, 1)

        for i in range(self.MAX_MODULE_NUM):

            label = QtWidgets.QLabel(self)

            label.setMinimumWidth(nminw)

            label.setMaximumWidth(100)

            label.setFont(font)

            label.setAlignment(QtCore.Qt.AlignCenter)

            label.setText("权限%d"%i)

            self.list_label.append(label)

            horlayout1.addWidget(label, 1)

        widget2 = QtWidgets.QWidget()

        verlayout = QtWidgets.QVBoxLayout(widget2)

        for j in range(self.MAX_LEVEL_NUM):

            widget = QtWidgets.QWidget()

            horlayout = QtWidgets.QHBoxLayout(widget)

            label = QtWidgets.QLabel(self)

            label.setText("权限等级")

            label.setFont(font)

            label.setMinimumWidth(nminw)

            horlayout.addWidget(label)

            self.list_labellevel.append(label)

            list_checkbox = []

            for i in range(self.MAX_MODULE_NUM):

                checkbox = QtWidgets.QCheckBox()

                checkbox.setText("")

                checkbox.setStyleSheet("QCheckBox::indicator\n"
                                        "{\n"
                                        "    width:80px;\n"
                                        "    height:80px;\n"
                                        "}\n"
                                       "QCheckBox::indicator::unchecked\n"
                                       "{ border-image: url(res/checkbox.png); }  \n"
                                       "QCheckBox::indicator::checked\n"
                                       " { border-image: url(res/checkbox-checked.png); } \n"
                                       )

                checkbox.setMinimumWidth(150)

                list_checkbox.append(checkbox)

                horlayout.addWidget(checkbox)

            verlayout.addWidget(widget)

            self.list_checkbox.append(list_checkbox)

        self.pushbutton_save = QtWidgets.QPushButton(self)

        self.pushbutton_save.setText("保存")

        self.pushbutton_save.setMinimumHeight(150)

        self.horlayout = QtWidgets.QVBoxLayout(self)

        self.horlayout.addWidget(widget1)

        self.horlayout.addWidget(widget2)

        self.horlayout.addWidget(self.pushbutton_save)


    def _initlabelname(self, list_modulename, list_levelname):
        """
        初始化label名称
        """
        for i in range(len(list_modulename)):

            self.list_label[i].setText(list_modulename[i])

        for i in range(len(list_levelname)):

            self.list_labellevel[i].setText(list_levelname[i])


    def _hidecheckbox_label(self, nshownum):
        """
        隐藏没有作用的checkbox,以及这些checkbox的title label
        """
        for i in range(self.MAX_LEVEL_NUM):

            for j in range(nshownum, self.MAX_MODULE_NUM):

                self.list_checkbox[i][j].setVisible(False)

        for j in range(nshownum, self.MAX_MODULE_NUM):

            self.list_label[j].setVisible(False)



    def getlevellist(self, nindex):
        """
        获取该权限下的权限列表
        """
        if nindex >= self.MAX_LEVEL_NUM:

            return '0' * self.MAX_MODULE_NUM

        else:

            str_result = ''

            list_checkbox = self.list_checkbox[nindex]

            for checkbox in list_checkbox:

                if checkbox.isChecked():

                    str_result += '1'

                else:

                    str_result += '0'

            return str_result


    def getinfo2save(self):
        """
        返回所有checkbox状态用双列表的方式保存下来
        """
        list_list_status = []

        for list_checkbox in self.list_checkbox:

            list_status = []

            for checkbox in list_checkbox:

                if checkbox.isChecked():

                    list_status.append('1')

                else:

                    list_status.append('0')

            list_list_status.append(list_status)

        return list_list_status


    def setinfo(self, list_list_status):
        """设置权限信息， 注意保护"""
        for nindex, list_status in enumerate(list_list_status):

            if nindex < len(self.list_checkbox):

                for njndex, status in enumerate(list_status):

                    if njndex < len(self.list_checkbox[nindex]):

                        if status == "0":

                            self.list_checkbox[nindex][njndex].setChecked(False)

                        else:

                            self.list_checkbox[nindex][njndex].setChecked(True)





if __name__ == "__main__":
    a = QtWidgets.QApplication([])
    w = CAddUserDialogNew(["账号管理", "控制工具栏",  "配方选择", "参数修改",
                                                                     "CP/CTS参数修改", "MES", "屏蔽检测区域"])
    w.show()
    a.exec_()