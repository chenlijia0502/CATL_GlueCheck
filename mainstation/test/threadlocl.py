import os
import cv2


def _threadfun_loadimg2buildmodel(data):
    """
    线程，载入图片进行建模。图片名称有序列要求，从 0 开始不断迭代，且中间不能断，且总数量不能小于两倍sum(list_imgnum)
    Parameters
    ----------
    data    :[list_imgnum, path]
             list_imgnum 图像列表数量，代表没一列采集多少张图像
             path        模拟取图路径
    Returns
    -------

    """
    list_imgnum = data[0]
    path = data[1]
    if os.path.isdir(path):
        nmaxnum = len(os.listdir(path))
        nreadindex = 0#读取索引
        listimg = []
        for nimgnum in list_imgnum:
            for i in range(nimgnum):
                readpath = path + "\\" + str(nreadindex) + ".bmp"
                nreadindex += 1
                if os.path.isfile(readpath):
                    listimg.append(cv2.imread(readpath, 1))
            nreadindex += nimgnum # 跳到下一列
        if len(listimg) != nmaxnum:
            return # 返回错误
        else:
            pass
    else:
        return # 返回错误




_threadfun_loadimg2buildmodel([[1, 2, 3], "E:\\项目图片\\2022-1-15\\001PBB0A000002C1E0A00008\\原图\\"])