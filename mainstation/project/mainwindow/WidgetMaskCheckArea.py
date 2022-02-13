from PyQt5 import QtCore, QtWidgets, QtGui


class CWidgetMaskCheckArea(QtWidgets.QWidget):
    """
    界面上直观显示检测区域数量，并且每次变化都需要设置一下
    （1）程序启动的时候需要外部设置roi数量，输入列表，列表由0、1数字组成
    （2）外部也即模板变化的时候需要更新，输入列表，列表由0、1数字组成
    （3）外部可以返回检测框情况，返回列表，列表由 0、1数字组成
    （4）保存时主动回调，将状态设置给参数
    """
    def __init__(self, hparent, list_data):
        super(CWidgetMaskCheckArea, self).__init__()
        self.hparent = hparent
        self._initui()
        self.push_button.clicked.connect(self.callback2changecheckstatus)
        self.setcheckarea(list_data)

    def setcheckarea(self, list_data):

        for nindex, data in enumerate(list_data):

            self.list_checkbox[nindex].setVisible(True)

            self.list_checkbox[nindex].setChecked(bool(data))

        #隐藏不需要的
        for nindex in range(len(list_data), len(self.list_checkbox)):

            self.list_checkbox[nindex].setVisible(False)



    def _initui(self):
        self.list_checkbox = []
        self.verlayout = QtWidgets.QVBoxLayout(self)

        for j in range(4):
            horwidget = QtWidgets.QWidget(self)
            horbiglayout = QtWidgets.QHBoxLayout(horwidget)
            for i in range(8):
                checkbox = QtWidgets.QCheckBox(self)
                font = QtGui.QFont()
                font.setFamily("Arial")
                font.setPointSize(18)
                font.setBold(False)
                font.setWeight(50)
                checkbox.setFont(font)
                checkbox.setText("屏蔽检测区域%d"%(j * 8 + i))
                horbiglayout.addWidget(checkbox)
                self.list_checkbox.append(checkbox)
            self.verlayout.addWidget(horwidget)

        self.push_button = QtWidgets.QPushButton(self)
        self.push_button.setMinimumHeight(150)
        font = QtGui.QFont()
        font.setFamily("Arial")
        font.setPointSize(30)
        self.push_button.setFont(font)
        self.push_button.setText("保存")
        self.verlayout.addWidget(self.push_button)

    def getcheckarea(self):
        list_data = []

        for checkbox in self.list_checkbox:

            if not checkbox.isHidden():

                if checkbox.isChecked():

                    list_data.append(1)

                else:

                    list_data.append(0)

            else:

                break

        return list_data


    def callback2changecheckstatus(self):

        self.hparent.callback2changecheckstatus(self.getcheckarea())


if __name__ == "__main__":
    A = QtWidgets.QApplication([])
    w = CWidgetMaskCheckArea([1, 1, 1, 1, 0,0 ])
    print(w.getcheckarea())
    w.show()
    A.exec_()