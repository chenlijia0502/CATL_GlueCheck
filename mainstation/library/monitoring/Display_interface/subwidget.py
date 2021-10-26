# -*- coding: utf-8 -*-
from pyqtgraph.Qt import QtGui, QtCore
from pyqtgraph.widgets.GraphicsView import GraphicsView
from numpy.random.mtrand import np

from PyQt5.QtCore import Qt
import pyqtgraph as pg

from pyqtgraph import functions as fn
from PyQt5 import QtWidgets

from PyQt5.QtGui import QAbstractItemView

pg.setConfigOptions(imageAxisOrder='row-major')  # 设置图片行读取


class DataMonitorWidget(QtGui.QWidget):
    '''
    显示数据统计:总数，好品数，坏品数，坏品率
    通过名字来修改对应的数值
    比如 ::
    self.h_datamonitor = DataMonitorWidget({"统计信息"=["总数", "好品", "坏品", "好品率"]})
    self.h_datamonitor = DataMonitorWidget({"统计":["总数", "好品", "坏品", "好品率"]})
    self.h_datamonitor.setvalue("总数", 1000)
    
    self.h_datamonitor.numcalc(self.dict_current)  #函数处理

    '''

    def __init__(self, dict_mydict=None):
        super(DataMonitorWidget, self).__init__()

        self.s_title = ""  # 用于存储标题
        self.list_name = []  # 用于存储对应名字
        self.dict_valuedict = {}  # 用于存储 名字-对应的实例
        self.dict_namedict = {}  # 用于存储 名字-对应的数值

        h_widget = self.makewidget(dict_mydict)

        h_layout = QtGui.QVBoxLayout()
        h_layout.setContentsMargins(0, 0, 0, 0)
        h_layout.setSpacing(2)
        h_layout.addWidget(h_widget)
        self.setLayout(h_layout)

    def makewidget(self, dict_mydict):
        assert len(dict_mydict) == 1, "DataMonitorWidget（dict_mydict）中dict_mydict要求只包含一对键值"
        for self.s_title, self.list_name in dict_mydict.items():
            pass
        #         print self.s_title, len(dict_mydict), self.list_name

        h_widget = QtGui.QWidget()
        h_grid = QtGui.QGridLayout()

        h_labeltitle = QtGui.QLabel(self.s_title)
        h_labeltitle.setAlignment(Qt.AlignCenter)
        h_labeltitle.setFont(QtGui.QFont("Roman times", 20, QtGui.QFont.Bold))  # 字体设置
        h_grid.addWidget(h_labeltitle, 0, 3, 1, 7)

        for n_count, s_name in enumerate(self.list_name):
            n_count += 1
            locals()['h_label' + str(n_count)] = QtGui.QLabel(s_name)  # 相当于h_label1 = QtGui.Qlabel(s_name),显示名字的控件
            locals()['h_label' + str(n_count)].setFont(QtGui.QFont("Roman times", 20, QtGui.QFont.Bold))  # 字体设置
            locals()['h_label' + str(n_count)].setAlignment(Qt.AlignVCenter | Qt.AlignHCenter)  # 数据水平中对齐
            h_grid.addWidget(locals()['h_label' + str(n_count)], n_count, 0, 1, 4)  # 布局
            locals()['h_label' + str(n_count) + '_value'] = QtGui.QLabel(str(None))  # 显示数据的控件
            locals()['h_label' + str(n_count) + '_value'].setAlignment(Qt.AlignVCenter | Qt.AlignHCenter)  # 数据水平中对齐
            locals()['h_label' + str(n_count) + '_value'].setFont(
                QtGui.QFont("Roman times", 20, QtGui.QFont.Bold))  # 字体设置
            h_grid.addWidget(locals()['h_label' + str(n_count) + '_value'], n_count, 4, 1, 10)  # 布局
            self.dict_valuedict[s_name] = locals()['h_label' + str(n_count) + '_value']  # 存储名字与对应的实例
            self.dict_namedict[s_name] = 0  # 存储名字与对应的数值

        h_widget.setLayout(h_grid)
        #         h_widget.setStyleSheet("border:1px solid black;border-radius:5px")
        return h_widget

    def setvalue(self, s_name, value):
        """
        设置值函数
        """
        h_instan = self.dict_valuedict[s_name]
        if isinstance(value, float):  # 如果是浮点数，转化为百分数显示
            h_instan.setText('%.2f%%' % (value * 100))
        else:
            h_instan.setText(str(value))

    def numcalc(self, quality):
        """
        传入字典运行函数
        """
        if quality == 0:
            self.dict_namedict[str(self.tr("Good"))] += 1
        else:
            self.dict_namedict[str(self.tr("Bad"))] += 1
        self.dict_namedict[str(self.tr("Total"))] = self.dict_namedict[str(self.tr("Good"))] + self.dict_namedict[
            str(self.tr("Bad"))]
        self.dict_namedict[str(self.tr("Ratio"))] = float(self.dict_namedict[str(self.tr("Good"))]) / float(
            self.dict_namedict[str(self.tr("Total"))])
        for s_name, value in self.dict_namedict.items():
            self.setvalue(s_name, value)

    def __getitem__(self, s_name):
        return self.dict_valuedict[s_name]

    def clear(self):
        for s_name in self.dict_namedict:
            self.dict_namedict[s_name] = 0
            self.setvalue(s_name, None)

    def getvalue(self):
        return [self.dict_namedict[str(self.tr("Good"))], self.dict_namedict[str(self.tr("Bad"))]]


