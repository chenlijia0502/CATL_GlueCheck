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



class FindEdgeToCalibrateNew(object):
    _OFFSET_X = 1200
    _OFFSET_Y = 200
    _WIDTH = 1100
    _HEIGHT = 1500
    _BLOCKNUM = 4
    def __init__(self):
        super(FindEdgeToCalibrateNew, self).__init__()


    def solveimg(self, srcimg):
        """
        程序设定的输入是一张绿色色卡 + 一个标定块。两者的相对位置固定。
        先提取出绿色的位置，再固定偏移，找到标定板的位置。
        """

        img1 = np.array(srcimg[:, :, 0], np.uint8)
        img2 = np.array(srcimg[:, :, 1], np.uint8)
        img3 = np.array(srcimg[:, :, 2], np.uint8)

        subimg1 = cv2.subtract(img2, img1)
        subimg2 = cv2.subtract(img2, img3)
        subimg = np.array(subimg1 / 2, np.uint8) + np.array(subimg2 / 2, np.uint8)
        ret, result = cv2.threshold(subimg, 200, 255, cv2.THRESH_OTSU)

        retval, labels, stats, centroids = cv2.connectedComponentsWithStats(result)

        list_gray = []
        list_h = []
        list_w = []

        if retval > 1:
            array_dots = stats[1:, 4]
            nmaxindex = np.argmax(array_dots) + 1
            pos = stats[nmaxindex, :]# x, y, w, h......
            x, y, w, h = pos[:4]

            targetimg = srcimg[y:y + h, x: x+w]

            cv2.rectangle(srcimg, (x, y), (x + w, y + h), (255, 0, 0), 3)
            list_gray = [int(np.mean(targetimg[:, :, 0])),
                   int(np.mean(targetimg[:, :, 1])), int(np.mean(targetimg[:, :, 2]))]

            # print ("识别到的色卡平均颜色: ", int(np.mean(targetimg[:, :, 0])),
            #        int(np.mean(targetimg[:, :, 1])), int(np.mean(targetimg[:, :, 2])))

            calibrate_x = max(0, x - self._OFFSET_X)
            calibrate_y = max(0, y - self._OFFSET_Y)

            calibrateblock = srcimg[calibrate_y:calibrate_y + self._HEIGHT, calibrate_x:calibrate_x + self._WIDTH]

            solveimg1 = calibrateblock[:, :, 0]

            ret, dst = cv2.threshold(solveimg1, 230, 255, cv2.THRESH_BINARY)

            retval, labels, stats, centroids = cv2.connectedComponentsWithStats(dst)

            array_alldots = stats[1:, 4]

            list_pos = []

            if len(array_alldots) >= self._BLOCKNUM:

                list_sortindex = sorted(range(len(array_alldots)), key=lambda k: array_alldots[k], reverse=True)[:self._BLOCKNUM]

                for nindex in list_sortindex:

                    list_pos.append(stats[nindex + 1, :4])

                for roi in list_pos:

                    list_w.append(roi[2])

                    list_h.append(roi[3])

                    cv2.rectangle(srcimg, (calibrate_x + roi[0], calibrate_y + roi[1]),
                                  (calibrate_x + roi[0] + roi[2], calibrate_y + roi[1] + roi[3]), (255, 0, 0), 3)

        return srcimg, list_w, list_h, list_gray



        #cv2.imwrite("d:\\result.bmp", result)






class ShowCalibrateWidget(QtWidgets.QDialog):
    """
    两个作用：
    （1）标定数据，跟参考数据进行比对
    （2）保存数据作为标准值
    """
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


    img1 = cv2.imread("d:\\img\\test.bmp", 1)

    A = FindEdgeToCalibrateNew()

    solveimg, list_w, list_h, list_gray = A.solveimg(img1)

    w.setimg(solveimg)
    s_word = "识别到的格子宽： " + str(list_w) + "\n\n识别到的格子高： " + str(list_h) + "\n\n识别到的标准色板灰度： " + str(list_gray)
    w.settext(s_word)

    w.show()

    App.exec_()

    # w = FindEdgeToCalibrateNew()
    # img = cv2.imread("D:\\img\\test.bmp", 1)
    # w.solveimg(img)




