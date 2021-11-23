import cv2
import numpy as np



class CMergeImg(object):
    """
    拼接拍摄图像为全局大图
    """
    def __init__(self):
        super(CMergeImg, self).__init__()
        self.n_firstbuild_imgnum = 0#首次建模单列图像数量，采集到这么多则开始新一列采集
        self.n_curcol = 0
        self.n_curcol_imgnum = 0#当前列已是第几张
        self.bigimg = None

    def clear(self):
        self.n_curcol = 0
        self.n_curcol_imgnum = 0

    def initinfo(self, firstbuild_imgnum, nw, nh):
        self.n_firstbuild_imgnum = firstbuild_imgnum
        self.bigimg = np.zeros((nh, nw), np.uint8)

    def IncreaseCol(self):
        """
        切换下一列拼图
        :return:
        """
        self.n_curcol_imgnum = 0
        self.n_curcol += 1

    def IsFull(self):
        """
        确认当前列是否已满
        :return:
        """
        return self.n_curcol_imgnum == self.n_firstbuild_imgnum


    def merge(self, img):
        """
        图像是一帧一帧来的，但要注意拼接顺序
        A   B   C  ....
        A   B   C  ....
        .   .   .  ....
        .   .   .  ....
        :param img:
        :return:
        """
        if self.n_curcol_imgnum == self.n_firstbuild_imgnum:# 当前列已填满
            return False
        nH, nW, nC = img.shape
        self.bigimg[self.n_curcol_imgnum * nH : (self.n_curcol_imgnum + 1) * nH, self.n_curcol * nW:(self.n_curcol + 1) * nW] = img
        self.n_curcol_imgnum += 1


class CMergeImgToList(object):
    """
    将不同列图像进行拼接，每一列拼成一张，存储在list中
    """
    def __init__(self):
        super(CMergeImgToList, self).__init__()
        self.list_build_imgnum = []#建模单列图像数量，采集到这么多则开始新一列采集
        self.n_curcol = 0#当前第几列
        self.n_curcol_imgnum = 0#当前列种，已是第几张
        self.list_bigimg = []

    def clear(self):
        self.n_curcol = 0
        self.n_curcol_imgnum = 0
        self.list_bigimg = []

    def initinfo(self, list_build_imgnum, nw, list_h):
        self.list_build_imgnum = list_build_imgnum
        for nh in list_h:
            self.list_bigimg.append(np.zeros((nh, nw), np.uint8))

    def IncreaseCol(self):
        """
        切换下一列拼图
        :return:
        """
        self.n_curcol_imgnum = 0
        self.n_curcol += 1

    def IsFull(self):
        """
        确认当前列是否已满
        :return:
        """
        return self.n_curcol_imgnum == self.list_build_imgnum[self.n_curcol]

    def merge(self, img):
        """
        图像是一帧一帧来的，将A/B/C区分开放入list
        A   B   C  ....
        A   B   C  ....
        .   .   .  ....
        .   .   .  ....
        :param img:
        :return:
        """
        if self.n_curcol_imgnum == self.list_build_imgnum[self.n_curcol]:# 当前列已填满
            return False
        nH, nW, nC = img.shape
        self.list_bigimg[self.n_curcol][self.n_curcol_imgnum * nH : (self.n_curcol_imgnum + 1) * nH] = img
        self.n_curcol_imgnum += 1