class ResizeWidget(QtGui.QWidget):
    """
    重写resizeEvent函数，使标志在右上角保持
    """

    def __init__(self):
        super(ResizeWidget, self).__init__()

    def resizeEvent(self, *args, **kwargs):  # 重写 resizeEvent 保证 textItem 在右上角位置
        h_child = self.dict_variable["booltext"]
        h_child.setPos(self.width() - 87, 5)


class ImageDisplayWidget(ResizeWidget):
    '''
    显示单张图片
    最终调用self.h_imagedisplay.getdict(dict_data)
    
    sigroichosed = QtCore.pyqtSignal(int)  #roi被左键或右键选中信号
    sigslotlearn = QtCore.pyqtSignal(int)  #roi被右键菜单点击学习
    '''
    #     sigroichange = QtCore.pyqtSignal(object)
    sigroichosed = QtCore.pyqtSignal(int)
    sigslotlearn = QtCore.pyqtSignal(dict)

    def __init__(self):
        super(ImageDisplayWidget, self).__init__()

        self.bool_flag = None  # 用于和传入的布尔值对比
        self.s_imagedir = None  # 读取图片的地址
        self.h_imagearr = None  # 图片数组
        self.rois = []  # 存储roi参数
        self.dict_variable = {"viewbox": None, "imageitem": None, "booltext": None}  # booltext用于标定好坏的布尔文本，好是对应绿色
        self.bool_stopflag = False  # 标志位用来判定是否停止，用来对触发事件进行判断
        h_widget = self.makewidget()  # 窗体

        self.fileptr = None

        h_layout = QtGui.QHBoxLayout()
        h_layout.setContentsMargins(0, 0, 0, 0)
        h_layout.setSpacing(2)
        h_layout.addWidget(h_widget)
        self.setLayout(h_layout)

    def autorangeview(self):
        self.dict_variable["viewbox"].autoRange()

    def makewidget(self):
        #         h_widget = ResizeWidget()
        h_widget = QtGui.QWidget()
        h_layout = QtGui.QHBoxLayout()

        h_display = GraphicsView(h_widget)
        h_display.resize(400, 400)
        self.h_diaplaywidth = h_display.width()
        self.dict_variable["displaywidget"] = h_display
        h_vb = pg.ViewBox(invertY=True, enableMenu=False)
        # h_vb.setAspectLocked(True)
        self.dict_variable["viewbox"] = h_vb
        h_display.setCentralItem(h_vb)

        text = pg.TextItem(html='</span><br><span style="color: #FF0; font-size: 16pt;">NONE </span>',
                           anchor=(-0.3, 0.5), angle=0, border='w', fill=(0, 255, 0, 255))
        # fill=(0, 255, 0, 100)绿色 fill=(255, 0, 0, 255)红色
        h_display.addItem(text)
        text.setPos(h_display.width() - 87, 5)

        self.dict_variable["booltext"] = text
        h_layout.addWidget(h_display)
        h_widget.setLayout(h_layout)

        self.img = pg.ImageItem(self.h_imagearr)
        self.dict_variable["imageitem"] = self.img
        h_vb.addItem(self.img)

        return h_widget

    def setroi(self, rois, defectnames):
        """
        传入的 rois = [[[51.0, 63.0], [1702.0, 627.0]], None, None, None]
        detectnames = [[点数,能量],[],[],[]]
        
        返回包含四个list[imageitem, labeltext]的list给 ImageLisetWidget类
        """
        for roi in self.rois:
            roi.sigroichosed.disconnect(self.sigroichosed)
            roi.sigslotlearn.disconnect(self.sigslotlearn)

        self.rois = []
        for i, roi in enumerate(rois):
            if roi == None:
                break
            else:
                roiTmp = myRectROI(roi[0:2], roi[2:], pen=(0, 9), word=str(i), \
                                   n_index=i, stopflag=self.bool_stopflag, \
                                   movable=False, invertible=False)
                # roiTmp.addScaleHandle([1, 0.5], [0.5, 0.5])
                # roiTmp.addScaleHandle([0.5, 1], [0.5, 0,5])
                roiTmp.setmenuvisable(True)
                self.rois.append(roiTmp)
                roiTmp.sigroichosed.connect(self.sigroichosed)
                roiTmp.sigslotlearn.connect(self.sigslotlearn)
                #             roi.sigRegionChanged.connect(update)     #不实时更新roi，会卡顿
                self.dict_variable["viewbox"].addItem(roiTmp)

        def update(rois, defectnames):
            #             v1b.autoRange()
            list_return = []
            for i, roi in enumerate(rois):
                if roi == None:
                    break
                locals()["h_imageitemout" + str(i)] = pg.ImageItem()
                y = int(rois[i][1])
                x = int(rois[i][0])
                width = int(rois[i][2])
                height = int(rois[i][3])
                locals()["h_imageitemout" + str(i)].setImage(self.h_imagearr[x: x + height, y: y + width], \
                                                             autoLevels=False, levels=(0, self.h_imagearr.max()))
                list_return.append([])
                list_return[i].append(locals()["h_imageitemout" + str(i)])
                list_return[i].append(defectnames[i])
            return list_return

        list_return1 = update(rois, defectnames)
        return list_return1

    def setbooltext(self, bool_good):  # good -- 0
        """
        右上角标志设置
        """
        if self.bool_flag == bool_good:
            return
        elif bool_good == 0:
            self.bool_flag = 0
            self.dict_variable["booltext"].setHtml(
                '</span><br><span style="color: #FF0; font-size: 16pt;">GOOD </span>')
            self.dict_variable["booltext"].fill = fn.mkBrush((0, 255, 0, 100))
        elif bool_good == None:
            self.bool_flag = None
            self.dict_variable["booltext"].setHtml(
                '</span><br><span style="color: #FF0; font-size: 16pt;">NONE </span>')
            self.dict_variable["booltext"].fill = fn.mkBrush((0, 255, 0, 255))
        else:
            self.bool_flag = 1
            self.dict_variable["booltext"].setHtml(
                '</span><br><span style="color: #FF0; font-size: 16pt;">_NG_ </span>')
            self.dict_variable["booltext"].fill = fn.mkBrush((255, 0, 0, 255))

    def getdict(self,quality, image, dict_data):


        self.h_imagearr =image

        list_rois = []
        list_defect = []
        for i, feature in enumerate(dict_data):
            rois = feature.get("pos", (0, 0, 0, 0))
            # print (rois)
            list_rois.append(rois)
            #             list_defect.append([dict_featurediclist[i].get(u"点数"), dict_featurediclist[i].get(u"能量")])
            list_defect.append([feature.get("Dots", -1),
                                feature.get("Energy", -1)])

        list_return = self.setdata(quality, list_rois, list_defect)
        return list_return


    def setdata(self, qualitylevel, rois, list_defect):
        self.dict_variable["viewbox"].clear()
        self.dict_variable["viewbox"].addItem(self.dict_variable["imageitem"])
        self.dict_variable["imageitem"].setImage(self.h_imagearr, autoLevels=False)
        self.setbooltext(qualitylevel)  # 标定好坏
        list_return = self.setroi(rois, list_defect)
        return list_return

    def setstopflag(self, bool_stopflag):
        self.bool_stopflag = bool_stopflag
        for roi in self.rois:  # 设置菜单弹出
            roi.setmenuvisable(bool_stopflag)

    def clear(self):
        self.dict_variable["imageitem"].clear()
        self.dict_variable["viewbox"].clear()
        self.setbooltext(None)
        # self.fileptr = None
        # if self.fileptr is not None:
        #     self.fileptr.close()

