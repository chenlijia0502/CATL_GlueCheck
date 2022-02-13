import cv2
import numpy as np
from PyQt5 import Qt, QtWidgets, QtGui
from pyqtgraph import GraphicsView
import pyqtgraph as pg
import copy


class FindEdgeToCalibrate(object):
    def __init__(self):
        super(FindEdgeToCalibrate, self).__init__()


    def _calboundrect(self, srcimg, roi):
        targetimg = srcimg[roi[1]:roi[3], roi[0]:roi[2], 0]

        ret, threshimg = cv2.threshold(targetimg, 100, 255, cv2.THRESH_BINARY)

        #nsum = (threshimg > 0).sum()

        im2, contours, hierarchy = cv2.findContours(threshimg, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)

        if len(contours) != 0:
            # the contours are drawn here
            cv2.drawContours(threshimg, contours, -1, 255, 3)

            ## find the biggest area of the contour
            c = max(contours, key=cv2.contourArea)

            x, y, w, h = cv2.boundingRect(c)

            naverage_left = roi[0] + x

            naverage_right = roi[0] + x + w - 1

            naverage_top = roi[1] + y

            naverage_bottom = roi[1] + y + h - 1

            paintimg = cv2.rectangle(srcimg, (naverage_left, naverage_top),
                                     (naverage_right, naverage_bottom), (0, 0, 255), 3)

            return paintimg, w, h

        else:

            return srcimg, 1, 1


    def solvemul(self, srcimg):
        # 宁德
        # list_roi = [[5852,  1250, 6150, 1680],
        #             [6250, 1260, 6500, 1680],
        #             [6500, 1300, 6750, 1650],
        #             [6860, 1330, 7000, 1650]]

        list_roi = [[5850,  880, 6200, 1300],
                    [6340, 900, 6600, 1250],
                    [6700, 900, 6940, 1230],
                    [7060, 950, 7200, 1230]]

        noffset = 20

        for nindex in range(len(list_roi)):
            list_roi[nindex][1] += noffset
            list_roi[nindex][3] += noffset

        solveimg = copy.copy(srcimg)

        list_w = []

        list_h = []


        for roi in list_roi:

            solveimg, w, h = self._calboundrect(solveimg, roi)

            list_w.append(w)

            list_h.append(h)

        return solveimg, list_w, list_h


    def solvecolor(self, srcimg):
        # roi = [6150,  900, 6700, 1200]#xstart, ystart, xend, yend

        roi = [6150,  552, 7000, 843]#xstart, ystart, xend, yend


        targetimg = srcimg[roi[1]:roi[3], roi[0]:roi[2]]

        list_gray = []

        for i in range(3):

            result = cv2.mean(targetimg[:, :, i])

            list_gray.append(int(result[0]))

        cv2.rectangle(srcimg, (roi[0], roi[1]), (roi[2], roi[3]), (0, 255, 0), 3)

        return list_gray


    def solveimg(self, srcimg):

        solveimg, list_w, list_h = self.solvemul(srcimg)

        list_gray = self.solvecolor(solveimg)

        return solveimg, list_w, list_h, list_gray



class ShowCalibrateWidget(QtWidgets.QDialog):
    def __init__(self):
        super(ShowCalibrateWidget, self).__init__()
        self.horlayout = QtWidgets.QHBoxLayout(self)
        self.widget1 = QtWidgets.QWidget()
        self.h_gVShowRealImg = GraphicsView(self.widget1)
        self.view = pg.ViewBox(invertY=True, enableMenu=False)
        self.h_gVShowRealImg.setCentralItem(self.view)
        self.view.setAspectLocked(True)
        self.h_imgitem = pg.ImageItem()
        self.view.addItem(self.h_imgitem)

        self.label_NGOK = QtWidgets.QLabel()
        font = QtGui.QFont()
        font.setFamily("Arial")
        font.setPointSize(30)
        font.setBold(False)
        font.setWeight(50)
        self.label_NGOK.setFont(font)

        self.label_WORD = QtWidgets.QLabel()

        self.widget2 = QtWidgets.QWidget()
        self.verlayout = QtWidgets.QVBoxLayout(self.widget2)
        self.verlayout.addWidget(self.label_NGOK)
        self.verlayout.addWidget(self.label_WORD)
        self.pushbutton = QtWidgets.QPushButton()
        self.pushbutton.setText("上传MES首件数据")
        font = QtGui.QFont()
        font.setFamily("Arial")
        font.setPointSize(14)
        self.pushbutton.setFont(font)
        self.pushbutton.setMinimumSize(100, 100)
        self.verlayout.addWidget(self.pushbutton)
        self.verlayout.setStretch(0, 5)
        self.verlayout.setStretch(1, 4)
        self.verlayout.setStretch(2, 1)
        self.horlayout.addWidget(self.h_gVShowRealImg)
        self.horlayout.addWidget(self.widget2)
        self.horlayout.setStretch(0, 7)
        self.horlayout.setStretch(1, 3)
        self.setMinimumSize(1000, 800)
        self.setWindowTitle("标定结果")


    def setimg(self, img):
        self.h_imgitem.setImage(img)


    def settext(self, sword):
        self.label_WORD.setText(sword)

    def setcalibrastatus(self, sword):
        self.label_NGOK.setText(sword)



if __name__ == "__main__":

    App = QtWidgets.QApplication([])
    w = ShowCalibrateWidget()


    img1 = cv2.imread("d:\\0.bmp", 1)
    print(img1.shape)
    # img2 = cv2.imread("d:\\2.bmp", 1)
    # img3 = cv2.imread("d:\\3.bmp", 1)
    #
    A = FindEdgeToCalibrate()
    #
    solveimg, list_w, list_h, list_gray =  A.solveimg(img1)
    #
    #
    #
    w.setimg(solveimg)
    #
    # w.settext(str(sums))

    w.show()

    App.exec_()




