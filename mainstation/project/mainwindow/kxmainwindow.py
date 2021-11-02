from library.mainwindow.BaseMainWindow import KXBaseMainWidget
from PyQt5 import QtGui,QtCore,QtWidgets
from PyQt5.QtWidgets import QWidget
from library.common.BaseRunLog import KxBaseRunLog
from library.monitoring.BaseMonitoringWidget import  KxBaseMonitoringWidget
from library.common.Permission_Management import kxprivilege_management
from project.monitoring import * #这行import保证实时界面能够挂载
from project.param import * #这行保证参数设置界面能挂载
from project.param.ChipParametersetting import ChipParameterSetting
from project.other.WorkList import WorkListWidget

class kxmainwindow(KXBaseMainWidget):
    def __init__(self, dict_config):
        super(kxmainwindow, self).__init__(dict_config)
        self.widget_Realtime = KxBaseMonitoringWidget.create(name=dict_config["mointoringwidget_classname"], h_parent=self)
        self.widget_Paramsetting = ChipParameterSetting(hparent=self, dict_config=dict_config)#参数设置
        self.widget_runlog = KxBaseRunLog(self)#日志
        self.widget_permission = kxprivilege_management()#权限管理
        self.widge_worklist = WorkListWidget(self)


        self._initstackwidget([self.ui.pbt_realtime, self.ui.pbt_paramset, self.ui.pbt_logview, self.ui.pushButton_worklist],
                              [self.widget_Realtime, self.widget_Paramsetting, self.widget_runlog, self.widge_worklist])
        self._completeui()
        self._completeconnect()
        self.ui.label_2.setText("下箱体托盘检测")



    def  _completeui(self):
        self.toolbutton_move = QtWidgets.QToolButton(self)
        self.toolbutton_move.setMinimumSize(QtCore.QSize(100, 100))
        self.toolbutton_move.setMaximumSize(QtCore.QSize(100, 100))
        self.toolbutton_move.setIcon(QtGui.QIcon('res/设备点检.png'))
        self.toolbutton_move.setStyleSheet('color:white;border:none;')
        self.toolbutton_move.setIconSize(QtCore.QSize(100, 90))
        self.toolbutton_move.setToolButtonStyle(QtCore.Qt.ToolButtonTextUnderIcon)
        self.ui.verticalLayout_2.addWidget(self.toolbutton_move)

        self.QPixmap_disMES = QtGui.QPixmap('res\\MES.png')
        self.label_MES = QtWidgets.QLabel(self.ui.labelwidget)  # 硬件网络连接
        self.label_MES.setAlignment(QtCore.Qt.AlignCenter)
        self.label_MES.setFixedWidth(70)
        self.label_MES.setPixmap(self.QPixmap_disMES)
        self.statusBar.addWidget(self.label_MES)

        #self.ui.toolButton_userlevel.setStyleSheet('color:white;border:none;')
        #self.ui.toolButton_userlevel.setIconSize(QtCore.QSize(100, 90))
        #self.ui.toolButton_userlevel.setToolButtonStyle(QtCore.Qt.ToolButtonTextUnderIcon)
        font = QtGui.QFont()
        font.setFamily("Arial")
        font.setPointSize(14)
        self.ui.toolButton_userlevel.setFont(font)

    def _completeconnect(self):
        self.ui.toolButton_userlevel.clicked.connect(self.showpermissiondialog)

    def _setlearnstatus(self):
        if self.ui.toolbtn_learn.isChecked():
            self.widget_Paramsetting.str2paramitemfun(0, 1, "setlearnstatus", True)
        else:
            self.widget_Paramsetting.str2paramitemfun(0, 1, "setlearnstatus", False)

    def _offlinerun(self):
        super(kxmainwindow, self)._offlinerun()
        if self.ui.toolbtn_offlinerun.isChecked():  # 开始离线跑
            self.widget_Realtime.clear()

    def showpermissiondialog(self):
        self.widget_permission.setpasswordpath("d:\\")
        self.widget_permission.exec_()
        permissonlevel = self.widget_permission.getpermissionlevel()
        self.updatepermission(permissonlevel)

    def updatepermission(self, PERMISSIONLEVEL):

        self.ui.toolButton_userlevel.setStyleSheet( 'color:white;border:none;')
        if PERMISSIONLEVEL == 0:
            self.ui.toolButton_userlevel.setText("操作员")
        elif PERMISSIONLEVEL == 1:
            self.ui.toolButton_userlevel.setText("ME")
        elif PERMISSIONLEVEL == 2:
            self.ui.toolButton_userlevel.setText("IMD")
        else:
            self.ui.toolButton_userlevel.setText("管理员")
