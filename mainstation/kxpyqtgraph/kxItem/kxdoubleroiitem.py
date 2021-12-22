# -*- coding: utf-8 -*-

import pyqtgraph as pg
from pyqtgraph.Qt import QtCore, QtGui
import numpy as np
import copy

# __all__ = ['KxDoubleRoiItem','KxRoiWithLineItem','ROIwithText']
FONT_SIZE = 8

class KxDoubleRoiItem(pg.QtGui.QGraphicsObject):

    sigposchanging =  QtCore.Signal(object)
    sigposchanged =  QtCore.Signal(object)

    def __init__(self, dict_pos=None, pen=(0,6), id=None, word="",**args):
        QtGui.QGraphicsObject.__init__(self)
        self.lines = []
        self.pen =pen
        self.id = id
        self.word = word
        self.roi_outside = None
        self.roi_inside = None

        if dict_pos is not None:
            self.dict_pos = dict_pos
        else:
            self.dict_pos = {"in":[10,10,30,30],"out":[0,0,40,40]}
            # [x0,y0,x1,y1]
        # self.dict_pos = dict_pos
        self.addSegment()
        # self.setdictpos(self.dict_pos)

    def updatedict_pos(self):
        out_pos = copy.deepcopy(self.roi_outside.pos())
        out_size = copy.deepcopy(self.roi_outside.size())

        in_pos = copy.deepcopy(self.roi_inside.pos())
        in_size = copy.deepcopy(self.roi_inside.size())

        self.dict_pos["out"] = [int(out_pos[0]),int(out_pos[1]),int(out_pos[0])+int(out_size[0]), \
                                 int(out_pos[1]) + int(out_size[1])]
        self.dict_pos["in"] = [int(in_pos[0]),int(in_pos[1]),int(in_pos[0])+int(in_size[0]), \
                                 int(in_pos[1]) + int(in_size[1])]

    def setdictpos(self,dict_pos):
        pos_out = []
        pos_in = []
        if "out" in dict_pos:
            pos_out = dict_pos["out"]
        if "in" in dict_pos:
            pos_in = dict_pos["in"]

        self.roi_inside.sigRegionChanged.disconnect(self.roi_inside_ChangingEvent)
        self.roi_outside.sigRegionChanged.disconnect(self.roi_outside_ChangingEvent)

        if len(pos_out) == 4:
            pos = pos_out[:2]
            w = pos_out[2] - pos_out[0]
            h = pos_out[3] - pos_out[1]
            pos = QtCore.QPointF(pos[0],pos[1])
            self.roi_outside.setPos(pos)
            self.roi_outside.setSize([w,h])
            self.dict_pos["out"] = pos_out


            # pos_roi不在中心或pos_in==[]使用默认值
            if pos_in == [] \
                    or (pos_out[0] <= pos_in[0]) or (pos_out[1] <= pos_in[1]) \
                    or (pos_out[2] >= pos_in[2]) or (pos_out[3] >= pos_in[3]) \
                    or ((pos_out[0] + pos_out[2]) != (pos_in[0] + pos_in[2])) \
                    or ((pos_out[1] + pos_out[3]) != (pos_in[1] + pos_in[3])):
                w = pos_out[2] - pos_out[0]
                h = pos_out[3] - pos_out[1]
                pos_in = [0,0,0,0]
                pos_in[0] = pos_out[0] + w / 4
                pos_in[1] = pos_out[1] + h / 4
                pos_in[2] = pos_out[2] - w / 4
                pos_in[3] = pos_out[3] - h / 4
                pos = pos_in[:2]

            w = pos_in[2] - pos_in[0]
            h = pos_in[3] - pos_in[1]
            pos = QtCore.QPointF(pos[0], pos[1])
            self.roi_inside.setPos(self.mapToParent(pos))
            self.roi_inside.setSize([w,h])
            self.dict_pos["in"] = pos_in

        self.roi_outside_startinfo = [self.roi_outside.pos(),self.roi_outside.size()] #pos,size
        self.roi_inside_startinfo = [self.roi_inside.pos(),self.roi_inside.size()] #pos,size

        self.roi_inside.sigRegionChanged.connect(self.roi_inside_ChangingEvent)
        self.roi_outside.sigRegionChanged.connect(self.roi_outside_ChangingEvent)

    def addSegment(self):
        pos_out = self.str2list(self.dict_pos["out"])
        pos = pos_out[:2]
        w = pos_out[2] - pos_out[0]
        h = pos_out[3] - pos_out[1]
        size = [w, h]
        self.roi_outside = ROIwithText(pos, size,word=self.word, parent=self, pen=self.pen,)
        self.roi_outside.translatable = True
        # self.roi_outside.handleSize = 50
        self.roi_outside.addScaleHandle([0, 0], [1, 1])
        self.roi_outside.addScaleHandle([0, 1], [1, 0])
        self.roi_outside.addScaleHandle([1, 0], [0, 1])
        self.roi_outside.addScaleHandle([1, 1], [0, 0])

        pos_in = self.str2list(self.dict_pos["in"])
        pos = pos_in[:2]
        w = pos_in[2] - pos_in[0]
        h = pos_in[3] - pos_in[1]
        size = [w, h]
        self.roi_inside = ROIwithText(pos,size,parent=self, pen=self.pen,bool_cross=True)
        self.roi_inside.translatable = False
        # self.roi_inside.handleSize = 50
        self.roi_inside.addScaleHandle([0, 0], [1, 1])
        self.roi_inside.addScaleHandle([0, 1], [1, 0])
        self.roi_inside.addScaleHandle([1, 0], [0, 1])
        self.roi_inside.addScaleHandle([1, 1], [0, 0])

        self.lines.append(self.roi_inside)
        self.lines.append(self.roi_outside)

        self.roi_outside_startinfo = [self.roi_outside.pos(),self.roi_outside.size()] #pos,size
        self.roi_inside_startinfo = [self.roi_inside.pos(),self.roi_inside.size()] #pos,size

        self.roi_outside.sigRegionChanged.connect(self.roi_outside_ChangingEvent)
        self.roi_inside.sigRegionChanged.connect(self.roi_inside_ChangingEvent)
        self.roi_outside.sigRegionChangeStarted.connect(self.roi_outside_ChangedstartEvent)
        # self.roi_inside.sigRegionChanged.connect(self.roi_outside_ChangedstartEvent)
        # self.roi_inside.sigRegionChanged.connect(self.roi_inside_ChangedEvent)
        self.roi_outside.sigRegionChangeFinished.connect(self.RegionChangeFinished)
        self.roi_inside.sigRegionChangeFinished.connect(self.RegionChangeFinished)
        # self.roi_inside.sigRegionChanged.connect(self.roi_outside_ChangedEvent)
        self.roi_outside_ChangingEvent()


    def RegionChangeFinished(self):
        self.sigposchanged.emit(self)

    def roi_inside_ChangingEvent(self):
        # self.roi_inside.setPos([0,0])
        pos = copy.deepcopy(self.roi_inside.pos())
        size = copy.deepcopy(self.roi_inside.size())

        out_pos = copy.deepcopy(self.roi_outside.pos())
        out_size = copy.deepcopy(self.roi_outside.size())

        x = pos[0]
        y = pos[1]
        w = size[0]
        h = size[1]
        out_x = out_pos[0]
        out_y = out_pos[1]
        out_w = out_size[0]
        out_h = out_size[1]
        new_pos = [out_x+(out_w-w)/2, out_y+(out_h-h)/2]
        # new_size = [w, h]
        self.roi_inside.sigRegionChanged.disconnect(self.roi_inside_ChangingEvent)
        self.roi_inside.setPos(new_pos)
        self.roi_inside.sigRegionChanged.connect(self.roi_inside_ChangingEvent)
        self.updatedict_pos()
        self.sigposchanging.emit(self)

    def roi_outside_ChangedstartEvent(self):
        self.roi_outside_startinfo = copy.deepcopy([self.roi_outside.pos(),self.roi_outside.size()]) #pos,size
        self.roi_inside_startinfo = copy.deepcopy([self.roi_inside.pos(),self.roi_inside.size()]) #pos,size


    def roi_outside_ChangingEvent(self):
        self.roi_inside.sigRegionChanged.disconnect(self.roi_inside_ChangingEvent)

        roi_outside_pos = copy.deepcopy(self.roi_outside.pos())
        roi_outside_size = copy.deepcopy(self.roi_outside.size())

        maxbound = QtCore.QRectF(roi_outside_pos[0],roi_outside_pos[1], \
                                 roi_outside_size[0],roi_outside_size[1])

        self.roi_inside.maxBounds = maxbound

        roi_inside_startsize = self.roi_inside_startinfo[1]

        roi_inside_size = copy.deepcopy(self.roi_inside.size())
        time_w = roi_outside_size[0]/self.roi_outside_startinfo[1][0]
        time_h = roi_outside_size[1]/self.roi_outside_startinfo[1][1]

        delt_x_start = self.roi_inside_startinfo[0][0] - self.roi_outside_startinfo[0][0]
        delt_y_start = self.roi_inside_startinfo[0][1] - self.roi_outside_startinfo[0][1]

        try:
            pos_new_x = delt_x_start /roi_inside_startsize[0]*roi_inside_size[0]+roi_outside_pos[0]
            pos_new_y = delt_y_start /roi_inside_startsize[1]*roi_inside_size[1]+roi_outside_pos[1]
        except ZeroDivisionError:
            # pos_new_x = delt_x_start  + roi_outside_pos[0]
            # pos_new_y = delt_y_start  + roi_outside_pos[1]
            pos_new_x = roi_outside_pos[0] + roi_outside_size[0]/2
            pos_new_y = roi_outside_pos[1] + roi_outside_size[1]/2
            roi_inside_startsize = [roi_outside_size[0]/2, roi_outside_size[1]/2]
            time_w = 1
            time_h = 1


        new_point = QtCore.QPointF(pos_new_x,pos_new_y)
        # self.roi_inside.setPos(roi_inside_startpos+delt_pos_in*[time_w,time_h])
        self.roi_inside.setPos(self.mapToParent(new_point))
        self.roi_inside.setSize(roi_inside_startsize)
        # if delt_pos == QtCore.QPointF(0,0):
        #     self.roi_inside.scale([time_w,time_h],[0,0])
        self.roi_inside.scale([time_w,time_h],[0,0])
        self.sigposchanging.emit(self)

        self.roi_inside.sigRegionChanged.connect(self.roi_inside_ChangingEvent)
        self.updatedict_pos()
        self.sigposchanging.emit(self)


    def paint(self, *args):
        pass
    # def paint(self, p, opt, widget):
    #     # p.save()0
    #     # Note: don't use self.boundingRect here, because subclasses may need to redefine it.
    #
    #     r = QtCore.QRectF(self.h_insideroi.state['pos'][0], self.h_insideroi.state['pos'][1], self.h_insideroi.state['size'][0], self.h_insideroi.state['size'][1]).normalized()
    #     p.setRenderHint(QtGui.QPainter.Antialiasing)
    #     p.setPen(self.currentPen)
    #     # p.translate(r.left(), r.top())
    #     # p.scale(r.width(), r.height())
    #     p.drawRect(r)
    #
    #     r = QtCore.QRectF(0, 0, self.state['size'][0], self.state['size'][1]).normalized()
    #     p.setRenderHint(QtGui.QPainter.Antialiasing)
    #     p.setPen(self.currentPen)
    #     p.translate(r.left(), r.top())
    #     # p.scale(r.width(), r.height())
    #     p.drawRect(r)

    def boundingRect(self):
        return QtCore.QRectF()

    def get_list_pos(self):
        """
        用于获取位置信息,先是外面roi的坐标,后是里面roi坐标
        坐标均为左上角和右下角坐标,返回整型列表
        :return: 
        """
        return self.dict_pos["out"] + self.dict_pos["in"]

    def get_str_pos(self):
        '''
        用于获取位置信息,与get_list_pos一致,返回字符
        :return: 
        '''
        list_pos = self.get_list_pos()
        list_pos = [str(i) for i in list_pos ]
        str_pos = ','.join(list_pos)
        return  str_pos

    def set_list_pos(self, list_pos):
        '''
        设置位置坐标,可以是列表也可以是字符串
        :param list_pos: 
        :return: 
        '''
        if isinstance(list_pos, str) :
            self.set_str_pos(list_pos)
        elif isinstance(list_pos, list):
            dict_pos = {"out": list_pos[:4], "in": list_pos[4:]}
            self.setdictpos(dict_pos)
        elif isinstance(list_pos, dict):
            self.setdictpos(list_pos)
        else:
            pass

    def set_str_pos(self, str):
        list_pos = str.split(",")
        list_pos = [int(i) for i in list_pos]
        self.set_list_pos(list_pos)

    def str2list(self,list_pos):
        if isinstance(list_pos, str):
            list_pos = list_pos.split(",")
            list_pos = [int(i) for i in list_pos]
            return list_pos
        else:
            return list_pos


