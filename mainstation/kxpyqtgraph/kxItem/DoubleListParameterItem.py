from pyqtgraph.parametertree.parameterTypes import WidgetParameterItem,ListParameter
from PyQt5 import QtGui, QtCore, QtWidgets
from kxpyqtgraph.kxparameterTree.KxparameterTypes import *
from kxpyqtgraph.kxparameterTree.KxCustomWidget import *




class CDoubleCombobox(QtWidgets.QWidget):
    sigChanged = QtCore.pyqtSignal(int)
    def __init__(self):
        super(CDoubleCombobox, self).__init__()
        self._initui()

    def _initui(self):
        layout = QtWidgets.QHBoxLayout(self)
        self.box1 = QtWidgets.QComboBox(self)
        self.box2 = QtWidgets.QComboBox(self)
        self.box1.currentIndexChanged.connect(self.indexchange)
        self.box2.currentIndexChanged.connect(self.indexchange)

        self.box1.setMaximumHeight(30)
        self.box2.setMaximumHeight(30)
        self.box1.setStyleSheet("font: 13pt \"Arial\";")
        self.box2.setStyleSheet("font: 13pt \"Arial\";")
        layout.addWidget(self.box1)
        layout.addWidget(self.box2)

    def indexchange(self, nint):
        self.sigChanged.emit(nint)

    def setCurrentIndex(self, nindex):
        self.box1.setCurrentIndex(nindex)

    def currentText(self):
        return self.box2.currentText()

    def box1currentText(self):
        return self.box1.currentText()

    def clear(self):
        self.box1.clear()
        self.box2.clear()

class DoubleListParameterItem(WidgetParameterItem):
    """
    author:         HYH
    date:           2021.3.19
    description:    双combobox，目的是第一个combobox会限制第二个combobox的选项，且返回结果是第二个combobox的选型。
                    输入exp:  value:{a:{choice1: choice1_value, choice2:choice2_value, choice3:choice3_value...], b:{...}, ...}

    Note:           values中所有的值与键必须都是 字符串 格式
"""

    def __init__(self, param, depth):
        self.targetValue = None
        WidgetParameterItem.__init__(self, param, depth)
        param.sigResetlimits.connect(self.clear)
        self.updateDisplayLabel()

    def makeWidget(self):
        w = CDoubleCombobox()
        w.value = self.value
        w.setValue = self.setValue
        self.widget = w
        self.widget.box1.currentIndexChanged.connect(self._updatebox2)
        self._initcomboboxItem(self.param.opts['values'], self.param.opts['value'])
        return w

    def _updatebox2(self):
        self.widget.box2.clear()
        values = self.param.opts['values']
        key = self.widget.box1.currentText()
        for z in values[key]:
            self.widget.box2.addItem(str(z))
        self.widget.box2.setCurrentIndex(0)


    def _initcomboboxItem(self, values, targetvalue):
        """
        初始两个combobox的值
        :param value:
        :return:
        """
        try:
            self.widget.blockSignals(True)# 阻塞信号
            #val = self.targetValue  # asUnicode(self.widget.currentText())
            self.widget.clear()
            for k in values:
                self.widget.box1.addItem(str(k))
                if k == targetvalue:
                    self.widget.setCurrentIndex(self.widget.box1.count() - 1)
                    #self.updateDisplayLabel()
                    for z in values[k]:
                        self.widget.box2.addItem(str(z))
                    self.widget.box2.setCurrentIndex(0)
        finally:
            self.widget.blockSignals(False)

    def updateDisplayLabel(self, value=None):
        """Update the display label to reflect the value of the parameter."""
        if value is None:
            value = self.param.value()
        else:
            value = self.value()
        self.displayLabel.setText(value)


    def value(self):
        key1 = self.widget.box1.currentText()
        key2 = self.widget.box2.currentText()
        try:
            value = self.param.opts['values'][key1][key2]
        except Exception:
            value = 'None'
        return value

    def setValue(self, val):
        """这个控件不需要setvalue"""
        pass


    def clear(self, *even):
        if even[0] is None:
            # self.widget.clear()
            self.updateDisplayLabel('')
        else:
            self.setValue(even[0])
            self.updateDisplayLabel()

class DoubleListParameter(KxParameter):
    itemClass = DoubleListParameterItem
    sigResetlimits = QtCore.pyqtSignal(object)

    def __init__(self, **opts):
        self.forward = OrderedDict()  ## {name: value, ...}
        self.reverse = ([], [])  ## ([value, ...], [name, ...])

        ## Parameter uses 'limits' option to define the set of allowed values
        if 'values' in opts:
            opts['limits'] = opts['values']
        if opts.get('limits', None) is None:
            opts['limits'] = []
        KxParameter.__init__(self, **opts)
        #self.setLimits(opts['limits'])

    # def setLimits(self, limits):
    #     self.forward, self.reverse = self.mapping(limits)
    #     self.sigResetlimits.emit(None)
    #     # self.clearlimits()
    #     KxParameter.setLimits(self, limits)
    #     if self.value() is not None:
    #         value = int(self.value())
    #     else:
    #         # value = self.value()
    #         value = 0
    #     if len(self.reverse[0]) > 0 and value not in self.reverse[0]:
    #         # self.setValue(self.reverse[0][0])
    #         self.setValue(value)
    #         self.sigResetlimits.emit(self.reverse[0][0])
    #     else:
    #         self.sigResetlimits.emit(self.value())

    @staticmethod
    def mapping(limits):
        ## Return forward and reverse mapping objects given a limit specification
        forward = OrderedDict()  ## {name: value, ...}
        reverse = ([], [])  ## ([value, ...], [name, ...])
        if isinstance(limits, dict):
            for k, v in limits.items():
                forward[k] = v
                reverse[0].append(v)
                reverse[1].append(k)
        else:
            for v in limits:
                n = asUnicode(v)
                forward[n] = v
                reverse[0].append(v)
                reverse[1].append(n)
        return forward, reverse


kxregisterParameterType('doublelist', DoubleListParameter, override=True)


class paramwidget(QtWidgets.QWidget):
    def __init__(self):
        super(paramwidget, self).__init__()
        self.verlayout = QtWidgets.QVBoxLayout(self)
        self.button = CDoubleCombobox()
        self.verlayout.addWidget(self.button)
        self.h_parameterTree = ParameterTree(self, False)

        self.h_parameterTree.setStyleSheet("QTreeWidget::item{height:40px;}")
        self.verlayout.addWidget(self.h_parameterTree)

        params = [{'name': u'全局设置', 'type': 'group', 'visible': True, 'children': [
            {'name': u'界面号', 'type': 'doublelist', 'values': {'0':{'a':'A', 'b':'B'}, '1':{'c':'C', 'd':'D'}}, 'value':'1', 'readonly': False},
            #{'name': u'界面号', 'type': 'int','value': 1, 'readonly': True},
            {'name': u'分辨率x', 'type': 'float', 'value': 0.057, 'step': 0.001, 'limits': (0, 1)},
            {'name': u'分辨率y', 'type': 'float', 'value': 0.057, 'step': 0.001, 'limits': (0, 1)},
        ]}]
        self.p = KxParameter.create(name='params', type='group', children=params)
        self.h_parameterTree.setParameters(self.p, showTop=False)

if __name__ == "__main__":
    a = QtWidgets.QApplication([])
    w = paramwidget()
    w.show()
    a.exec()

