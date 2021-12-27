from PyQt5 import QtGui, QtWidgets, QtCore
from kxpyqtgraph.kxparameterTree.KxCustomWidget import *


class WidgetCPCTSParam(QtWidgets.QWidget):
    def __init__(self):
        super(WidgetCPCTSParam, self).__init__()
        self.h_parameterTree = ParameterTree(self, False)
        self.h_parameterTree.setStyleSheet("QTreeWidget::item{height:30px;}")
        self.verlayout = QtWidgets.QVBoxLayout(self)
        self.verlayout.addWidget(self.h_parameterTree)

        self.params = [
            {'name': '涂胶总面积上限', 'type': 'int', 'value': 100000, 'limits': [1, 100000000]},
            {'name': '涂胶总面积下限', 'type': 'int', 'value': 10000, 'limits': [1, 100000000]},
            {'name': '检测气泡尺寸', 'type': 'int', 'value': 5, 'limits': [1, 100000000]},
            {'name': '叛废气泡个数', 'type': 'int', 'value': 2, 'limits': [1, 100000000]},
            {'name': '检测异物尺寸', 'type': 'int', 'value': 2, 'limits': [1, 100000000]},
            {'name': '叛废异物个数', 'type': 'int', 'value': 2, 'limits': [1, 100000000]},
            {'name': '涂胶位置度上限', 'type': 'int', 'value': 3, 'limits': [1, 100000000]},
            {'name': '涂胶位置度下限', 'type': 'int', 'value': 1, 'limits': [1, 100000000]}
        ]
        self.p = KxParameter.create(name='params', type='group', children=self.params)
        self.h_parameterTree.setParameters(self.p, showTop=False)
        self.setEnabled(False)

if __name__ == "__main__":
    a = QtWidgets.QApplication([])
    w = WidgetCPCTSParam()
    w.show()
    a.exec_()