class KxRoiWithLineItem(pg.ROI):

    sigposchanging =  QtCore.Signal(object)
    sigposchanged =  QtCore.Signal(object)

    def __init__(self, dict_pos=None, pen=(0,6), id=None, word="", bool_vertical=False,*args, **kwargs):
        if dict_pos is None:
            self.dict_pos = {"out":[0,0,40,40]}
        else:
            self.dict_pos =dict_pos
        out_pos = self.dict_pos["out"]
        pos = [out_pos[0], out_pos[1]]
        size = [out_pos[2] - out_pos[0], out_pos[3] - out_pos[1]]
        self.pen = pen
        super(KxRoiWithLineItem, self).__init__(pos,size, pen=self.pen,*args, **kwargs)
        self.translatable = True
        self.addScaleHandle([0, 0], [1, 1])
        self.addScaleHandle([0, 1], [1, 0])
        self.addScaleHandle([1, 0], [0, 1])
        self.addScaleHandle([1, 1], [0, 0])
        self.bool_vertical = bool_vertical
        self.word = word
        self.id = id

        self.sigRegionChangeFinished.connect(self.sigposchanged)
        self.sigRegionChanged.connect(self.sigposchanging)
        self.sigRegionChanged.connect(self.updatedict_pos)
        self.init_update_dict()

    def updatedict_pos(self, pos_in=None):
        if pos_in is not list:
            pos_in = None#连接信号sigRegionChanged传入的对象进行屏蔽
        out_pos = copy.deepcopy(self.pos())
        out_size = copy.deepcopy(self.size())

        self.dict_pos["out"] = [int(out_pos[0]),int(out_pos[1]),int(out_pos[0])+int(out_size[0]), \
                                 int(out_pos[1]) + int(out_size[1])]
        if pos_in is None:
            n_scale = 3
            x1 = int(self.state['pos'][0])
            y1 = int(self.state['pos'][1])
            w = int(self.state['size'][0])
            h = int(self.state['size'][1])
            if self.bool_vertical is False:
                x0 = x1 - int(w / n_scale)
                y = y1 + int(h / 2)
                x2 = x1 + w + int(w / n_scale)
                pos_in = [x0, y, x2, y]
            else:
                x = x1 + int(w / 2)
                y0 = y1 + h + int(h / n_scale)
                y2 = y1 - int(h / n_scale)
                pos_in = [x, y0, x, y2]
        self.dict_pos["in"] = pos_in

    def boundingRect(self):
        n_size = min(self.state['size'][0], self.state['size'][1]) / 10.0
        if n_size < FONT_SIZE:
            n_size = FONT_SIZE
        if self.bool_vertical is False:
            return QtCore.QRectF(-self.state['size'][0]/3, -n_size*2, self.state['size'][0]*5/3, self.state['size'][1]*9/8+n_size*2).normalized()
        else:

            return QtCore.QRectF(0, -self.state['size'][1]/3-n_size*2, self.state['size'][0] , self.state['size'][1]*5/3+n_size*2).normalized()
        # return QtCore.QRectF(-self.state['size'][0], -self.state['size'][1], self.state['size'][0]*3, self.state['size'][1]*3)

    def setdictpos(self,dict_pos):
        pos_out = []

        if "out" in dict_pos:
            pos_out = dict_pos["out"]

        if len(pos_out) == 4:
            pos = pos_out[:2]
            w = pos_out[2] - pos_out[0]
            h = pos_out[3] - pos_out[1]
            pos = QtCore.QPointF(pos[0],pos[1])
            self.setPos(pos)
            self.setSize([w,h])
            self.dict_pos["out"] = pos_out

    def paint(self, p, opt, widget):
        # p.save()0
        # Note: don't use self.boundingRect here, because subclasses may need to redefine it.

        # r = QtCore.QRectF(self.h_insideroi.state['pos'][0], self.h_insideroi.state['pos'][1], self.h_insideroi.state['size'][0], self.h_insideroi.state['size'][1]).normalized()
        # p.setRenderHint(QtGui.QPainter.Antialiasing)
        # p.setPen(self.currentPen)
        # # p.translate(r.left(), r.top())
        # # p.scale(r.width(), r.height())
        # p.drawRect(r)
        r = QtCore.QRectF(0, 0, self.state['size'][0], self.state['size'][1]).normalized()
        p.setRenderHint(QtGui.QPainter.Antialiasing)
        p.setPen(self.currentPen)

        n_scale = 3
        x1 = int(self.state['pos'][0])
        y1 = int(self.state['pos'][1])
        w = int(self.state['size'][0])
        h = int(self.state['size'][1])
        if self.bool_vertical is False:
            x0 = x1 - int(w/n_scale)
            y = y1 + int(h/2)
            x2 = x1 + w + int(w/n_scale)
            point_start = self.mapFromParent(QtCore.QPointF(x0,y))
            point_end = self.mapFromParent(QtCore.QPointF(x2,y))
            # self.updatedict_pos([x0,y,x2,y])
        else:
            x = x1 + int(w/2)
            y0 = y1 + h + int(h/n_scale)
            y2 = y1 - int(h/n_scale)
            point_start = self.mapFromParent(QtCore.QPointF(x, y0))
            point_end = self.mapFromParent(QtCore.QPointF(x, y2))
            # self.updatedict_pos([x, y0,x, y2])

        # p.scale(3, 3)
        if self.word != None:
            s_font1 = QtGui.QFont()
            s_font1.setWeight(90)
            n_size = min(self.state['size'][0], self.state['size'][1])/10.0
            if n_size < FONT_SIZE:
                n_size = FONT_SIZE
            s_font1.setPointSize(n_size)
            p.setFont(s_font1)
            n_add = 0
            if self.state['size'][0]< len(self.word)*n_size:
                n_add = len(self.word)*n_size - self.state['size'][0]
            # font_r = QtCore.QRectF(0, -n_size*3/2, self.state['size'][0] + n_add, self.state['size'][1]).normalized()
            font_r = QtCore.QRectF(0, -n_size*1.5, self.state['size'][0]+ n_add, self.state['size'][1]+n_size*1.5).normalized()
            p.drawText(font_r, QtCore.Qt.AlignLeft, self.word)
        p.drawLine(point_start, point_end)

        p.translate(r.left(), r.top())
        # p.scale(r.width(), r.height())
        p.drawRect(r)

    def init_update_dict(self):
        n_scale = 3
        x1 = int(self.state['pos'][0])
        y1 = int(self.state['pos'][1])
        w = int(self.state['size'][0])
        h = int(self.state['size'][1])
        if self.bool_vertical is False:
            x0 = x1 - int(w/n_scale)
            y = y1 + int(h/2)
            x2 = x1 + w + int(w/n_scale)
            point_start = self.mapFromParent(QtCore.QPointF(x0,y))
            point_end = self.mapFromParent(QtCore.QPointF(x2,y))
            self.updatedict_pos([x0,y,x2,y])
        else:
            x = x1 + int(w/2)
            y0 = y1 + h + int(h/n_scale)
            y2 = y1 - int(h/n_scale)
            point_start = self.mapFromParent(QtCore.QPointF(x, y0))
            point_end = self.mapFromParent(QtCore.QPointF(x, y2))
            self.updatedict_pos([x, y0,x, y2])

    def get_list_pos(self):
        """
        用于获取位置信息,先是外面roi的坐标,后是里面roi坐标
        坐标均为左上角和右下角坐标,返回整型列表
        :return: 
        """
        return self.dict_pos["out"] + self.dict_pos["in"]

    def get_str_pos(self):
        '''
        用于获取位置信息,与get_list_pos一致,返回字符
        :return: 
        '''
        list_pos = self.get_list_pos()
        list_pos = [str(i) for i in list_pos ]
        str_pos = ','.join(list_pos)
        return  str_pos

    def set_list_pos(self, list_pos):
        '''
        设置位置坐标,可以是列表也可以是字符串
        :param list_pos: 
        :return: 
        '''
        if isinstance(list_pos, str) :
            self.set_str_pos(list_pos)
        elif isinstance(list_pos, list):
            dict_pos = {"out": list_pos[:4], "in": list_pos[4:]}
            self.setdictpos(dict_pos)
        elif isinstance(list_pos, dict):
            self.setdictpos(list_pos)
        else:
            pass

    def set_str_pos(self, str):
        list_pos = str.split(",")
        list_pos = [int(i) for i in list_pos]
        self.set_list_pos(list_pos)

    def str2list(self,list_pos):
        if isinstance(list_pos, str):
            list_pos = list_pos.split(",")
            list_pos = [int(i) for i in list_pos]
            return list_pos
        else:
            return list_pos

    def getsize(self):
        return  self.size()


