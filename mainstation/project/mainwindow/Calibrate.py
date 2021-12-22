import cv2
import numpy as np
from PyQt5 import Qt, QtWidgets, QtGui
from pyqtgraph import GraphicsView
import pyqtgraph as pg


class FindEdgeToCalibrate(object):
    def __init__(self):
        super(FindEdgeToCalibrate, self).__init__()


    def solveimg(self, srcimg):
        roi = [5800,  1200, 6100, 1680]#x, y, xend, yend

        targetimg = srcimg[roi[1]:roi[3], roi[0]:roi[2], 0]

        ret, threshimg = cv2.threshold(targetimg, 40, 255, cv2.THRESH_BINARY)

        sums = (threshimg > 0).sum()

        # image, contours, hierarchy = cv2.findContours(threshimg, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
        #
        # cv2.drawContours(targetimg, contours, -1, (0, 255, 0), thickness=3)

        h, w = threshimg.shape

        startx = 0

        endx = int(w / 2)

        starty = int(h / 4 * 1)

        endy = int(h / 4 * 3)

        list_x = []

        for y in range(starty, endy):
            for x in range(startx, endx):
                if threshimg[y][x] > 0:
                    list_x.append(x)
                    break

        naverage_left = int(np.sum(np.array(list_x) / len(list_x)))

        startx = int(w / 2)

        endx = w - 1

        list_x = []

        for y in range(starty, endy):
            for x in range(endx, startx, -1):
                if threshimg[y][x] > 0:
                    list_x.append(x)
                    break

        naverage_right = int(np.sum(np.array(list_x) / len(list_x)))

        startx = int(w / 4 * 1)

        endx = int(w / 4 * 3)

        starty = 0

        endy = int(h / 2)

        list_y = []

        for x in range(startx, endx):
            for y in range(starty, endy):
                if threshimg[y][x] > 0:
                    list_y.append(y)
                    break

        naverage_top = int(np.sum(np.array(list_y) / len(list_y)))

        startx = int(w / 4 * 1)

        endx = int(w / 4 * 3)

        starty = int(h / 2)

        endy = h - 1

        list_y = []

        for x in range(startx, endx):
            for y in range(endy, starty, -1):
                if threshimg[y][x] > 0:
                    list_y.append(y)
                    break

        naverage_bottom = int(np.sum(np.array(list_y) / len(list_y)))

        # print (naverage_left, naverage_right, naverage_right - naverage_left + 1)
        #
        # print (naverage_top, naverage_bottom, naverage_bottom - naverage_top + 1)
        # threshimg[:, naverage_left:naverage_left+1] = [100]
        # threshimg[:, naverage_right:naverage_right+1] = [100]
        #
        # threshimg[naverage_top:naverage_top+1, :] =  [100]
        # threshimg[naverage_bottom:naverage_bottom+1, :] = [100]
        #
        #
        # cv2.namedWindow("threshimg", 0)
        #
        # cv2.imshow("threshimg", threshimg)
        #
        # cv2.waitKey(0)

        naverage_left += roi[0]

        naverage_right += roi[0]

        naverage_top += roi[1]

        naverage_bottom += roi[1]

        cv2.line(srcimg, (naverage_left, naverage_top), (naverage_right, naverage_top), (0, 0, 255), 3)

        cv2.line(srcimg, (naverage_left, naverage_top), (naverage_left, naverage_bottom), (0, 0, 255), 3)

        cv2.line(srcimg, (naverage_left, naverage_bottom), (naverage_right, naverage_bottom), (0, 0, 255), 3)

        cv2.line(srcimg, (naverage_right, naverage_bottom), (naverage_right, naverage_top), (0, 0, 255), 3)

        return srcimg, sums, naverage_right - naverage_left + 1, naverage_bottom - naverage_top + 1


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


    img1 = cv2.imread("d:\\1.bmp", 1)
    img2 = cv2.imread("d:\\2.bmp", 1)
    img3 = cv2.imread("d:\\3.bmp", 1)

    A = FindEdgeToCalibrate()

    img, sums =  A.solveimg(img1)



    w.setimg(img1)

    w.settext(str(sums))

    w.show()

    App.exec_()




