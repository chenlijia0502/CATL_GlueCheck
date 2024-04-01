#coding:utf-8

import pyqtgraph as pg
from PyQt5 import QtGui, QtCore


class FiniteLineItem(pg.InfiniteLine):
    
    sigLinePosChanged = QtCore.pyqtSignal()
    
    def __init__(self, text='', pos=0, lineLength=100, orientation=1, movable=True, bounds=None, pen=None, hoverPen=None):
        self.lineLength = lineLength
        if orientation == pg.LinearRegionItem.Horizontal:
            angle = 0
        else:
            angle = 90
        super(FiniteLineItem, self).__init__(label=text, labelOpts={"color":(255,0,0)}, pos=pos, angle=angle, pen=pen, movable=movable, bounds=bounds, hoverPen=hoverPen)
        self.label.setFont(QtGui.QFont('Consolas', 15, 60))
        self.sigPositionChangeFinished.connect(self.sigLinePosChanged)
        
    def setPos(self, v):
        super(FiniteLineItem, self).setPos(v)
        self.sigLinePosChanged.emit()
            
    def getPos(self):
        pos = self.value()
        if isinstance(pos, list):
            return int(pos[0])
        else:
            return int(pos)
    
    def setLineLength(self, length):#��λ����
        self.lineLength = length
        self.update() 
        
    def getLineLength(self):
        return self.lineLength
    
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
            # print (self._boundingRect)
            self._line = QtCore.QLineF(0, 0, self.lineLength, 0)

            # HYH 2020.06.04 新加
            self._endPoints = [0, self.lineLength]
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