class ROIwithText(pg.ROI):

    sigposchanged = QtCore.Signal(object)

    def __init__(self,pos=[0,0],size=[100,100], pen=(0,6),word="",scaleable=False,bool_cross=False,*args, **kwargs):
        self.dict_pos ={}
        self.pen = pen
        super(ROIwithText, self).__init__(pos,size, pen=self.pen,*args, **kwargs)
        self.word = word
        self.bool_cross = bool_cross
        self.sigRegionChangeFinished.connect(self.sigposchanged)
        self.setAcceptedMouseButtons(QtCore.Qt.LeftButton)
        self.sigClicked.connect(self.slotRoiClicked)
        
        self.scaleable = scaleable
        self.handleList = []
        self.fontRect = QtCore.QRectF(0, 0, 0, 0)

#         if scaleable is True:
#             self.addScaleHandle([0, 0], [1, 1])
#             self.addScaleHandle([0, 1], [1, 0])
#             self.addScaleHandle([1, 0], [0, 1])
#             self.addScaleHandle([1, 1], [0, 0])
            
    def slotRoiClicked(self, roi, ev):
        expand = 25
        for item in self.getViewBox().addedItems:
            if isinstance(item, ROIwithText):
                item.removeAllHandles()
        
        if self.scaleable:
            if ev.pos().x() >= -expand and ev.pos().x() <= expand and ev.pos().y() >= 0 and ev.pos().y() <= self.state['size'][1]:
                self.handleList.append(self.addScaleHandle([0, 0], [1, 1]))#lt
                self.handleList.append(self.addScaleHandle([0, 1], [1, 0]))#lb
            elif ev.pos().x() >= self.state['size'][0] - expand and ev.pos().x() <= self.state['size'][0] + expand \
                 and ev.pos().y() >= 0 and ev.pos().y() <= self.state['size'][1]:
                self.handleList.append(self.addScaleHandle([1, 0], [0, 1]))
                self.handleList.append(self.addScaleHandle([1, 1], [0, 0]))#rb
            elif ev.pos().y() >= -expand and ev.pos().y() <= expand and ev.pos().x() >= 0 and ev.pos().x() <= self.state['size'][0]:
                self.handleList.append(self.addScaleHandle([0, 0], [1, 1]))
                self.handleList.append(self.addScaleHandle([1, 0], [0, 1]))
            elif ev.pos().y() >= self.state['size'][1] - expand and ev.pos().y() <= self.state['size'][1] + expand \
                 and ev.pos().x() >= 0 and ev.pos().x() <= self.state['size'][0]:
                self.handleList.append(self.addScaleHandle([0, 1], [1, 0]))
                self.handleList.append(self.addScaleHandle([1, 1], [0, 0]))
            elif super(ROIwithText, self).boundingRect().contains(ev.pos()):
                self.handleList.append(self.addScaleHandle([0, 0], [1, 1]))
                self.handleList.append(self.addScaleHandle([0, 1], [1, 0]))
                self.handleList.append(self.addScaleHandle([1, 0], [0, 1]))
                self.handleList.append(self.addScaleHandle([1, 1], [0, 0]))
            
    def removeAllHandles(self):
        if len(self.handleList) > 0:
            for handle in self.handleList:
                self.removeHandle(handle)
            self.handleList = []

    def boundingRect(self):
        if len(self.handleList) > 0:
            return super(ROIwithText, self).boundingRect().adjusted(-(self.handleList[0].boundingRect().width() / 2 + 1),
                                                                    -(self.handleList[0].boundingRect().height() / 2),
                                                                    self.handleList[0].boundingRect().width() / 2 + 1,
                                                                    self.handleList[0].boundingRect().height() / 2 + 1).united(self.fontRect.adjusted(-100, -100, 100, 100))
        else:
        #      return super(ROIwithText, self).boundingRect().united(self.fontRect.adjusted(-100, -100, 100, 100))
            return super(ROIwithText, self).boundingRect()


    def paint(self, p, opt, widget):

        r = QtCore.QRectF(0, 0, self.state['size'][0], self.state['size'][1]).normalized()
        p.setRenderHint(QtGui.QPainter.Antialiasing)
        self.currentPen.setWidthF(1.7)
        p.setPen(self.currentPen)

        if self.word != None:
            s_font1 = QtGui.QFont()
            s_font1.setWeight(90)
            n_size = min(self.state['size'][0], self.state['size'][1])/10.0
            if n_size < FONT_SIZE:
                n_size = FONT_SIZE
            n_size = 20
            s_font1.setPointSize(n_size)
            p.setFont(s_font1)
