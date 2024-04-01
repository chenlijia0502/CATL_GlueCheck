
import cv2

import numpy as np


basepath = "D:\\all\\15\\1\\"

baseimg = cv2.imread(basepath + "0.bmp", 1)

orisize = list(baseimg.shape[:2])

RESIZEFACTOR = 4

newsize = (int(orisize[1] / RESIZEFACTOR), int(orisize[0] / RESIZEFACTOR))# w h



def makebigimg(nstart, nend):

    nh = (nend - nstart) * newsize[1]

    nw = newsize[0]

    bigimg = np.zeros((nh, nw, 3), np.uint8)

    i = 0

    for nindex in range(nstart, nend):

        img = cv2.imread(basepath + "%d.bmp"%nindex, 1)

        resizeimg = cv2.resize(img, newsize)

        bigimg[i * newsize[1]:(i+1) * newsize[1]] = resizeimg

        i += 1

    return bigimg


img = makebigimg(12, 24)

newimg = cv2.flip(img, 0)

cv2.imwrite("d:2.bmp", newimg)