class FeatureList(pg.TableWidget):
    def __init__(self, parent=None):
        super(FeatureList, self).__init__(parent)
        data = np.array([(None, None)], dtype=[('特征名', object), ('当前值 ', object)])
        # data = np.array([(None, None)], dtype=[(str(self.tr(u'特征名')), object), (str(self.tr(u'当前值 ')), object)])
        self.setData(data)
        self.setRowCount(0)
        self.setSelectionMode(QtGui.QAbstractItemView.SingleSelection)  # 只能选择单行
        self.setSelectionBehavior(QtGui.QAbstractItemView.SelectRows)  # 选中全行
        self.horizontalHeader().setStretchLastSection(True)  # 不留空后
        self.setAlternatingRowColors(True)  # 颜色交替
        self.setSortingEnabled(False)

        # hyh:record the current class
        self.h_curclass = None

    # def rowchosed(self, num):
    #     self.appendData(self.getarr(self.dict_featuredicList[num]))

    def appendclass(self, class_data):
        arr_np = self.getarr(class_data)
        self.appendData(arr_np)
        self.h_curclass = class_data        # hyh:record the current class

    def getarr(self, class_data):  # list_dict = [{},{},{},{}]
        list_return = []
        self.setRowCount(0)
        list_data = self.unpack(class_data)
        for key in list_data:
            list_return.append(tuple([QtCore.QString.fromUtf8(key[0]), str(key[1])]))
        return np.array(list_return)

    def unpack(self, class_data):
        list_data = []
        try:
            for degectmsg in class_data.hDefectMsg:
                for featurlist in degectmsg.hFeatureList:
                    list_data.append((featurlist.szFeatureName.decode('gbk'),featurlist.fFeatureVaule))
        except AttributeError:
            list_data.append("NULL","NULL")
        return list_data

    def showspecfeature(self, n_index):
        if self.h_curclass is not None:
            arr_np = self.getarr(self.h_curclass)[15*n_index : 15*n_index + 15]
            self.appendData(arr_np)

