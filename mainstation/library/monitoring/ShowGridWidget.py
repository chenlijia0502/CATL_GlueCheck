# coding=utf-8

'''
Created on 2018年4月10日

@author: kexin
'''
from PyQt5 import QtWidgets, QtGui, QtCore
import sys, cv2, logging
import numpy as np


class ImgListDetectWidget(QtWidgets.QWidget):
    #__latestDefectCount = 40#单个页面显示最多
    _imgCntPerCol = 5 #每列显示（行）
    __imgCntPerRow = 6 #每行显示（列）

    def __init__(self, parent=None):
        super(ImgListDetectWidget, self).__init__(parent)
        self.h_parentwidget = parent
        self.maxDefectShowCount = 40
        self.maxDefectCountPerSec = 10
        self.compressionRatio = 75
        self.curDefectTablePos = [0, 0]
        self.defectTableValidCount = 0
        self.isAutoScrollAndShow = True
        self.tableRowHeight = 100
        self.tableRowWidth = 100
        self.initUi()
        self.initLogInfo()


    def initUi(self):
        self.historyShowArea = TableWidget()
        self.historyScrollBar = QtWidgets.QScrollBar()
        enlargeRatio = -1
        layout_9 = QtWidgets.QHBoxLayout(self)
        layout_9.addWidget(self.historyShowArea)
        layout_9.addWidget(self.historyScrollBar)
        layout_9.setSpacing(0)
        layout_9.setContentsMargins(0, 0, 0, 0)
        layout = QtWidgets.QGridLayout(self)
        layout.addLayout(layout_9, 0, 0, 1, 2)
        layout.setSpacing(0)
        layout.setContentsMargins(0, 0, 0, 0)
        self.inithistoryShowArea(enlargeRatio=enlargeRatio)

    def initLogInfo(self):
        self.logger = logging.getLogger('UI.%s' % self.__class__.__name__)

    def inithistoryShowArea(self, enlargeRatio=-1, isFirstInit=True):
        #self.historyShowArea.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.historyShowArea.clear()
        self.historyShowArea.setRowCount(self._imgCntPerCol)
        self.historyShowArea.setColumnCount(self.__imgCntPerRow)

        for row in range(self.historyShowArea.rowCount()):
            for column in range(self.historyShowArea.columnCount()):
                imgShowWidget = ComboImageLabelWidget()
                self.historyShowArea.setCellWidget(row, column, imgShowWidget)
            self.historyShowArea.setRowHeight(row, self.tableRowHeight)

        self.historyShowArea.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAsNeeded)
        self.historyScrollBar.setRange(0, 0)
        self.historyScrollBar.setValue(0)
        self.historyShowArea.updateGeometries()
        self.historyShowArea.update()
        self.historyShowArea.viewport().update()

        if isFirstInit:
            self.historyShowArea.horizontalHeader().setVisible(False)
            self.historyShowArea.verticalHeader().setVisible(False)
            #self.historyShowArea.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
            #self.historyShowArea.horizontalHeader().setResizeMode(QtGui.QHeaderView.Stretch)
            self.historyShowArea.setVerticalScrollBar(self.historyScrollBar)
            self.historyScrollBar.setPageStep(1)
            self.historyScrollBar.setSingleStep(1)
            self.historyShowArea.setShowGrid(False)
            self.historyShowArea.setStyleSheet('''QTableWidget::item{border:5px solid gray;}
                                                  QTableWidget::item:selected{border:2px solid yellow;}''')

            self.historyShowArea.setContextMenuPolicy(QtCore.Qt.CustomContextMenu)
            self.historyShowArea.customContextMenuRequested.connect(self.historyTableContextMenu)
            self.historyShowArea.verticalScrollBar().installEventFilter(self)
            self.historyShowArea.installEventFilter(self)

    # def eventFilter(self, obj, event):
    #     if event.type() == QtCore.QEvent.Wheel:
    #         leftTop, rightBottom = QtCore.QPoint(0, 0), QtCore.QPoint(
    #             self.historyShowArea.verticalScrollBar().width() - 1,
    #             self.historyShowArea.verticalScrollBar().height() - 1)
    #         leftTop = self.historyShowArea.verticalScrollBar().mapToGlobal(leftTop)
    #         rightBottom = self.historyShowArea.verticalScrollBar().mapToGlobal(rightBottom)
    #         rect = QtCore.QRect(leftTop, rightBottom)
    #         if rect.contains(event.globalPos()):
    #             obj.wheelEvent(QtGui.QWheelEvent(QtCore.QPoint(event.pos().x(), event.pos().y()), event.delta() / 4,
    #                                              event.buttons(), event.modifiers(), event.orientation()))
    #             return True
    #         else:
    #             return True
    #     return QtCore.QObject.eventFilter(self, obj, event)

    def slotIsAutoScrollChanged(self, newState):
        if newState == QtCore.Qt.Unchecked:
            self.isAutoScrollAndShow = False
        else:
            self.isAutoScrollAndShow = True



    def updateImgView(self, imgView, img):
        """
        yl 2020.04.26 更新每个小格子中的缺陷图，此处包含缺陷图像显示在一个个的小格子中
        :param imgView:
        :param img:
        :return:
        """
        try:
            img = cv2.resize(img, (100, 100), interpolation=cv2.INTER_LINEAR)
            imgView.clear()

            # img = self.grey_scale(img)
            if img.ndim == 2:
                img = cv2.cvtColor(img, cv2.COLOR_GRAY2RGB)
            imgView.setImage(img)
        except Exception as e:
            self.logger.error('', exc_info=True)

    def grey_scale(self, image):
        rows,cols = image.shape
        flat_gray = image.reshape((cols * rows,)).tolist()
        A = min(flat_gray)
        B = max(flat_gray)
        if self.grayScaleIndex == 0:
            output = np.uint8(255 / (B - A + 1) * (image - A + 1) + 0.5)
        elif (self.grayScaleIndex == 1 or self.grayScaleIndex == 2):
            output = cv2.multiply(self.grayScaleIndex * 2, image - A)
        else:
            output = image
        return output


    def getCurHighLightView(self):
        if self.defectTableValidCount == 0:
            return None
        else:
            if self.curDefectTablePos.count(0) == len(self.curDefectTablePos):
                return (self.historyShowArea.rowCount() - 1, self.historyShowArea.columnCount() - 1)
            else:
                curRow, curCol = self.curDefectTablePos
                if curCol == 0:
                    return (curRow - 1, self.historyShowArea.columnCount() - 1)
                else:
                    return (curRow, curCol - 1)

    def getNextTablePos(self):
        curRow, curCol = self.curDefectTablePos
        # print("x",self.historyShowArea.columnCount())
        #print("y",self.historyShowArea.rowCount())
        if curCol == (self.historyShowArea.columnCount() - 1):
            if curRow + 1 >= self.historyShowArea.rowCount():  # 第4行写满了，然后写不下了
                if curRow + 1 < self.maxHistoryTableRowCount:  # 写满了第4行但是还没有写满第100行
                    self.historyShowArea.insertRow(curRow + 1)
                    self.historyScrollBar.setRange(0, max(0, self.historyShowArea.rowCount() -  (self._imgCntPerCol)))
                    self.historyShowArea.updateGeometries()
                    self.historyShowArea.setRowHeight(curRow + 1, self.tableRowHeight)
                    for column in range(self.historyShowArea.columnCount()):
                        imgShowWidget = ComboImageLabelWidget()
                        self.historyShowArea.setCellWidget(curRow + 1, column, imgShowWidget)
                    return [curRow + 1, 0]
                else:
                    return [0, 0]  # 重新从[0,0]来
            else:  # 还有匹配的行，直接填写
                return [curRow + 1, 0]
        else:
            return [curRow, curCol + 1]



    def historyTableContextMenu(self, point):
        row, column = self.historyShowArea.rowAt(point.y()), self.historyShowArea.columnAt(point.x())
        if (self.historyShowArea.columnCount() * row + column) in range(self.defectTableValidCount):
            contextMenu = QtGui.QMenu(self.historyShowArea)
            contextMenu.addAction(self.partialLearnAction)
            contextMenu.addAction(self.setNgCheckResultAction)
            contextMenu.addAction(self.setOkCheckResultAction)
            contextMenu.exec_(QtGui.QCursor.pos())


    def clear(self):
        self.inithistoryShowArea(isFirstInit=False)
        # self.defectTypeId2NameDict = [None for i in range(self.h_parentwidget.getStationCount())]
        self.curDefectTablePos = [0, 0]
        self.defectTableValidCount = 0
        #self.initStatisticTable()
        #self.closeFileHandle()

    def resizeEvent(self, event):
        self.tableRowHeight = int(self.historyShowArea.geometry().height() * 0.99 / \
                              (self._imgCntPerCol))
        self.tableRowWidth = int(self.historyShowArea.geometry().width() * 0.99 / \
                                  (self.__imgCntPerRow))

        for row in range(self.historyShowArea.rowCount()):
            self.historyShowArea.setRowHeight(row, self.tableRowHeight)
        for col in range(self.historyShowArea.columnCount()):
            self.historyShowArea.setColumnWidth(col, self.tableRowWidth)

        QtWidgets.QWidget.resizeEvent(self, event)

    def setMaxGridNum(self,number):
        self.maxDefectShowCount = number
        self.maxHistoryTableRowCount = self.maxDefectShowCount / self.__imgCntPerRow

    def addOneDefectItem(self, defectImage):
        try:
            imgView = self.historyShowArea.cellWidget(*self.curDefectTablePos)
            if isinstance(imgView, ComboImageLabelWidget):
                self.updateImgView(imgView, defectImage)
                if self.defectTableValidCount < self.maxHistoryTableRowCount * self.__imgCntPerRow:
                    self.defectTableValidCount += 1
            self.curDefectTablePos = self.getNextTablePos()
        except Exception as e:
            self.logger.error('', exc_info=True)

    def addOneDefectItemwithText(self, defectImage, text1, text2=""):
        try:
            imgView = self.historyShowArea.cellWidget(*self.curDefectTablePos)
            if isinstance(imgView, ComboImageLabelWidget):
                self.updateImgView(imgView, defectImage)
                imgView.setShowValues((text1, text2))
                if self.defectTableValidCount < self.maxHistoryTableRowCount * self.__imgCntPerRow:
                    self.defectTableValidCount += 1
            self.curDefectTablePos = self.getNextTablePos()
        except Exception as e:
            self.logger.error('', exc_info=True)


