# -*- coding: utf-8 -*-

import pyqtgraph as pg
from PyQt5 import QtGui,QtCore
from pyqtgraph.Point import Point
from pyqtgraph.graphicsItems.GraphicsObject import GraphicsObject
from pyqtgraph.graphicsItems.UIGraphicsItem import UIGraphicsItem

__all__ = ['KxLineRegionItem']

class KxLineRegionItem(pg.LinearRegionItem):
    
    '''
    lineLength:length of per line
    values:  A list of the positions of the lines in the region.
    orientation:the orientation of the region,1 is horizontal, 0 is vertical
    '''
    
    sigLineRegionChange = QtCore.pyqtSignal()
    
    def __init__(self, lineLength=100, values=[0,100], orientation=1, movable=True, lineFixed=False, 
                 resolution=1, pen=None, bounds=None, hoverPen=None):
        
        values = [val / float(resolution) for val in values]
        super(KxLineRegionItem, self).__init__(values, orientation, QtGui.QBrush(QtGui.QColor(0, 0, 0, 0)), movable, bounds)
        
        self.lineLength = lineLength
        self.linePen = pen
        self.hoverPen = hoverPen
        self.boundVals = bounds
        self.resolution = resolution
        
        for l in self.lines:
            l.setParentItem(None)
            l.sigPositionChangeFinished.disconnect(self.lineMoveFinished)
            l.sigPositionChanged.disconnect(self.lineMoved)
        
        if orientation == pg.LinearRegionItem.Horizontal:
            self.lines = [
                FiniteLine(lineLength, QtCore.QPointF(0, values[0]), 0, pen=pen, movable=not lineFixed, bounds=bounds, hoverPen=hoverPen), 
                FiniteLine(lineLength, QtCore.QPointF(0, values[1]), 0, pen=pen, movable=not lineFixed, bounds=bounds, hoverPen=hoverPen)]
        elif orientation == pg.LinearRegionItem.Vertical:
            self.lines = [
                FiniteLine(lineLength, QtCore.QPointF(values[1], 0), 90, pen=pen, movable=not lineFixed, bounds=bounds, hoverPen=hoverPen), 
                FiniteLine(lineLength, QtCore.QPointF(values[0], 0), 90, pen=pen, movable=not lineFixed, bounds=bounds, hoverPen=hoverPen)]
        else:
            raise Exception('Orientation must be one of Vertical(0) or Horizontal(1)')
        
        for l in self.lines:
            l.setParentItem(self)
            l.sigPositionChangeFinished.connect(self.lineMoveFinished)
            l.sigPositionChanged.connect(self.lineMoved)
            
        self.setMovable(movable)
        self.setLineFixed(lineFixed)
        
        self.sigRegionChangeFinished.connect(self.sigLineRegionChange)