class DataListWidget(pg.TableWidget):
    '''
    显示报废列表 ::
        h_imagedisplay = DataListWidget()
        h_imagedisplay.setcolumnname([('缺陷名', object), ('Column 2', object), ('Column 3', object)]) 
        h_imagedisplay.addRow(('1','2','3'))
        
        Sigkeypress = QtCore.pyqtSignal()  #键盘控制列表上下移动信号
        siglearn = QtCore.pyqtSignal(int)  #右键学习信号 发送id号
    '''

    Sigkeypress = QtCore.pyqtSignal(int)  # 键盘控制列表上下移动信号
    siglearn = QtCore.pyqtSignal(int)

    def __init__(self, dict_save, dtypes=None):
        super(DataListWidget, self).__init__()
        self.contextMenu = QtGui.QMenu()  # 右键菜单重写
        #         self.contextMenu.addAction(u'学习').triggered.connect(self.learn)
        self.contextMenu.addAction(self.tr("Image learn")).triggered.connect(self.learn)

        self.setSelectionMode(QAbstractItemView.SingleSelection)  # 只能选择单行
        self.setSelectionBehavior(QAbstractItemView.SelectRows)  # 选中全行
        self.horizontalHeader().setStretchLastSection(True)  # 不留空后
        self.setAlternatingRowColors(True)  # 颜色交替
        self.dtypes = dtypes
        self.setcolumnname(dtypes)
        self.n_indexmax = 400  # 默认最大显示行数
        #         self.dict_save = {}  #以Id号作为键值，存储数据
        self.bool_stopflag = False  # 标志位用来判定是否停止，用来对触发事件进行判断
        self.setSortingEnabled(False)
        self.dict_save = dict_save

    def kxclear(self):
        # QtGui.QTableWidget.clear(self)
        self.setRowCount(0)

    def setindexmax(self, n_indexmax):
        self.n_indexmax = n_indexmax

    def contextMenuEvent(self, ev):
        if self.bool_stopflag is not False:
            if self.dict_save[self.selectedItems()[0].value]['QualityLevel'] == 1:
                self.contextMenu.popup(ev.globalPos())

    def setstopflag(self, bool_stopflag):
        self.bool_stopflag = bool_stopflag
        self.setSortingEnabled(bool_stopflag)  # 结束后允许排序

    def tableclickevent(self, dict_save):
        dict_return = dict_save[self.selectedItems()[0].value]
        return dict_return

    def setcolumnname(self, dtypes=None):
        if dtypes == None:
            dtypes = [('Column 1', object), ]
            data = np.array([('None',) * len(dtypes)], dtypes)
        else:
            #             data = np.array([('None',) * len(dtypes)], dtypes)
            list_data = []
            new_dtypes = []
            for i, dtype in enumerate(dtypes):
                #                 if isinstance(dtype[1], int):
                new_dtypes.append(tuple([str(dtype[0]), dtype[1]]))
                if dtype[1] == type(0):
                    list_data.append(0)
                else:
                    list_data.append('None')
        data = np.array([tuple(list_data), ], new_dtypes)
        self.clear()
        self.appendData(data)
        self.setRowCount(0)


    def addmyrow(self,id, dict_data):
        """
        将数据显示到对应的位置上，并加到数组中，实现指定数据一个循环
        """
        n_id = id
        if dict_data == None or dict_data == []:
            defectname = "OK"
        else:
            if "defect name" in dict_data[0]:
                defectname = dict_data[0]["defect name"]
            else:
                defectname = "defect error"


        list_showdata = [n_id + 1, defectname]# +1是为显示能从1开始
        if n_id >= self.n_indexmax:  # 设定字典400循环
            # 列表显示更新
            newrow = n_id % self.n_indexmax
            self.setRow(newrow, list_showdata)
            self.selectRow(newrow)
        else:
            self.setRow(n_id, list_showdata)
            self.selectRow(n_id)
        self.setFocus(Qt.MouseFocusReason)

    def myclear(self):
        self.clear()
        self.setcolumnname(self.dtypes)
        self.setstopflag(False)

    def keyPressEvent(self, ev):
        QtGui.QTableWidget.keyPressEvent(self, ev)
        ncurrow = self.currentRow()
        self.Sigkeypress.emit(ncurrow)

    def learn(self):
        self.siglearn.emit(self.selectedItems()[0].value)




