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
        self.nroinum = len(list_data)
        self._initui()
        self.push_button.clicked.connect(self.callback2changecheckstatus)
        self.setcheckarea(list_data)
        self.slot_checkboxclicked()# 初始化


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
                font.setBold(True)
                font.setWeight(50)
                checkbox.setFont(font)
                checkbox.setText("屏蔽检测区域%d"%(j * 8 + i))
                checkbox.setStyleSheet("QCheckBox::indicator\n"
                                        "{\n"
                                        "    width:40px;\n"
                                        "    height:40px;\n"
                                        "}\n"
                                       "QCheckBox::indicator::unchecked\n"
                                       "{ border-image: url(res/checkbox.png); }  \n"
                                       "QCheckBox::indicator::checked\n"
                                       " { border-image: url(res/checkbox-checked.png); } \n"
                                       )
                checkbox.clicked.connect(self.slot_checkboxclicked)
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

        self.widget_map = Ccarmap(2, 3)
        self.verlayout.addWidget(self.widget_map)
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



    def slot_checkboxclicked(self):
        list_checkstatus = []
        for i in range(self.nroinum):
            if self.list_checkbox[i].isChecked():
                list_checkstatus.append(1)
            else:
                list_checkstatus.append(0)
        self.widget_map.setselected(list_checkstatus)



