#coding:utf-8


class SearchLayoutKernel(object):
    def __init__(self):
        pass

    def search(self, srcimg):
        """
        进行搜索定位核
        Parameters
        ----------
        srcimg          :一个版面的图

        Returns         :返回的信息包含定位核坐标，以及相关的一些参数、标志位，这些参数是可以辅助你完成在一张无限长的图
                         里分出每个版周的图片（这些参数可以包括相似定位核数量、搜索方向以及搜索范围等）

        -------

        """