class ImageListWidget(QtWidgets.QWidget):
    # """
    #     显示缺陷图片 根据image1~4访问对应的ImageListSubWidget实例::
    #     h_imagedisplay = ImageListWidget()
    #     h_imagedisplay["image4"].setimage({"label" :"no good", "image" : "C:\Users\PC\Desktop\image1.jpg"})
    # """


    def __init__(self):
        super(ImageListWidget, self).__init__()
        # self.dict_imagedict = {}  # 存储对应的image1~4对应的实例
        self.list_imagewidget = []
        h_widget = self.makewidget()

        h_layout = QtGui.QVBoxLayout()
        #         h_layout.setContentsMargins(0, 0, 0, 0)
        h_layout.setContentsMargins(0, 0, 0, 0)
        h_layout.setSpacing(0)
        h_layout.addWidget(h_widget)
        self.setLayout(h_layout)

    def makewidget(self, n_num=4):
        h_widget = QtGui.QWidget()
        h_grid = QtGui.QGridLayout()
        h_grid.setMargin(0)

        for n_count in range(n_num):
            self.list_imagewidget.append(ImageListSubWidget())
            h_grid.addWidget(self.list_imagewidget[n_count], n_count / 2, n_count % 2)

        h_widget.setLayout(h_grid)

        return h_widget

    def setdata(self, image, list_word, list_roi):
        """
        list_return 来自ImageDisplayWidget的setroi函数::
            list_return = [[imageitem, labeltext],[],[],[]] #代表四个窗体的数据  其中labeltext = [[点数,能量],[],[],[]]
        """
        # if list_return == None:
        #     return
        # if len(list_return) == 0:
        #     return
        # for n_count, list_one in enumerate(list_return):
        #     if len(list_one) == 0 or n_count > 3:
        #         return
        #     n_count += 1
        #     #             if n_count == 5:
        #     #                 break
        #     dict_temp = {"label": list_one[1], "image": list_one[0]}
        #     self.dict_imagedict["image" + str(n_count)].setimage(dict_temp)

        nmaxshow = min(len(list_roi), len(self.list_imagewidget))
        for nindex in range(nmaxshow):

            roi = list_roi[nindex]
            word = list_word[nindex]
            smallimg = image[roi[1]:roi[1] + roi[3], roi[0]:roi[0]+roi[2]]
            self.list_imagewidget[nindex].setimage(word, smallimg)




    def clear(self, n_num=4):
        for widget in self.list_imagewidget:
            widget.clear()

        # for
        # for n_count in range(n_num):
        #     self.dict_imagedict['image' + str(n_count + 1)].clear()

    # def __getitem__(self, s_name):
    #     return self.dict_imagedict[s_name]


