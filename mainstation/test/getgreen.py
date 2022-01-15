import cv2
import numpy as np
import matplotlib.pyplot as plt

# 使用2g-r-b分离土壤与背景

#src = cv2.imread(r'E:\\项目图片\\1642081923\\原图\\16.bmp')


def getgreen(src):
    # 转换为浮点数进行计算
    fsrc = np.array(src, dtype=np.float32) / 255.0
    #fsrc = src
    (b, g, r) = cv2.split(fsrc)
    #gray = 2 * g - b - r
    gray = g - r

    # 求取最大值和最小值
    (minVal, maxVal, minLoc, maxLoc) = cv2.minMaxLoc(gray)
    print (minVal, maxVal)

    # 计算直方图
    hist = cv2.calcHist([gray], [0], None, [256], [minVal, maxVal])
    # plt.plot(hist)
    # plt.show()


    # 转换为u8类型，进行otsu二值化
    gray_u8 = np.array((gray - minVal) / (maxVal - minVal) * 255, dtype=np.uint8)# 归一化到0-255
    #
    #gray_u8 = gray

    #(thresh, bin_img) = cv2.threshold(gray_u8, 130, 255, cv2.THRESH_BINARY)
    (thresh, bin_img) = cv2.threshold(gray_u8, -1.0, 255, cv2.THRESH_OTSU)

    print (thresh)

    # kernel = np.ones((3, 3), np.uint8)
    # openimg = cv2.morphologyEx(gray_u8, cv2.MORPH_OPEN, kernel)
    # gray_u8 = openimg


    cv2.namedWindow("gray_u8", 0)
    cv2.namedWindow("bin_img", 0)
    cv2.namedWindow("color_img", 0)

    cv2.imshow('bin_img', bin_img)
    cv2.imshow('gray_u8', gray_u8)

    # 得到彩色的图像
    (b8, g8, r8) = cv2.split(src)
    color_img = cv2.merge([b8 & bin_img, g8 & bin_img, r8 & bin_img])
    cv2.imshow('color_img', color_img)

    cv2.waitKey(0)
    #cv2.destroyAllWindows()


for i in range(6):
    src = cv2.imdecode(np.fromfile("D:\\%d.bmp"%i, dtype=np.uint8), -1)
    getgreen(src)