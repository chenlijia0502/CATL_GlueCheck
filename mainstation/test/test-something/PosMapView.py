from PyQt5 import Qt, QtWidgets, QtCore, QtGui
import pyqtgraph as pg
import numpy as np

class CPosMapView(QtWidgets.QWidget):
    def __init__(self):
        super(CPosMapView, self).__init__()
        self.verticallayout = QtWidgets.QVBoxLayout(self)
        self.graphicsView = pg.GraphicsView(self)
        self.verticallayout.addWidget(self.graphicsView)
        self.view = pg.ViewBox()
        self.graphicsView.setCentralItem(self.view)
        self.imgitem = pg.ImageItem()
        self.view.addItem(self.imgitem)
        g = pg.GraphItem()
        self.view.addItem(g)
        bigimg = np.ones([100, 100], np.uint8) * 100
        self.imgitem.setImage(bigimg)

        PEN = pg.mkPen(color=(255, 0, 0), width=3)
        symbols = ['o', '+']
        g.setData(pos=np.array([[50, 50], [80, 80]]),  symbolBrush=(0, 0, 200), size=30, symbol=symbols)
        g.setData(pos=np.array([[70, 50], [70, 80]]),  symbolBrush=(0, 0, 200), size=30, symbol=symbols)
        text2 = pg.TextItem("test", anchor=(0.5, -1.0))
        text2.setParentItem(g)
        curvePoint = pg.CurvePoint()




""""""



if __name__ == "__main__":
    app = QtWidgets.QApplication([])
    w = CPosMapView()
    w.show()
    app.exec_()

#
# # -*- coding: utf-8 -*-
# """
# Simple example of GraphItem use.
# """
#
#
# from pyqtgraph.Qt import QtCore, QtGui
# import numpy as np
#
# # Enable antialiasing for prettier plots
# pg.setConfigOptions(antialias=True)
#
# w = pg.GraphicsLayoutWidget(show=True)
# w.setWindowTitle('pyqtgraph example: GraphItem')
# v = w.addViewBox()
# v.setAspectLocked()
#
# g = pg.GraphItem()
# v.addItem(g)
#
# ## Define positions of nodes
# pos = np.array([
#     [0, 0],
#     [10, 0],
#     [0, 10],
#     [10, 10],
#     [5, 5],
#     [15, 5]
# ])
#
# ## Define the set of connections in the graph
# adj = np.array([
#     [0, 1],
#     [1, 3],
#     [3, 2],
#     [2, 0],
#     [1, 5],
#     [3, 5],
# ])
#
# ## Define the symbol to use for each node (this is optional)
# symbols = ['o', 'o', 'o', 'o', 't', '+']
#
# ## Define the line style for each connection (this is optional)
# lines = np.array([
#     (255, 0, 0, 255, 1),
#     (255, 0, 255, 255, 2),
#     (255, 0, 255, 255, 3),
#     (255, 255, 0, 255, 2),
#     (255, 0, 0, 255, 1),
#     (255, 255, 255, 255, 4),
# ], dtype=[('red', np.ubyte), ('green', np.ubyte), ('blue', np.ubyte), ('alpha', np.ubyte), ('width', float)])
#
# ## Update the graph
# g.setData(pos=pos, adj=adj, pen=lines, size=1, symbol=symbols, pxMode=False)
#
# ## Start Qt event loop unless running in interactive mode or using pyside.
# if __name__ == '__main__':
#     import sys
#
#     if (sys.flags.interactive != 1) or not hasattr(QtCore, 'PYQT_VERSION'):
#         QtGui.QApplication.instance().exec_()