class ImageListSubWidget(QtWidgets.QWidget):
    '''
    缺陷图片的子窗口 通过::
        imagelist = ImageListSubWidget()
        imagelist["label"]和imagelist["image"]访问对应的Qlabel和ViewBox控件
    '''

    def __init__(self):
        super(ImageListSubWidget, self).__init__()
        self.s_label = ''  # 显示的文字
        self.s_imagepath = ''  # 图片路径
        self.dict_valuedict = {}  # 存储名字对应的控件 {“label”：，“iamge”：}
        self.arrimage = None
        h_widget = self.makewidget()
        h_layout = QtGui.QVBoxLayout()
        h_layout.setContentsMargins(0, 0, 0, 0)
        h_layout.addWidget(h_widget)
        self.setLayout(h_layout)

    def makewidget(self):
        h_widget = QtWidgets.QWidget()
        h_layout = QtWidgets.QVBoxLayout()
        self.h_label = QtWidgets.QLabel()

        self.h_display = GraphicsView(h_widget)
        self.view = pg.ViewBox(invertY=True, enableMenu=False)

        self.h_display.setCentralItem(self.view)
        self.h_imageitem = pg.ImageItem(self.arrimage)
        self.view.addItem(self.h_imageitem)
        self.view.setAspectLocked(True)

        # self.dict_valuedict["label"] = h_label
        # self.dict_valuedict["image"] = h_image

        h_layout.addWidget(self.h_display)
        h_layout.addWidget(self.h_label)
        h_widget.setLayout(h_layout)

        return h_widget

    def setimage(self, slabelword, smallimage):
        # for name in self.dict_valuedict.keys():
        #     assert name in dict_dict, "没有对应的%s变量" % name
        #         self.dict_valuedict["label"].setText("[" + u'点数' + "," + u'能量' +"]" + "=" +str(dict_dict["label"]))
        # self.h_label.setText("[" + self.tr("Dot")+ "," \
        #                                      + "Energy" + "]" + "=" + str(dict_dict["label"]))
        self.h_label.setText(slabelword)
        self.h_label.setAlignment(Qt.AlignVCenter | Qt.AlignHCenter)
        # self.dict_valuedict["image"].clear()
        #         self.dict_valuedict["image"].addItem(pg.ImageItem(np.array(Image.open(dict_dict["image"]))))
        # self.dict_valuedict["image"].addItem((dict_dict["image"]))
        self.h_imageitem.setImage(smallimage)
        # print (self.view.state['aspectLocked'] )
        # self.h_imageitem.setAspectLocked(True)
        # self.dict_valuedict["image"].setAspectLocked(True)

    def clear(self):
        self.h_imageitem.clear()
        self.h_label.setText("")
        # self.dict_valuedict["image"].clear()
        # self.dict_valuedict["label"].setText("")

    def __getitem__(self, s_name):
        return self.dict_valuedict[s_name]


