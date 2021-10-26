#coding: utf-8

from PyQt5 import QtWidgets,QtCore
from UI.ui_loginInterface import Ui_LoginDialog
from library.common.readconfig import ReadConfigWidget
from library.common.readconfig import ReadConfigDialog
from library.common.readconfig import readconfig

class LoginInterface(QtWidgets.QDialog):
    """
    登录窗口类，在进入正式的程序之前用户可使用它进行修改配置（子站，主站）
    先改主站，确定子站的配置位置，再改子站配置
    """
    def __init__(self, mainconfigpath, subconfigpath):
        super(LoginInterface, self).__init__()
        self.ui = Ui_LoginDialog()
        self.ui.setupUi(self)
        self.s_mainconfigpath = mainconfigpath
        self.s_subconfigpath = subconfigpath
        self.b_status = False
        self.__initui()
        self.setWindowFlags(QtCore.Qt.FramelessWindowHint)#全屏


    def __initui(self):
        self.ui.pushButton_quit.clicked.connect(self.close)
        self.ui.pushButton_login.clicked.connect(self.__changestatus)
        self.ui.pushButton_main.clicked.connect(self.__showmainconfigdialog)
        self.ui.pushButton_sub.clicked.connect(self.__showsubconfigdialog)



    def __showmainconfigdialog(self):
        widget_config = ReadConfigDialog(self.s_mainconfigpath)
        if widget_config.b_initstatus:
            widget_config.show()
            widget_config.exec_()

    def __showsubconfigdialog(self):
        dict_mainconfig = readconfig(self.s_mainconfigpath)
        list_subconfigpath = []
        try:
            #将末尾checkstation.exe替换成self.s_subconfigpath
            list_exepath = [dict_mainconfig[u'substation'][stationname][u'exepath'] for stationname in
                            dict_mainconfig[u'substation']]

            for exepath in list_exepath:
                subexepath = exepath.split("\\")
                subexepath[-1] = self.s_subconfigpath
                list_subconfigpath.append("\\".join(subexepath))
        except KeyError as e:
            QtWidgets.QMessageBox.warning(self, u"配置异常", u"主站配置缺少键值：" + str(e), QtWidgets.QMessageBox.Ok)
            return
        widget_config = ReadConfigDialog(list_subconfigpath)
        if widget_config.b_initstatus:
            widget_config.show()
            widget_config.exec_()


    def __changestatus(self):
        self.b_status = True
        self.close()

    def getstatus(self):
        return self.b_status

if __name__ == "__main__":
    A = QtWidgets.QApplication([])
    w = LoginInterface()
    w.show()
    w.exec_()
