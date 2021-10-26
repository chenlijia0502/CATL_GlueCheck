# -*- coding: utf-8 -*-

import pyqtgraph as pg
from PyQt5 import QtGui,QtCore

__all__ = ['LinearRegionItem']

class KxLinearRegionItem(pg.LinearRegionItem):

    Vertical = 0
    Horizontal = 1
    def __init__(self,values=[0,1], orientation=None, brush=None, movable=True, bounds=None):
        pg.UIGraphicsItem.__init__(self)
        if orientation is None:
            orientation = KxLinearRegionItem.Vertical
        self.orientation = orientation
        self.bounds = QtCore.QRectF()
        self.blockLineSignal = False
        self.moving = False
        self.mouseHovering = False

        if orientation == KxLinearRegionItem.Horizontal:
            self.lines = [
                finiteline(QtCore.QPointF(0, values[0]), 0, movable=movable, bounds=bounds),
                finiteline(QtCore.QPointF(0, values[1]), 0, movable=movable, bounds=bounds)]
        elif orientation == KxLinearRegionItem.Vertical:
            self.lines = [
                finiteline(QtCore.QPointF(values[1], 0), 90, movable=movable, bounds=bounds),
                finiteline(QtCore.QPointF(values[0], 0), 90, movable=movable, bounds=bounds)]
        else:
            raise Exception('Orientation must be one of LinearRegionItem.Vertical or LinearRegionItem.Horizontal')

        for l in self.lines:
            l.setParentItem(self)
            l.sigPositionChangeFinished.connect(self.lineMoveFinished)
            l.sigPositionChanged.connect(self.lineMoved)

        if brush is None:
            brush = QtGui.QBrush(QtGui.QColor(0, 0, 255, 50))
        self.setBrush(brush)

        self.setMovable(movable)

    def setlinebound(self, list_bound):
        for l in self.lines:
            l.setline(list_bound)

    # def boundingRect(self):
    #     br = pg.UIGraphicsItem.boundingRect(self)
    #     rng = self.getRegion()
    #
    #     br.setLeft(rng[0])
    #     br.setRight(rng[1])
    #     br.setTop(1000)
    #     br.setBottom(0)
    #     return br.normalized()

    def mouseDragEvent(self, ev):
        pass

    def boundingRect(self):
        return  QtCore.QRectF()

    def paint(self, p, *args):
        # profiler = pg.graphicsItems.debug.Profiler()
        # pg.graphicsItems.UIGraphicsItem.paint(self, p, *args)
        # p.setBrush(self.currentBrush)
        # p.setPen(fn.mkPen(None))
        # p.drawRect(self.boundingRect())
        pass

class finiteline(pg.InfiniteLine):
    def __init__(self,*args,**kwargs):
        super(finiteline, self).__init__(*args,**kwargs)
        self.list_bound = None
        self.setHoverPen(color=(255,0,0), width=self.pen.width())
        self.setPen(color=(200,200,100), width=3*self.pen.width())


    def setline(self, list_bound):
        self.list_bound = list_bound
        if self.list_bound is None:
            return
        n_delt = (list_bound[0]-list_bound[1])/8
        self._line =  QtCore.QLineF(list_bound[0]+n_delt, 0.0, list_bound[1]-n_delt, 0.0)

    def boundingRect(self):
        self.setline(self.list_bound)
        if self._boundingRect is None:
            # br = UIGraphicsItem.boundingRect(self)
            br = self.viewRect()
            if br is None:
                return QtCore.QRectF()

            ## add a 4-pixel radius around the line for mouse interaction.
            px = self.pixelLength(direction=QtCore.QPoint(1, 0), ortho=True)  ## get pixel length orthogonal to the line
            if px is None:
                px = 0
            w = (max(4, self.pen.width() / 2, self.hoverPen.width() / 2) + 1) * px
            br.setBottom(-w)
            br.setTop(w)

            br = br.normalized()
            self._boundingRect = br
            # self._line = QtCore.QLineF(10000, 0.0, 2000, 0.0)
            # self._line = QtCore.QLineF(0, 0.0, 1000, 0.0)
        return self._boundingRect

    def mouseDragEvent(self, ev):
        if self.movable and ev.button() == QtCore.Qt.LeftButton:
            if ev.isStart():
                self.moving = True

                self.cursorOffset = self.pos() - self.mapToParent(ev.buttonDownPos())
                self.startPosition = self.pos()
            ev.accept()

            if not self.moving:
                return

            # self.setPos(self.cursorOffset + self.mapToParent(ev.pos()))

            if self.angle == 90:
                y = self.startPosition[1]
                # x = self.cursorOffset + self.mapToParent(ev.pos())
                x = self.mapToParent(ev.pos()).x()
                self.setPos(QtCore.QPointF(x,y))
            else:
                x = self.startPosition[0]
                # x = self.cursorOffset + self.mapToParent(ev.pos())
                y = self.mapToParent(ev.pos()).y()
                self.setPos(QtCore.QPointF(x,y))

            self.sigDragged.emit(self)
            if ev.isFinish():
                self.moving = False
                self.sigPositionChangeFinished.emit(self)

    def paint(self, p, *args):
        p.setPen(self.currentPen)

        if self._line is not None:
            p.drawLine(self._line)
        # p.drawLine(QtCore.QLineF(100, 100, 1000, 0))
