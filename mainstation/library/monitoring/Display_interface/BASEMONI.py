#coding:utf-8

class BASEMONI(object):
    def __init__(self):
        pass

    def show(self, image, list_dictfeature, *args):
        """

        :param image:               输入图像
        :param list_dictfeature:    列表-字典，每个字典代表一个缺陷的特征参数，如下
        exp:[{"缺陷名":"污渍", "点数": 100, "能量": 100, "缺陷位置": [x, y, w, h], .......}, {...}....]
        :param *args:               继承该类的界面会使用到的参数，根据各个界面不同使用
        :return:
        """

    def clear(self):
        """清除本轮数据"""
        pass