#         self.sigRegionChangeFinished.connect(self.slotRegionChanged)
        
    def setValues(self, vals): #单位mm
        vals = [val / float(self.resolution) for val in vals]
        self.setRegion(vals)
        
        
    def setLineInterval(self, interval):  #单位mm
        vals = self.getValues()
        self.setValues([vals[0], vals[0] + interval - 1])
        
    def setLineLength(self, length):#单位像素
        self.lineLength = length
        
        oldPos = self.lines[0].pos()
        for l in self.lines:
            l.setParentItem(None)
            l.sigPositionChangeFinished.disconnect(self.lineMoveFinished)
            l.sigPositionChanged.disconnect(self.lineMoved)
        
        vals = self.getRegion()
        if self.orientation == pg.LinearRegionItem.Horizontal:
            self.lines = [
                FiniteLine(length, QtCore.QPointF(oldPos.x(), vals[0]), 0, pen=self.linePen, movable=not self.lineFixed, bounds=self.boundVals, hoverPen=self.hoverPen), 
                FiniteLine(length, QtCore.QPointF(oldPos.x(), vals[1]), 0, pen=self.linePen, movable=not self.lineFixed, bounds=self.boundVals, hoverPen=self.hoverPen)]
        else:
            self.lines = [
                FiniteLine(length, QtCore.QPointF(vals[1], oldPos.y()), 90, pen=self.linePen, movable=not self.lineFixed, bounds=self.boundVals, hoverPen=self.hoverPen), 
                FiniteLine(length, QtCore.QPointF(vals[0], oldPos.y()), 90, pen=self.linePen, movable=not self.lineFixed, bounds=self.boundVals, hoverPen=self.hoverPen)]
        
        for l in self.lines:
            l.setParentItem(self)
            l.sigPositionChangeFinished.connect(self.lineMoveFinished)
            l.sigPositionChanged.connect(self.lineMoved)
            
        self.setMovable(self.movable)
        self.update()
        
    def setMovable(self, m):
        self.movable = m
        self.setAcceptHoverEvents(m)
    
    def setLineFixed(self, flag):
        self.lineFixed = flag
        for l in self.lines:
            l.setMovable(not flag) 
            
    def setAllFixed(self, flag):
        self.setMovable(not flag)
        self.setLineFixed(flag)
            
    def getValues(self):
        vals = self.getRegion()
        return [float('%.1f' % (val * float(self.resolution))) for val in vals]
    
    def getLineInterval(self):
        rgn = self.getRegion()
        vals = [val * float(self.resolution) for val in rgn]
        return float('%.1f' %(vals[1] - vals[0] + 1))
    
    def getLineLength(self):
        return self.lineLength
    
    def getIsLineFixed(self):
        return self.lineFixed
    
    def setResolution(self, resolution):
        actualVals = self.getValues()
        vals = [acVal / float(resolution) for acVal in actualVals]
        self.setRegion(vals)
        self.resolution = resolution
        
    def setLinePos(self, posVal):
        if self.orientation == 1:
            newPos = [(posVal, self.lines[0].getYPos()),
                      (posVal, self.lines[1].getYPos())]
        else:
            newPos = [(self.lines[0].getXPos(), posVal),
                      (self.lines[1].getXPos(), posVal)]
            
        self.lines[0].blockSignals(True)  # only want to update once
        for i, l in enumerate(self.lines):
            l.setPos(newPos[i])
        self.lines[0].blockSignals(False)

        
    def boundingRect(self):
        br = UIGraphicsItem.boundingRect(self)
        rng = self.getRegion()
        if self.orientation == pg.LinearRegionItem.Vertical:
            m = self.lines[0].getYPos()
            br.setLeft(rng[0])
            br.setRight(rng[1])
            br.setTop(m)
            br.setBottom(m + self.lineLength)
        else:
            m = self.lines[0].getXPos()
            br.setTop(rng[0])
            br.setBottom(rng[1])
            br.setLeft(m)
            br.setRight(m + self.lineLength)
            
        return br.normalized()
        
#     def slotRegionChanged(self):
#         r = [self.lines[0].value(), self.lines[1].value()]
#         interval = r[1] - r[0] + 1
#         minIndex = r.index(min(r))
#         maxIndex = r.index(max(r))
#         if self.orientation == pg.LinearRegionItem.Horizontal:
#             self.lines[minIndex].setBounds([self.boundVals[2], self.boundVals[3] - interval + 1])
#             self.lines[maxIndex].setBounds([self.boundVals[2] + interval - 1, self.boundVals[3]])
#         else:    
#             self.lines[minIndex].setBounds([self.boundVals[0], self.boundVals[1] - interval + 1])
#             self.lines[maxIndex].setBounds([self.boundVals[0] + interval - 1, self.boundVals[1]])
    
    
    

class FiniteLine(pg.InfiniteLine):
    def __init__(self, lineLength=100, *args,**kwargs):
        self.lineLength = lineLength
        super(FiniteLine, self).__init__(*args, **kwargs)
        
    def setValue(self, v):
        if self.angle % 180 == 0:
            self.setPos([self.pos().x(), v])
        else:
            self.setPos([v, self.pos().y()])
    
    def boundingRect(self):
        expand = 10
        if self._boundingRect is None:
            br = self.viewRect()
            if br is None:
                return QtCore.QRectF()
            
            px = self.pixelLength(direction=QtCore.QPointF(1,0), ortho=True) 
            if px is None:
                px = 0
            w = (max(4, self.pen.width()/2, self.hoverPen.width()/2)+1) * px
            br.setBottom(-w)
            br.setTop(w)
            br.setLeft(-expand)
            br.setRight(self.lineLength + expand)
            
            br = br.normalized()
            self._boundingRect = br
            
            self._line = QtCore.QLineF(0, 0, self.lineLength, 0)
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
            
            if self.angle % 180 == 0:
                newPos = QtCore.QPointF(self.pos().x(), (self.cursorOffset + self.mapToParent(ev.pos())).y())
            else:
                newPos = QtCore.QPointF((self.cursorOffset + self.mapToParent(ev.pos())).x(), self.pos().y())
            
            self.setPos(newPos)
            self.sigDragged.emit(self)
            if ev.isFinish():
                self.moving = False
                self.sigPositionChangeFinished.emit(self)
                
