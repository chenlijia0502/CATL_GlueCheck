from PyQt5 import QtGui, QtWidgets


class WidgetEdgePos(QtWidgets.QWidget):
    def __init__(self):
        super(WidgetEdgePos, self).__init__()
        self.initUI()

    def initUI(self):
        self.setWindowTitle("涂胶位置度偏移")
        layout=QtWidgets.QHBoxLayout()

        #实现的效果是一样的，N行M列，所以要灵活运用函数，这里只是示范一下如何单独设置行列
        self.TableWidget=QtWidgets.QTableWidget(4,6)

        # TableWidget = QTableWidget()
        # TableWidget.setRowCount(4)
        # TableWidget.setColumnCount(3)
        #设置水平方向的表头标签与垂直方向上的表头标签，注意必须在初始化行列之后进行，否则，没有效果
        self.TableWidget.setHorizontalHeaderLabels(['检测区域1', '检测区域2','检测区域3','检测区域4', '检测区域5', '检测区域6'])
        #Todo 优化1 设置垂直方向的表头标签
        #self.TableWidget.setVerticalHeaderLabels(['左', '右', '上', '下', '涂胶面积'])
        self.TableWidget.setVerticalHeaderLabels(['block面积', '涂胶面积', '异物面积', '气泡面积','左', '右', '上', '下'])

        for i in range(4):
            self.TableWidget.setRowHeight(i, 25)

        #TODO 优化 2 设置水平方向表格为自适应的伸缩模式
            self.TableWidget.horizontalHeader().setSectionResizeMode(QtWidgets.QHeaderView.Stretch)

        #TODO 优化3 将表格变为禁止编辑
        #TableWidget.setEditTriggers(QAbstractItemView.NoEditTriggers)

        #TODO 优化 4 设置表格整行选中
        self.TableWidget.setSelectionBehavior(QtWidgets.QAbstractItemView.SelectRows)

        #TODO 优化 5 将行与列的高度设置为所显示的内容的宽度高度匹配
        #QTableWidget.resizeColumnsToContents(TableWidget)
        QtWidgets.QTableWidget.resizeRowsToContents(self.TableWidget)

        #TODO 优化 6 表格头的显示与隐藏
        #TableWidget.verticalHeader().setVisible(False)
        #TableWidget.horizontalHeader().setVisible(False)

        #TOdo 优化7 在单元格内放置控件
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


        #添加数据
        # newItem=QtWidgets.QTableWidgetItem('张三')
        # TableWidget.setItem(0,0,newItem)
        #
        # newItem=QtWidgets.QTableWidgetItem('男')
        # TableWidget.setItem(0,1,newItem)
        #
        # newItem=QtWidgets.QTableWidgetItem('160')
        # TableWidget.setItem(0,2,newItem)

        layout.addWidget(self.TableWidget)

        self.setLayout(layout)


    def setdata(self, list_data, rowindex):
        for col, data in enumerate(list_data):
            newItem = QtWidgets.QTableWidgetItem(str(data) + "mm²")
            self.TableWidget.setItem(rowindex, col, newItem)


    def sethead(self, list_shead):

        self.TableWidget.setHorizontalHeaderLabels(list_shead)

        self.TableWidget.setVerticalHeaderLabels(['block面积', '涂胶面积', '异物面积', '气泡面积'])


    def clear(self):
        self.TableWidget.clear()


if __name__ == "__main__":
    a = QtWidgets.QApplication([])
    w = WidgetEdgePos()
    w.setdata([69369856, 69369856, 69369856, 69369856, 69369856, 69369856], 4)
    w.show()
    a.exec_()