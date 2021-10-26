from library.mainwindow.BaseMainWindow import KXBaseMainWidget
from PyQt5 import QtGui,QtCore,QtWidgets
from PyQt5.QtWidgets import QWidget
from library.common.BaseRunLog import KxBaseRunLog
from library.monitoring.BaseMonitoringWidget import  KxBaseMonitoringWidget
from library.common.Permission_Management import kxprivilege_management
from project.monitoring import * #这行import保证实时界面能够挂载
from project.param import * #这行保证参数设置界面能挂载
from project.param.ChipParametersetting import ChipParameterSetting

class kxmainwindow(KXBaseMainWidget):
    def __init__(self, dict_config):
        super(kxmainwindow, self).__init__(dict_config)
        self.widget_Realtime = KxBaseMonitoringWidget.create(name=dict_config["mointoringwidget_classname"], h_parent=self)
        self.widget_Paramsetting = ChipParameterSetting(hparent=self, dict_config=dict_config)#参数设置
        self.widget_runlog = KxBaseRunLog(self)#日志
        self.widget_permission = kxprivilege_management()#权限管理
        self._initstackwidget([self.ui.pbt_realtime, self.ui.pbt_paramset, self.ui.pbt_logview],
                              [self.widget_Realtime, self.widget_Paramsetting, self.widget_runlog])
        self._completeui()
        self.ui.label_2.setText("下箱体托盘检测")


    def  _completeui(self):
        self.toolbutton_move = QtWidgets.QToolButton(self)
        self.toolbutton_move.setMinimumSize(QtCore.QSize(100, 100))
        self.toolbutton_move.setMaximumSize(QtCore.QSize(100, 100))
        self.toolbutton_move.setIcon(QtGui.QIcon('res/startmove.png'))
        self.toolbutton_move.setStyleSheet('color:white;border:none;')
        self.toolbutton_move.setIconSize(QtCore.QSize(100, 90))
        self.toolbutton_move.setText('测试')
        self.toolbutton_move.setToolButtonStyle(QtCore.Qt.ToolButtonTextUnderIcon)
        self.ui.verticalLayout_2.addWidget(self.toolbutton_move)

    def _setlearnstatus(self):
        if self.ui.toolbtn_learn.isChecked():
            self.widget_Paramsetting.str2paramitemfun(0, 1, "setlearnstatus", True)
        else:
            self.widget_Paramsetting.str2paramitemfun(0, 1, "setlearnstatus", False)

    def _offlinerun(self):
        super(kxmainwindow, self)._offlinerun()
        if self.ui.toolbtn_offlinerun.isChecked():  # 开始离线跑
            self.widget_Realtime.clear()