class myRectROI(pg.ROI):
    """
    重写RectROI ,去掉改变大小 scaleHandle 添加右键菜单
    sigroichosed = QtCore.pyqtSignal(int)  #roi被左键或右键选中信号
    sigslotlearn = QtCore.pyqtSignal(int)  #roi被右键菜单点击学习  
    """
    sigroichosed = QtCore.pyqtSignal(int)
    sigslotlearn = QtCore.pyqtSignal(dict)

    def __init__(self, pos, size, n_index=0, word="", stopflag=False, centered=False, sideScalers=False, **args):
        # QtGui.QGraphicsRectItem.__init__(self, 0, 0, size[0], size[1])
        pg.ROI.__init__(self, pos, size, **args)
        if centered:
            center = [0.5, 0.5]
        else:
            center = [0, 0]
        self.pos=pos
        self.size=size
        self.stopflag = stopflag
        self.n_index = n_index
        self.word = word
        self.setPen((255, 0, 0), width=3)

    #         self.roimenu = QtGui.QMenu()  #右键菜单重写
    #         self.roimenu.addAction(u'学习').triggered.connect(self.learn)

    def mouseClickEvent(self, ev):
        if ev.button() == QtCore.Qt.RightButton and self.stopflag:
            self.raiseContextMenu(ev)
            self.sigroichosed.emit(self.n_index)
            ev.accept()
        elif ev.button() == QtCore.Qt.LeftButton and self.stopflag:

            self.sigroichosed.emit(self.n_index)
        else:
            ev.ignore()

    def raiseContextMenu(self, ev):

        menu = self.getMenu()
        #         menu = self.scene().addParentContextMenus(self, menu, ev)
        pos = ev.screenPos()
        menu.popup(QtCore.QPoint(pos.x(), pos.y()))

    def getMenu(self):
        if self.menu is None:
            self.menu = QtGui.QMenu()  # 菜单有export选项
            remAct = QtGui.QAction(u"缺陷学习", self.menu)
            # remAct = QtGui.QAction(unicode(self.tr("Defect learn"), 'utf-8'), self.menu)
            remAct.triggered.connect(self.learn)
            self.menu.addAction(remAct)
        return self.menu

    def setmenuvisable(self, stopflag):
        self.stopflag = stopflag

    def learn(self):
        self.sigslotlearn.emit({"宽":self.size[0],"高":self.size[1],"X":self.pos[0],"X":self.pos[1]})

    def paint(self, p, opt, widget):
        """
        左上角对应的数字
        """
        r = QtCore.QRectF(0, 0, self.state['size'][0], self.state['size'][1]).normalized()
        expand = 0
        #         rectText = QtCore.QRectF(r.x() + expand, r.y() + expand , r.width() + 2 * expand, r.height() + 2 * expand)
        rectText = QtCore.QRectF(-r.x() - 10, -r.y() - 10, 2 * 10, 2 * 10)
        p.setRenderHint(QtGui.QPainter.Antialiasing)
        p.setPen(self.currentPen)
        p.scale(3, 3)
        if self.word != None:
            p.drawText(rectText, QtCore.Qt.AlignLeft, self.word)
        p.translate(r.left(), r.top())
        p.scale(r.width() / 3, r.height() / 3)
        p.drawRect(0, 0, 1, 1)