class Ccarmap(QtWidgets.QWidget):
    """
    小车地图，通过它设置一共有多少个检测区域
    其实就是Tablewidget，然后设置让每个格子亮
    另外要注意下顺序
    """
    def __init__(self, nrow, ncol):
        super(Ccarmap, self).__init__()
        self._initui()
        self.row = 3
        self.col = 2
        self.TableWidget.setRowCount(self.row)
        self.TableWidget.setColumnCount(self.col)
        self.row = 3
        self.col = 2
        self._init_item()


    def _initui(self):
        layout = QtWidgets.QHBoxLayout()
        # 实现的效果是一样的，N行M列，所以要灵活运用函数，这里只是示范一下如何单独设置行列
        self.TableWidget = QtWidgets.QTableWidget()
        self.TableWidget.verticalHeader().setVisible(False)
        self.TableWidget.horizontalHeader().setVisible(False)

        # TableWidget = QTableWidget()
        # TableWidget.setRowCount(4)
        # TableWidget.setColumnCount(3)
        # 设置水平方向的表头标签与垂直方向上的表头标签，注意必须在初始化行列之后进行，否则，没有效果
        #self.TableWidget.setHorizontalHeaderLabels(['检测区域1', '检测区域2', '检测区域3', '检测区域4', '检测区域5', '检测区域6'])
        # Todo 优化1 设置垂直方向的表头标签
        #self.TableWidget.setVerticalHeaderLabels(['左', '右', '上', '下', '涂胶面积'])

        # for i in range(4):
        #     self.TableWidget.setRowHeight(i, 25)

        # TODO 优化 2 设置水平方向表格为自适应的伸缩模式
        self.TableWidget.horizontalHeader().setSectionResizeMode(QtWidgets.QHeaderView.Stretch)
        self.TableWidget.verticalHeader().setSectionResizeMode(QtWidgets.QHeaderView.Stretch)

        # TODO 优化3 将表格变为禁止编辑
        self.TableWidget.setEditTriggers(QtWidgets.QAbstractItemView.NoEditTriggers)

        # TODO 优化 4 设置表格整行选中
        self.TableWidget.setSelectionBehavior(QtWidgets.QAbstractItemView.SelectRows)

        # TODO 优化 5 将行与列的高度设置为所显示的内容的宽度高度匹配
        # QTableWidget.resizeColumnsToContents(TableWidget)
        QtWidgets.QTableWidget.resizeRowsToContents(self.TableWidget)

        # TODO 优化 6 表格头的显示与隐藏
        # TableWidget.verticalHeader().setVisible(False)
        # TableWidget.horizontalHeader().setVisible(False)

        # TOdo 优化7 在单元格内放置控件
        # comBox=QComboBox()
        # comBox.addItems(['男','女'])
        # comBox.addItem('未知')
        # comBox.setStyleSheet('QComboBox{margin:3px}')
        # TableWidget.setCellWidget(0,1,comBox)
        #
        # searchBtn=QPushButton('修改')
        # searchBtn.setDown(True)
        # searchBtn.setStyleSheet('QPushButton{margin:3px}')
        # TableWidget.setCellWidget(0,2,searchBtn)

        # 添加数据
        # newItem=QtWidgets.QTableWidgetItem('张三')
        # TableWidget.setItem(0,0,newItem)
        #
        # newItem=QtWidgets.QTableWidgetItem('男')
        # TableWidget.setItem(0,1,newItem)
        #
        # newItem=QtWidgets.QTableWidgetItem('160')
        # TableWidget.setItem(0,2,newItem)


        self.label1 = QtWidgets.QLabel(self)
        self.label2 = QtWidgets.QLabel(self)
        font = QtGui.QFont()
        font.setPointSize(50)
        font.setBold(True)
        self.label1.setFont(font)
        self.label1.setText("车\n\n\n头")
        self.label1.setAlignment(QtCore.Qt.AlignCenter)
        self.label2.setFont(font)
        self.label2.setText("车\n\n\n尾")
        self.label2.setAlignment(QtCore.Qt.AlignCenter)
        layout.addWidget(self.label1, 1)
        layout.addWidget(self.TableWidget, 5)
        layout.addWidget(self.label2, 1)
        self.setLayout(layout)


    def _init_item(self):
        font = QtGui.QFont()
        font.setPointSize(20)
        font.setBold(True)

        newItem = QtWidgets.QTableWidgetItem("  检测区域4")
        newItem.setFont(font)
        newItem.setBackground(QtGui.QColor(255, 255, 255))
        self.TableWidget.setItem(0, 0, newItem)
        newItem = QtWidgets.QTableWidgetItem("  检测区域5")
        newItem.setFont(font)
        newItem.setBackground(QtGui.QColor(255, 255, 255))
        self.TableWidget.setItem(0, 1, newItem)
        newItem = QtWidgets.QTableWidgetItem("  检测区域2")
        newItem.setFont(font)
        newItem.setBackground(QtGui.QColor(255, 255, 255))
        self.TableWidget.setItem(1, 0, newItem)
        newItem = QtWidgets.QTableWidgetItem("  检测区域3")
        newItem.setFont(font)
        newItem.setBackground(QtGui.QColor(255, 255, 255))
        self.TableWidget.setItem(1, 1, newItem)
        newItem = QtWidgets.QTableWidgetItem("  检测区域0")
        newItem.setFont(font)
        newItem.setBackground(QtGui.QColor(255, 255, 255))
        self.TableWidget.setItem(2, 0, newItem)
        newItem = QtWidgets.QTableWidgetItem("  检测区域1")
        newItem.setFont(font)
        newItem.setBackground(QtGui.QColor(255, 255, 255))
        self.TableWidget.setItem(2, 1, newItem)
        self.TableWidget.setEnabled(False)


    def setselected(self, list_status):
        try:
            list_row = [2, 2, 1, 1, 0, 0]
            list_col = [0, 1, 0, 1, 0, 1]

            font = QtGui.QFont()
            font.setPointSize(20)
            font.setBold(True)
            for i, status in enumerate(list_status):
                if status:
                    newItem = QtWidgets.QTableWidgetItem("  检测区域%d"%i)
                    newItem.setFont(font)
                    newItem.setBackground(QtGui.QColor(255, 0, 0))
                    self.TableWidget.setItem(list_row[i], list_col[i], newItem)
                else:
                    newItem = QtWidgets.QTableWidgetItem("  检测区域%d"%i)
                    newItem.setFont(font)
                    newItem.setBackground(QtGui.QColor(255, 255, 255))
                    self.TableWidget.setItem(list_row[i], list_col[i], newItem)
        except Exception as e:
            print (e)





if __name__ == "__main__":
    A = QtWidgets.QApplication([])
    w = CWidgetMaskCheckArea(None, [1, 1, 1, 1, 0,0 ])
    #w = Ccarmap(3, 2)
    w.show()
    A.exec_()