# registerkxmointorwidget(name='ImgListDetectWidget', cls=ImgListDetectWidget, override=True)


class TableWidget(QtWidgets.QTableWidget):
    keyPressedSig = QtCore.pyqtSignal(object)
    rightBtnDoubleClicked = QtCore.pyqtSignal(object)

    def __init__(self, parent=None):
        super(TableWidget, self).__init__(parent)

    def keyPressEvent(self, event):
        if event.key() in [QtCore.Qt.Key_Left, QtCore.Qt.Key_Right, QtCore.Qt.Key_Enter, QtCore.Qt.Key_Return]:
            self.keyPressedSig.emit(event)
        else:
            super(TableWidget, self).keyPressEvent(event)  # QtCore.Qt.Key_Up, QtCore.Qt.Key_Down,

    def mouseDoubleClickEvent(self, event):
        if event.button() == QtCore.Qt.RightButton:
            selectedItem = self.itemAt(event.pos())
            if selectedItem:
                self.rightBtnDoubleClicked.emit(selectedItem.row())
        super(TableWidget, self).mouseDoubleClickEvent(event)




class ComboImageLabelWidget(QtWidgets.QWidget):
    def __init__(self, parent=None):
        super(ComboImageLabelWidget, self).__init__(parent)
        self.topLabel1 = QtWidgets.QLabel(self)
        self.topLabel1.setFixedHeight(40)
        self.topLabel1.setFont(QtGui.QFont('Consolas', 11, 59))
        self.topLabel1.setAlignment(QtCore.Qt.AlignLeft)
        self.topLabel2 = QtWidgets.QLabel(self)
        self.topLabel2.setFixedHeight(40)
        self.topLabel2.setFont(QtGui.QFont('Consolas', 11, 65))
        self.topLabel2.setAlignment(QtCore.Qt.AlignRight)
        self.imgLabel = QtWidgets.QLabel(self)
        self.imgLabel.setStyleSheet('background:black;')
        self.imgLabel.setAlignment(QtCore.Qt.AlignCenter)
        self.imgLabel.setScaledContents(True)
        layout_1 = QtWidgets.QHBoxLayout()
        #layout_1.setMargin(0)
        layout_1.setSpacing(0)
        layout_1.addWidget(self.topLabel1)
        layout_1.addWidget(self.topLabel2)
        layout = QtWidgets.QVBoxLayout(self)
        layout.addWidget(self.imgLabel, 10)
        layout.addLayout(layout_1, 1)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(0)
        self.setLabelBackGround(True)

        self.associateData = None
        self.initLogInfo()

    def initLogInfo(self):
        self.logger = logging.getLogger('UI.%s' % self.__class__.__name__)

    def setShowValues(self, vals):
        self.topLabel1.setText(vals[0])
        self.topLabel2.setText(vals[1])

    def setLabelBackGround(self, isOk):
        if isOk:
            self.topLabel1.setStyleSheet("background-color:#188bd1;color:black;")
            self.topLabel2.setStyleSheet("background-color:#188bd1;color:white;")
        else:
            self.topLabel1.setStyleSheet("background-color:red;color:black;")
            self.topLabel2.setStyleSheet("background-color:red;color:white;")

    def setBorderColor(self, showBorder):
        if showBorder:
            self.setStyleSheet("border:2px solid yellow;")
        else:
            self.setStyleSheet("border:0px;")

    def setImage(self, data):
        try:
            self.imgLabel.clear()
            if data is not None:
                pitch = data.shape[1] * 3 if data.ndim == 3 else data.shape[1]
                image = QtGui.QImage(data[:], data.shape[1], data.shape[0], pitch, QtGui.QImage.Format_RGB888)
                self.imgLabel.setPixmap(QtGui.QPixmap.fromImage(image))
        except Exception as e:
            self.logger.error('', exc_info=True)

    def getData(self):
        return self.associateData

    def setData(self, data):
        self.associateData = data

    def clear(self):
        self.topLabel1.clear()
        self.topLabel2.clear()
        self.imgLabel.clear()


if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    win = ImgListDetectWidget()
    win.setMaxGridNum(50)   #设置最多可以容纳多少个小格子来显示
    path1 = "F:\\Lcd_2CAA8ED23969\\Lcd_2CAA8ED23969_rgba_blue.bmp"
    path = "BaseImg2.bmp"
    path2 = "lena.bmp"
    defectImage1 = cv2.imread(path1)
    #defectImage2 = cv2.imread(path2)
    #defectImage = cv2.imread(path)

    #win.appendDefect(defectImage)  #调用appedDefect 1次
    for i in range(18):
        win.addOneDefectItemwithText(defectImage1, str(i))  #循环调用appendDefect 99次
    #win.addOneDefectItem(defectImage1)  #测试，满100个之后是否是从头开始显示图片的
    win.showMaximized()
    app.exec_()