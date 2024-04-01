import cv2
import numpy as np



class FindEdgeToCalibrate(object):
    def __init__(self):
        super(FindEdgeToCalibrate, self).__init__()


    def solveimg(self, srcimg):
        roi = [5800,  1200, 6100, 1680]#x, y, xend, yend

        targetimg = srcimg[roi[1]:roi[3], roi[0]:roi[2], 0]

        ret, threshimg = cv2.threshold(targetimg, 40, 255, cv2.THRESH_BINARY)

        sums = (threshimg > 0).sum()

        #statusarray)

        print(sums)

        return sums

        #cv2.findContours(threshimg, )

        # h, w = threshimg.shape
        #
        # startx = 0
        #
        # endx = w / 2
        #
        # starty = int(h / 4 * 1)
        #
        # endy = int(h / 4 * 3)
        #
        # list_y = []


        # cv2.namedWindow("threshimg", 0)
        #
        # cv2.imshow("threshimg", threshimg)
        #
        # cv2.waitKey(0)



if __name__ == "__main__":
    img = cv2.imread("d:\\1.bmp", 1)

    A = FindEdgeToCalibrate()

    A.solveimg(img)