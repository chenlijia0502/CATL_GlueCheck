import cv2
import numpy as np



class CMergeImg(object):
    """
    拼接拍摄图像
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