#             n_add = 0
#             if self.state['size'][0]< len(self.word)*n_size:
#                 n_add = len(self.word)*n_size - self.state['size'][0]
            self.fontRect = QtCore.QRectF(0, -n_size*1.5, max(self.state['size'][0], len(self.word)*n_size*1.5), n_size*1.5)
#             self.fontRect = QtCore.QRectF(0, -n_size*1.5, len(self.word)*n_size, n_size*1.5)
#             self.fontRect = QtCore.QRectF(0, -n_size*1.5, self.state['size'][0]+ n_add, self.state['size'][1]+n_size*1.5).normalized()
            p.drawText(self.fontRect, QtCore.Qt.AlignLeft, self.word)

        p.translate(r.left(), r.top())
        # p.scale(r.width(), r.height())
        
        p.drawRect(r)

        if self.bool_cross:
            radius = min(self.state['size'][0], self.state['size'][1])/4
            n_centre_x = self.state['size'][0]/2
            n_centre_y = self.state['size'][1]/2
            p.drawLine(QtCore.QPoint(n_centre_x, n_centre_y-radius),\
                       QtCore.QPoint(n_centre_x, n_centre_y + radius))
            p.drawLine(QtCore.QPoint(n_centre_x-radius, n_centre_y),
                       QtCore.QPoint(n_centre_x+radius, n_centre_y))

    def updatedict_pos(self):
        out_pos = copy.deepcopy(self.pos())
        out_size = copy.deepcopy(self.size())

        self.dict_pos["out"] = [int(out_pos[0]),int(out_pos[1]),int(out_pos[0])+int(out_size[0]), \
                                 int(out_pos[1]) + int(out_size[1])]

    def get_list_pos(self):
        """
        用于获取位置信息,先是外面roi的坐标,后是里面roi坐标
        坐标均为左上角和右下角坐标,返回整型列表
        :return: 
        """
        self.updatedict_pos()
        return self.dict_pos["out"]

    def get_str_pos(self):
        '''
        用于获取位置信息,与get_list_pos一致,返回字符
        :return: 
        '''
        list_pos = self.get_list_pos()
        list_pos = [str(i) for i in list_pos ]
        str_pos = ','.join(list_pos)
        return  str_pos

    def set_list_pos(self, list_pos):
        '''
        设置位置坐标,可以是列表也可以是字符串
        :param list_pos: 
        :return: 
        '''
        if isinstance(list_pos, str) :
            self.set_str_pos(list_pos)
        elif isinstance(list_pos, list):
            dict_pos = {"out": list_pos[:4]}
            self.setdictpos(dict_pos)
        elif isinstance(list_pos, dict):
            self.setdictpos(list_pos)
        else:
            pass

    def set_str_pos(self, str):
        list_pos = str.split(",")
        list_pos = [int(i) for i in list_pos]
        self.set_list_pos(list_pos)

    def str2list(self,list_pos):
        if isinstance(list_pos, str):
            list_pos = list_pos.split(",")
            list_pos = [int(i) for i in list_pos]
            return list_pos
        else:
            return list_pos

    def setdictpos(self,dict_pos):
        pos_out = []

        if "out" in dict_pos:
            pos_out = dict_pos["out"]

        if len(pos_out) == 4:
            pos = pos_out[:2]
            w = pos_out[2] - pos_out[0]
            h = pos_out[3] - pos_out[1]
            pos = QtCore.QPointF(pos[0],pos[1])
            self.setPos(pos)
            self.setSize([w,h])
            self.dict_pos["out"] = pos_out

def main():
    app = QtGui.QApplication([])
    w = pg.GraphicsWindow(size=(1000, 800), border=True)
    w3 = w.addLayout(row=1, col=0)
    v3 = w3.addViewBox(row=1, col=0, lockAspect=True,invertY=True)

    #r3a = KxDoubleRoiItem(dict_pos={'in': [13, 13, 100, 100], 'out': [10, 10, 120, 120]},word="你好")
    # r3a = KxDoubleRoiItem(word="qqqqq")
    # r3a = ROIwithText([0,0],[40,40],word="qqqqq")
    r3a = KxRoiWithLineItem(dict_pos={u'in': [1085, 202, 1377, 306], u'out': [10, 10, 120, 120]},word=u"你好", pen=(0, 9),bool_vertical=False)

    v3.addItem(r3a)

    app.exec_()

if __name__ == '__main__':

    main()