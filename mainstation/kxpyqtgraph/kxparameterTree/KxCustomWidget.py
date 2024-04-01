# coding:utf-8
import pyqtgraph as pg
from pyqtgraph.Qt import QtCore, QtGui
from pyqtgraph import configfile
from pyqtgraph.parametertree import Parameter
from pyqtgraph.parametertree.parameterTypes import WidgetParameterItem,\
    ActionParameter, SimpleParameter, ListParameter
from pyqtgraph.widgets.SpinBox import SpinBox
import pyqtgraph.parametertree.parameterTypes as pTypes
from pyqtgraph.parametertree import ParameterItem, ParameterTree
from kxpyqtgraph.kxparameterTree.KxParameter import  KxParameter, kxregisterParameterType
from PyQt5 import QtGui, QtCore, QtWidgets
from pyqtgraph.python2_3 import asUnicode
from pyqtgraph import OrderedDict
from kxpyqtgraph.kxItem.kxdoubleroiitem import *
from kxpyqtgraph.kxItem.KxLineRegionItem import KxLineRegionItem
from kxpyqtgraph.kxItem.FiniteLineItem import FiniteLineItem
from kxpyqtgraph.kxItem.kxlinearregionitem import KxLinearRegionItem
from kxpyqtgraph.kxparameterTree.KxparameterTypes import *

"""
    这些类的实现方法非常值得去钻，主要是parametertree上的控件
"""


class KxSlider(QtWidgets.QSlider):
    '''
    custom slider, use to  set a value by clicking slider
    '''
    def __init__(self, HOrV, parent = None):
        super(KxSlider, self).__init__(HOrV, parent)
        self.orientation = HOrV

    def mousePressEvent(self, event):
        super(KxSlider, self).mousePressEvent(event)
        if event.button() == QtCore.Qt.LeftButton:
            dur = self.maximum() - self.minimum()
            if self.orientation == QtCore.Qt.Horizontal:
                pos = int(round(self.minimum() + dur * (float(event.x()) / self.width())))
            if self.orientation == QtCore.Qt.Vertical:
                pos = int(round(self.minimum() + dur * ((self.height()-float(event.y())) / self.height())))
            if pos != self.sliderPosition():
                self.setValue(pos)
                self.sliderReleased.emit()



class KxSliderParameterItem(WidgetParameterItem):
    """
    WidgetParameterItem subclass providing slider that lets the user select a value options.

    """
    def __init__(self, param, depth):
        WidgetParameterItem.__init__(self, param, depth)
        self.hideWidget = False
        param.sigOptionsChanged.connect(self.lock)


    def makeWidget(self):
        opts = self.param.opts
        self.w = QtWidgets.QWidget()
        self.layout = QtWidgets.QHBoxLayout()

        self.slider = KxSlider(QtCore.Qt.Horizontal)
        self.slider.setMaximumHeight(20)
        self.spin = SpinBox()
        self.spin.setMaximumHeight(20)
        # self.spin.setMaximumWidth(55)

        defs = {
                'value': 0, 'min': None, 'max': None,
                'step': 1.0, 'bounds':[0,100],'int':True
               }

        for k in defs:
            if k in opts:
               defs[k] = opts[k]
            if 'limits' in opts:
                defs['bounds'] = opts['limits']

        self.spin.setOpts(**defs)
        self.setSliderOpts(**defs)

        self.layout.addWidget(self.slider,3)
        self.layout.addWidget(self.spin,1)

        self.w.setLayout(self.layout)

        # self.spin.sigValueChanged.connect(self.spinValueChanged)
        # self.spin.sigValueChanged.connect(self.setValue)
        self.spin.sigValueChanging.connect(self.setValue)
        # self.slider.valueChanged.connect(self.sliderValueChanged)
        # TODO：change by hyh in 2017/8/1
        self.slider.sliderReleased.connect(self.sliderValueChanged)
        # self.slider.valueChanged.connect(self.sliderresetvalue)


        self.w.sigChanged = self.spin.valueChanged
        self.w.value = self.value
        self.w.setValue = self.setValue

        # 2017/7/14
        self.bool_lock = False
        return self.w


    def sliderValueChanged(self,value=None):
        # print 'here'
        if value == None:
            value = self.slider.value()
        self.spin.setValue(value)

    # def sliderresetvalue(self):
    #     """use for bug"""
    #     if self.slider.value() == self.slider.maximum() or self.slider.value() == self.slider.minimum():
    #         self.spin.setValue(self.slider.value())


    def spinValueChanged(self):
        # if self.bool_lock is not True:
        value = self.spin.value()
        self.slider.setValue(value)
        self.slider.valueChanged.emit(value)

    # def setbool(self, bool = None):
    #     if bool is not None:
    #         self.bool_lock = bool


    def value(self):
        """
        Return the value of this Widget.

        """
        return self.spin.value()

    def setValue(self, value):
        """
        Return the value of this Widget.
        #TODO : 2020.03.23 这有个bug，当直接点击slider到最大值时，信号未被触发，也即这个函数未被调用
        """

        if isinstance(value, SpinBox):
            values = value.value()
        else:
            values = value
        self.spin.setValue(int(values))
        self.slider.setValue(int(str(values)))


    def setSliderOpts(self, **opts):
        """
        Changes the behavior of the Slider. Accepts most of the arguments
        allowed in :func:`__init__ <pyqtgraph.Slider.__init__>`.

        """
        for k in opts:
            if k == 'bounds':
                self.slider.setMinimum(opts[k][0])
                self.slider.setMaximum(opts[k][1])
            elif k == 'min':
                if opts[k] is not None:
                    self.slider.setMinimum(opts[k])
            elif k == 'max':
                if opts[k] is not None:
                    self.slider.setMaximum(opts[k])
            elif k == 'step':
                self.slider.setSingleStep(opts[k])
            elif k == 'value' or k == 'int':
                pass


            else:
                raise TypeError("Invalid keyword argument '%s'." % k)
        if 'value' in opts:
            self.slider.setValue(int(opts['value']))

    def lockslider(self):
        self.slider.setValue(self.curvalue)
        self.spin.setValue(self.curvalue)

    def lock(self, param, opts):
        if 'lock' in opts:
            if opts['lock']:
                self.widget.setEnabled(False)
            else:
                self.widget.setEnabled(True)

class KxSliderParameter(KxParameter):
    """Used for displaying a button within the tree."""
    itemClass = KxSliderParameterItem

    def setReadonly(self, readonly=True):
        self.sigOptionsChanged.emit(None, {'lock': True})

    def setWritable(self, writable=True):
        self.sigOptionsChanged.emit(None, {'lock': False})

kxregisterParameterType('slider', KxSliderParameter, override=True)


class KxTextParameterItem(WidgetParameterItem):
    def __init__(self, param, depth):
        WidgetParameterItem.__init__(self, param, depth)
        self.hideWidget = False
        self.subItem = QtWidgets.QTreeWidgetItem()
        self.addChild(self.subItem)
        param.sigValueWrong.connect(self.changebackupcolor)
        param.sigOptionsChanged.connect(self.setreadonly)

    def treeWidgetChanged(self):
        ## TODO: fix so that superclass method can be called
        ## (WidgetParameter should just natively support this style)
        # WidgetParameterItem.treeWidgetChanged(self)
        self.treeWidget().setFirstItemColumnSpanned(self.subItem, True)
        self.treeWidget().setItemWidget(self.subItem, 0, self.textBox)

        # for now, these are copied from ParameterItem.treeWidgetChanged
        self.setHidden(not self.param.opts.get('visible', True))
        self.setExpanded(self.param.opts.get('expanded', True))


    def makeWidget(self):
        self.textBox = QtWidgets.QTextEdit()
        self.textBox.setMaximumHeight(100)
        self.textBox.setReadOnly(self.param.opts.get('readonly', False))
        self.textBox.value = lambda: str(self.textBox.toPlainText())
        self.textBox.setValue = self.textBox.setPlainText
        self.textBox.sigChanged = self.textBox.textChanged
        self.textBox.setAutoFillBackground(True)
        # self.textBox.connect()
        # self.changebackupcolor()
        self.palette = QtGui.QPalette()

        return self.textBox

    def changebackupcolor(self, bool_signal=True):
        if bool_signal:
            # self.palette.setColor(QtGui.QPalette.Background, QtGui.QColor(255, 255, 255))
            # self.textBox.setPalette(self.palette)
            self.textBox.setStyleSheet("background-color: rgba(255, 255, 255,100%);")

        else:
            # self.palette.setColor(QtGui.QPalette.Background, QtGui.QColor(255, 64, 64))
            # self.textBox.setPalette(self.palette)
            self.textBox.setStyleSheet("background-color: rgba(255, 64, 64,10%);")

            # self.textBox.clear()


    def setreadonly(self, param, opts):
        if param is None:
            if 'lock' in opts:
                if opts['lock'] is True:
                    self.textBox.setReadOnly(True)
                else:
                    self.textBox.setReadOnly(False)

class KxTextParameter(KxParameter):
    """Editable string; displayed as large text box in the tree."""
    itemClass = KxTextParameterItem
    sigValueWrong = QtCore.pyqtSignal(object)

    def evalwrong(self, bool_signal):
        self.sigValueWrong.emit(bool_signal)

    def setReadonly(self, readonly=True):
        self.sigOptionsChanged.emit(None, {'lock': True})

    def setWritable(self, writable=True):
        self.sigOptionsChanged.emit(None, {'lock': False})

kxregisterParameterType('kxtext', KxTextParameter, override=True)


class KxGroupParameterItem(ParameterItem):
    """
    Group parameters are used mainly as a generic parent item that holds (and groups!) a set
    of child parameters. It also provides a simple mechanism for displaying a button or combo
    that can be used to add new parameters to the group.
    """

    def __init__(self, param, depth):
        ParameterItem.__init__(self, param, depth)
        param.KxSigsethide.connect(self.setvisible)
        self.updateDepth(depth)

        self.addItem = None
        if 'addText' in param.opts:
            addText = param.opts['addText']
            if 'addList' in param.opts:
                self.addWidget = QtWidgets.QComboBox()
                self.addWidget.setSizeAdjustPolicy(QtWidgets.QComboBox.AdjustToContents)
                self.updateAddList()
                self.addWidget.currentIndexChanged.connect(self.addChanged)
            else:
                self.addWidget = QtWidgets.QPushButton(addText)
                self.addWidget.clicked.connect(self.addClicked)
            w = QtWidgets.QWidget()
            l = QtWidgets.QHBoxLayout()
            l.setContentsMargins(0, 0, 0, 0)
            w.setLayout(l)
            l.addWidget(self.addWidget)
            l.addStretch()
            # l.addItem(QtGui.QSpacerItem(200, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum))
            self.addWidgetBox = w
            self.addItem = QtWidgets.QTreeWidgetItem([])
            self.addItem.setFlags(QtCore.Qt.ItemIsEnabled)
            ParameterItem.addChild(self, self.addItem)

    def updateDepth(self, depth):
        ## Change item's appearance based on its depth in the tree
        ## This allows highest-level groups to be displayed more prominently.
        if depth == 0:
            for c in [0, 1]:
                #self.setBackground(c, QtGui.QBrush(QtGui.QColor(100, 100, 100)))
                #self.setForeground(c, QtGui.QBrush(QtGui.QColor(220, 220, 255)))
                #HYH 2021.7.22
                self.setBackground(c, QtGui.QBrush(QtGui.QColor(24, 139, 209)))#深蓝色
                self.setForeground(c, QtGui.QBrush(QtGui.QColor(164, 221, 254)))
                font = self.font(c)
                font.setBold(True)
                font.setPointSize(font.pointSize() + 1)
                self.setFont(c, font)
                self.setSizeHint(0, QtCore.QSize(0, 25))
        else:
            for c in [0, 1]:
                self.setForeground(c, QtGui.QBrush(QtGui.QColor(164, 221, 254)))
                #self.setBackground(c, QtGui.QBrush(QtGui.QColor(220, 220, 220)))
                font = self.font(c)
                font.setBold(True)
                # font.setPointSize(font.pointSize()+1)
                self.setFont(c, font)
                self.setSizeHint(0, QtCore.QSize(0, 20))

    def addClicked(self):
        """Called when "add new" button is clicked
        The parameter MUST have an 'addNew' method defined.
        """
        self.param.addNew()

    def addChanged(self):
        """Called when "add new" combo is changed
        The parameter MUST have an 'addNew' method defined.
        """
        if self.addWidget.currentIndex() == 0:
            return
        typ = asUnicode(self.addWidget.currentText())
        self.param.addNew(typ)
        self.addWidget.setCurrentIndex(0)

    def treeWidgetChanged(self):
        ParameterItem.treeWidgetChanged(self)
        if self.treeWidget() is not None:
            self.treeWidget().setFirstItemColumnSpanned(self, True)
            if self.addItem is not None:
                self.treeWidget().setItemWidget(self.addItem, 0, self.addWidgetBox)
                self.treeWidget().setFirstItemColumnSpanned(self.addItem, True)

    def addChild(self, child):  ## make sure added childs are actually inserted before add btn
        if self.addItem is not None:
            ParameterItem.insertChild(self, self.childCount() - 1, child)
        else:
            ParameterItem.addChild(self, child)

    def optsChanged(self, param, changed):
        if 'addList' in changed:
            self.updateAddList()

    def updateAddList(self):
        self.addWidget.blockSignals(True)
        try:
            self.addWidget.clear()
            self.addWidget.addItem(self.param.opts['addText'])
            for t in self.param.opts['addList']:
                self.addWidget.addItem(t)
        finally:
            self.addWidget.blockSignals(False)

    def sethid(self):
        self.setHidden(True)

    def setshow(self):
        self.setHidden(False)

    def setvisible(self):
        self.treeWidgetChanged()

class KxGroupParameter(KxParameter):
    """
    Group parameters are used mainly as a generic parent item that holds (and groups!) a set
    of child parameters. 

    It also provides a simple mechanism for displaying a button or combo
    that can be used to add new parameters to the group. To enable this, the group 
    must be initialized with the 'addText' option (the text will be displayed on
    a button which, when clicked, will cause addNew() to be called). If the 'addList'
    option is specified as well, then a dropdown-list of addable items will be displayed
    instead of a button.
    """
    itemClass = KxGroupParameterItem
    # Siggrouphide = QtCore.Signal(object)
    KxSigsethide = QtCore.pyqtSignal()

    def addNew(self, typ=None):
        """
        This method is called when the user has requested to add a new item to the group.
        """
        raise Exception("Must override this function in subclass.")

    def setAddList(self, vals):
        """Change the list of options available for the user to add to the group."""
        self.setOpts(addList=vals)
        # def
        # self.sigOptionsChanged.emit(self, {'visible': s})

    def hide(self):
        self.setOpts(visible=False)
        self.KxSigsethide.emit()


    def show(self):
        self.setOpts(visible=True)
        self.KxSigsethide.emit()

kxregisterParameterType('group', KxGroupParameter, override=True)


class KxListParameterItem(WidgetParameterItem):
    """
    WidgetParameterItem subclass providing comboBox that lets the user select from a list of options.

    """

    def __init__(self, param, depth):
        self.targetValue = None
        WidgetParameterItem.__init__(self, param, depth)
        param.sigResetlimits.connect(self.clear)

    def makeWidget(self):
        opts = self.param.opts
        t = opts['type']
        w = QtWidgets.QComboBox()
        w.setMaximumHeight(30)  ## set to match height of spin box and line edit
        #w.setStyleSheet( "QComboBox QAbstractItemView::item{height:50px;}" )
        w.setStyleSheet("font: 13pt \"Arial\";")

        w.sigChanged = w.currentIndexChanged
        w.value = self.value
        w.setValue = self.setValue
        self.widget = w  ## needs to be set before limits are changed
        self.limitsChanged(self.param, self.param.opts['limits'])
        if len(self.forward) > 0:
            if self.param.value() is not None:
                self.setValue(self.param.value())
        return w

    def value(self):
        key = asUnicode(self.widget.currentText())
        return self.forward.get(key, None)

    def setValue(self, val):
        #TODO : 2022.1.21 修改这里只能输入数字的问题
        if str(val).isdigit():
            val = int(val)
        self.targetValue = val
        if val not in self.reverse[0]:
            self.widget.setCurrentIndex(0)
        else:
            key = self.reverse[1][self.reverse[0].index(val)]
            ind = self.widget.findText(key)
            self.widget.setCurrentIndex(ind)
        # self.defaultBtn.setEnabled(False)

    def limitsChanged(self, param, limits):
        # set up forward / reverse mappings for name:value

        if len(limits) == 0:
            limits = ['']  ## Can never have an empty list--there is always at least a singhe blank item.

        self.forward, self.reverse = ListParameter.mapping(limits)
        try:
            self.widget.blockSignals(True)
            val = self.targetValue  # asUnicode(self.widget.currentText())

            self.widget.clear()
            for k in self.forward:
                self.widget.addItem(k)
                if k == val:
                    self.widget.setCurrentIndex(self.widget.count() - 1)
                    self.updateDisplayLabel()
        finally:
            self.widget.blockSignals(False)

    def clear(self, *even):
        if even[0] is None:
            # self.widget.clear()
            self.updateDisplayLabel('')
        else:
            self.setValue(even[0])
            self.updateDisplayLabel()

class KxListParameter(KxParameter):
    itemClass = KxListParameterItem
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
        self.setLimits(opts['limits'])

    def setLimits(self, limits):
        self.forward, self.reverse = self.mapping(limits)
        self.sigResetlimits.emit(None)
        # self.clearlimits()
        KxParameter.setLimits(self, limits)
        # TODO:CHANGE BY HYH 2017/8/18
        if str(self.value()).isdigit():
            if self.value() is not None:
                value = int(self.value())
            else:
                # value = self.value()
                value = 0
        else:
            value = self.value()
        if len(self.reverse[0]) > 0 and value not in self.reverse[0]:
            # self.setValue(self.reverse[0][0])
            self.setValue(value)
            self.sigResetlimits.emit(self.reverse[0][0])
        else:
            self.sigResetlimits.emit(self.value())

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


kxregisterParameterType('list', KxListParameter, override=True)


class KxActionParameterItem(ParameterItem):
    def __init__(self, param, depth):
        ParameterItem.__init__(self, param, depth)
        param.sigOptionsChanged.connect(self.setreadonly)
        self.layoutWidget = QtWidgets.QWidget()
        self.layout = QtWidgets.QHBoxLayout()
        self.layoutWidget.setLayout(self.layout)
        self.button = QtWidgets.QPushButton(param.name())
        # self.layout.addSpacing(100)
        self.layout.addWidget(self.button)
        #         self.layout.addStretch()
        self.button.clicked.connect(self.buttonClicked)
        param.sigNameChanged.connect(self.paramRenamed)
        self.setText(0, '')

    def treeWidgetChanged(self):
        ParameterItem.treeWidgetChanged(self)
        tree = self.treeWidget()
        if tree is None:
            return

        tree.setFirstItemColumnSpanned(self, True)
        tree.setItemWidget(self, 0, self.layoutWidget)

    def paramRenamed(self, param, name):
        self.button.setText(name)

    def buttonClicked(self):
        self.param.activate()

    def setreadonly(self, param, opts):
        if param is None:
            if 'lock' in opts:
                if opts['lock'] is True:
                    self.button.setEnabled(False)
                else:
                    self.button.setEnabled(True)

class KxActionParameter(KxParameter):
    """Used for displaying a button within the tree."""
    itemClass = KxActionParameterItem
    sigActivated = QtCore.pyqtSignal(object)

    def activate(self):
        self.sigActivated.emit(self)
        self.emitStateChanged('activated', None)

    def setReadonly(self, readonly=True):
        self.sigOptionsChanged.emit(None, {'lock': True})

    def setWritable(self, writable=True):
        self.sigOptionsChanged.emit(None, {'lock': False})

kxregisterParameterType('action', KxActionParameter, override=True)


class ZScheckbox(QtWidgets.QCheckBox):
    sigvaluechange = QtCore.pyqtSignal()
    def __init__(self):
        super(ZScheckbox, self).__init__()
        self.stateChanged.connect(self.sig)

    def sig(self):
        self.sigvaluechange.emit()


class KxRoiParameterItem(WidgetParameterItem):
    """
    WidgetParameterItem subclass providing comboBox that lets the user select from a list of options.

    """

    # sigKxValueChange = QtCore.Signal(object, object)

    def __init__(self, param, depth):
        self.h_checkbox = QtWidgets.QCheckBox()
        #self.h_checkbox = ZScheckbox()
        self.h_label = QtWidgets.QLineEdit("roi_pos")
        WidgetParameterItem.__init__(self, param, depth)
        self.init_info_visible()


    def makeWidget(self):
        """
        Return a single widget that should be placed in the second tree column.
        The widget must be given three attributes:

        ==========  ============================================================
        sigChanged  a signal that is emitted when the checkbox's value is changed
        value       a function that returns the value [bool,str]
        setValue    a function that sets the value [bool,str]
        ==========  ============================================================

        This is a good function to override in subclasses.
        """
        opts = self.param.opts

        layout = QtWidgets.QHBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        h_widget = QtWidgets.QWidget()

        # self.h_label = QtGui.QLineEdit("roi_info")
        # self.h_label.textChanged.connect(self.kx_valuechange())
        # self.h_checkbox = QtGui.QCheckBox()
        # self.h_checkbox.toggled.connect(self.kx_valuechange)

        self.h_label.setReadOnly(True)
        self.h_checkbox.setEnabled(not opts.get('readonly', False))

        layout.addWidget(self.h_checkbox)
        layout.addWidget(self.h_label)
        h_widget.setLayout(layout)
        h_widget.sigChanged = self.h_checkbox.toggled
        #h_widget.sigChanged = self.h_checkbox.sigvaluechange# HYH 20220324 改动
        h_widget.value = self.kx_value
        h_widget.setValue = self.kx_setvalue

        self.hideWidget = False
        return h_widget


    def init_info_visible(self, bool_visible=False):
        """
        用于设置信息是否可见,使用"infovisible":True
        默认不可见
        :param bool_visible:
        :return:
        """
        if "infovisible" in self.param.opts:
            bool_visible = self.param.opts["infovisible"]
        self.h_label.setVisible(bool_visible)

    def valueChanged(self, param, val, force=False):
        ## called when the parameter's value has changed
        #         ParameterItem.valueChanged(self, param, val)
        #         self.widget.sigChanged.disconnect(self.widgetValueChanged)
        ParameterItem.valueChanged(self, param, val)
        #if self.widget.sigChanged.
        try:
            self.widget.sigChanged.disconnect(self.widgetValueChanged)
        except TypeError as e:
            print(e)

        try:
            if force or val != self.kx_value():
                if "isShow" in val:
                    if isinstance(val["isShow"], str):  # 从文件中读取回刷时势unicoide问题
                        val["isShow"] = val["isShow"].lower() == u"true"
                    self.h_checkbox.setChecked(val["isShow"])
                if "pos" in val:
                    self.h_label.setText(val["pos"])
                    # else:
                    #     print "There are somting wrong with valueChange in KxRoiParameterItem"
            self.updateDisplayLabel(val)  ## always make sure label is updated, even if values match!
        finally:
            self.widget.sigChanged.connect(self.widgetValueChanged)
        self.updateDefaultBtn()


    def widgetValueChanged(self):
        ## called when the widget's value has been changed by the user
        val = self.kx_value()
        newVal = self.param.setValue(val)

    # def kx_valuechange(self):
    #     bool_value = self.h_checkbox.isChecked
    #     s_value = unicode(self.h_label.text())
    #     self.sigKxValueChange.emit([bool_value, s_value])
    #
    def kx_value(self):
        bool_value = self.h_checkbox.isChecked()
        s_value = self.h_label.text()
        return {"isShow": bool_value, "pos": s_value}

    #
    def kx_setvalue(self, bool_v=None, s_v=None):
        # if bool_v is not None:
        #     self.h_checkbox.setChecked(bool_v)
        # if s_v is not None:
        #     self.h_label.setText(unicode(s_v))
        print ("kx_setvalue")


class KxRoiParameter(KxParameter):
    '''
    roi 需要有set_roi_pos接口
    '''
    itemClass = KxRoiParameterItem
    sigposchanged = QtCore.Signal(object)

    def __init__(self, *args, **kargs):
        if "roi_opt" in kargs:
            roi_opt = kargs["roi_opt"]
            if kargs["type"] == "doubleroi":
                self.roi = KxDoubleRoiItem(**roi_opt)
            elif kargs["type"] == "roiwithline":
                self.roi = KxRoiWithLineItem(**roi_opt)
            elif kargs["type"] == "roiwithtext":
                self.roi = ROIwithText(**roi_opt)

            self.roi.sigposchanged.connect(self.roi_region_changed)

        self.h_viewbox = None
        self.maxbounds = None

        KxParameter.__init__(self, *args, **kargs)
        # print ("----------------2---------------")


    def set_roi(self, roi):
        self.roi = roi
        self.roi.sigposchanged.connect(self.roi_region_changed)

    def get_roi(self):
        return self.roi

    def add2view(self, h_view):
        if self.roi not in h_view.scene().items():
            h_view.addItem(self.roi)
            self.h_viewbox = h_view
            if "ismaxbounds" in self.opts:
                if self.opts["ismaxbounds"]:
                    self.h_viewbox.scene().changed.connect(self.sceneChanged)
            if self.opts["type"] == "doubleroi":  #doubleroi 没有viewbox 需要手动调用
                self.h_viewbox.sigRangeChangedManually.connect(self.update_doubleroi_handles)
        self.set_roi_visible(self.value_translate(self.opts['value']))

    def update_doubleroi_handles(self):
        '''
        doubleroi 没有viewbox 需要手动调用
        更新handles大小
        :return:
        '''
        for handle in self.roi.roi_outside.handles:
            handle['item'].viewTransformChanged()
        for handle in self.roi.roi_inside.handles:
            handle['item'].viewTransformChanged()

    def sceneChanged(self):
        """
        场景变化中目前只用在设置roi的maxBounds,变动太频繁,会影响刷新,自动断开
        :return:
        """
        list_shape = []
        if "ismaxbounds" in self.opts:
            ismaxbounds = self.opts["ismaxbounds"]
            if ismaxbounds:
                # 最大范围添加至roi中
                for item in self.h_viewbox.scene().items():
                    if isinstance(item, pg.ImageItem):
                        if item.image is not None:
                            list_shape = item.image.shape
                        break
                if (list_shape == self.maxbounds) or (list_shape == []):
                    return
                else:
                    self.maxbounds = list_shape
                    self.setbounds(self.maxbounds)
                    self.h_viewbox.scene().changed.disconnect(self.sceneChanged)

    def setbounds(self, image_shape):
        y, x = image_shape[:2]
        x = int(x)
        y = int(y)
        pointlt = self.roi.mapToScene(QtCore.QPoint(0, 0))
        pointrb = self.roi.mapToScene(QtCore.QPoint(x, y))
        if hasattr(self.roi, "roi_outside"):
            self.roi.roi_outside.maxBounds = QtCore.QRectF(pointlt, pointrb)
        else:
            self.roi.maxBounds = QtCore.QRectF(pointlt, pointrb)

    def get_list_pos(self):
        return self.roi.get_list_pos()

    def set_list_pos(self, list_pos):
        self.roi.set_list_pos(list_pos)

    def roi_region_changed(self):
        # pos = self.roi.pos()
        # size = self.roi.size()
        #
        # x1 = unicode(int(pos[0]))
        # y1 = unicode(int(pos[1]))
        # x2 = unicode(int(pos[0]) + int(size[0]))
        # y2 = unicode(int(pos[1]) + int(size[1]))
        s_info = self.roi.get_str_pos()

        # 无法直接获得checkbox的值,通过这种方式获得
        if "isShow" in self.value():
            bool_value = self.value()["isShow"]
        else:
            print ("roi_region_changed------------isShow")
        self.setValue({"isShow": bool_value, "pos": s_info})
        self.sigposchanged.emit(self)

    def set_roi_visible(self, bool_visible):
        if bool_visible is not self.roi.isVisible():
            #
            self.roi.setVisible(bool_visible)

    def setValue(self, value, blockSignal=None):
        """
        Set the value of this Parameter; return the actual value that was set.
        (this may be different from the value that was requested)
        """
        try:
            if blockSignal is not None:
                self.sigValueChanged.disconnect(blockSignal)
            if self.opts['value'] == value:
                return value
            self.set_roi_visible(self.value_translate(value))  # 设置roi可见
            self.opts['value'] = value
            self.set_roi_pos()  # 以界面上的位置信息为标准
            self.sigValueChanged.emit(self, value)
        finally:
            if blockSignal is not None:
                self.sigValueChanged.connect(blockSignal)

    def value_translate(self, value):
        """
        用于判断显示roi的操作,参数树回刷字符串导致的问题
        :param opt_v:
        :param value:
        :return:
        """
        bool_new_value = value
        if "isShow" in value:
            bool_new_value = value["isShow"]

            if isinstance(bool_new_value, str):
                if bool_new_value.lower() == "true":
                    bool_new_value = True
                else:
                    bool_new_value = False
        return bool_new_value

    def set_roi_pos(self):
        # print ('in', self.value())
        if "pos" in self.value():
            # pos = self.roi.pos()
            # size = self.roi.size()
            #
            # x1 = unicode(int(pos[0]))
            # y1 = unicode(int(pos[1]))
            # x2 = unicode(int(pos[0]) + int(size[0]))
            # y2 = unicode(int(pos[1]) + int(size[1]))
            s_info = self.roi.get_str_pos()
            if s_info != self.value()["pos"]:
                list_pos = self.value()["pos"].split(",")
                list_pos = [int(i) for i in list_pos]
                self.roi.set_list_pos(list_pos)
                # w = list_pos[2] - list_pos[0]
                # h = list_pos[3] - list_pos[1]
                # self.roi.setPos(list_pos[0], list_pos[1])
                # self.roi.setSize(w,h)

    def setinfovisible(self, bool_visible=False):
        self.siginfovisible.emit(bool_visible)

    def isShow(self, bool_show):
        s_info = self.roi.get_str_pos()

        if (self.value()["isShow"] != bool_show) or \
                (self.value()["isShow"] != bool_show) :

            self.setValue({"isShow": bool_show, "pos": s_info})

kxregisterParameterType('doubleroi', KxRoiParameter, override=True)
kxregisterParameterType('roiwithline', KxRoiParameter, override=True)
kxregisterParameterType('roiwithtext', KxRoiParameter, override=True)


# --------------------------------------- 2020.06.03 面检新加 ------------------------------------------------------ #


class KxBoundingParameterItem(WidgetParameterItem):
    """
    WidgetParameterItem subclass providing comboBox that lets the user select from a list of options.

    """

    # sigKxValueChange = QtCore.Signal(object, object)

    def __init__(self, param, depth):
        self.h_checkbox = QtGui.QCheckBox()
        self.h_label = QtGui.QLineEdit("roi_pos")
        WidgetParameterItem.__init__(self, param, depth)
        self.init_info_visible()

    def makeWidget(self):
        """
        Return a single widget that should be placed in the second tree column.
        The widget must be given three attributes:

        ==========  ============================================================
        sigChanged  a signal that is emitted when the checkbox's value is changed
        value       a function that returns the value [bool,str]
        setValue    a function that sets the value [bool,str]
        ==========  ============================================================

        This is a good function to override in subclasses.
        """
        opts = self.param.opts

        layout = QtGui.QHBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        h_widget = QtGui.QWidget()

        # self.h_label = QtGui.QLineEdit("roi_info")
        # self.h_label.textChanged.connect(self.kx_valuechange())
        # self.h_checkbox = QtGui.QCheckBox()
        # self.h_checkbox.toggled.connect(self.kx_valuechange)

        self.h_label.setReadOnly(True)
        self.h_checkbox.setEnabled(not opts.get('readonly', False))

        layout.addWidget(self.h_checkbox)
        layout.addWidget(self.h_label)
        h_widget.setLayout(layout)
        h_widget.sigChanged = self.h_checkbox.toggled
        h_widget.value = self.kx_value
        h_widget.setValue = self.kx_setvalue

        self.hideWidget = False

        return h_widget

    def init_info_visible(self, bool_visible=False):
        """
        用于设置信息是否可见,使用"infovisible":True
        默认不可见
        :param bool_visible:
        :return:
        """
        if "infovisible" in self.param.opts:
            bool_visible = self.param.opts["infovisible"]
        self.h_label.setVisible(bool_visible)

    def valueChanged(self, param, val, force=False):
        ## called when the parameter's value has changed
        #         ParameterItem.valueChanged(self, param, val)
        #         self.widget.sigChanged.disconnect(self.widgetValueChanged)

        ParameterItem.valueChanged(self, param, val)
        self.widget.sigChanged.disconnect(self.widgetValueChanged)

        try:
            if force or val != self.kx_value():
                if "isShow" in val:
                    if isinstance(val["isShow"], str):  # 从文件中读取回刷时势unicoide问题
                        val["isShow"] = (val["isShow"] == "True") or (val["isShow"] == "True")
                    self.h_checkbox.setChecked(val["isShow"])
                if "pos" in val:
                    self.h_label.setText(val["pos"])
            self.updateDisplayLabel(val)  ## always make sure label is updated, even if values match!
        finally:
            self.widget.sigChanged.connect(self.widgetValueChanged)
        self.updateDefaultBtn()

    def widgetValueChanged(self):
        ## called when the widget's value has been changed by the user
        val = self.kx_value()
        newVal = self.param.setValue(val)

    # def kx_valuechange(self):
    #     bool_value = self.h_checkbox.isChecked
    #     s_value = unicode(self.h_label.text())
    #     self.sigKxValueChange.emit([bool_value, s_value])
    #
    def kx_value(self):
        bool_value = self.h_checkbox.isChecked()
        s_value = str(self.h_label.text())
        return {"isShow": bool_value, "pos": s_value}

    #
    def kx_setvalue(self, bool_v=None, s_v=None):
        # if bool_v is not None:
        #     self.h_checkbox.setChecked(bool_v)
        # if s_v is not None:
        #     self.h_label.setText(unicode(s_v))
        pass


class KxBoundingParameter(KxParameter):
    '''
    roi 需要有set_roi_pos接口
    '''
    itemClass = KxBoundingParameterItem
    sigposchanged = QtCore.Signal(object)

    def __init__(self, *args, **kargs):
        KxParameter.__init__(self, *args, **kargs)

        roi_opt = {}
        if "roi_opt" in self.opts:
            roi_opt = self.opts["roi_opt"]
        self.v_linear = KxLinearRegionItem(orientation=KxLinearRegionItem.Vertical, **roi_opt)
        self.v_linear.setlinebound([0, 1280])
        self.h_linear = KxLinearRegionItem(orientation=KxLinearRegionItem.Horizontal, **roi_opt)
        self.h_linear.setlinebound([0, 1920])
        self.v_linear.sigRegionChangeFinished.connect(self.roi_region_changed)
        self.h_linear.sigRegionChangeFinished.connect(self.roi_region_changed)

        if self.opts["type"] == "crossinglines":
            self.crossingmode()
        self.h_viewbox = None
        self.maxbounds = None

    def crossingmode(self):
        """
        用于只要一横一竖情况,将线隐藏
        :return:
        """
        if self.v_linear.lines[1].isVisible():
            self.v_linear.lines[1].setVisible(False)
        if self.h_linear.lines[1].isVisible():
            self.h_linear.lines[1].setVisible(False)

    # def set_roi(self, roi):
    #     self.roi = roi
    #     self.roi.sigposchanged.connect(self.roi_region_changed)

    def get_roi(self):
        return self.v_linear, self.h_linear

    def add2view(self, h_view):
        if self.v_linear not in list(h_view.scene().items()):
            h_view.addItem(self.v_linear)
        if self.h_linear not in list(h_view.scene().items()):
            h_view.addItem(self.h_linear)

        self.h_viewbox = h_view
        if "ismaxbounds" in self.opts:
            if self.opts["ismaxbounds"]:
                h_view.scene().changed.connect(self.sceneChanged)
        self.set_roi_visible(self.value_translate(self.opts['value']))

    def sceneChanged(self):
        """
        场景变化中目前只用在设置roi的maxBounds,变动太频繁,会影响刷新,自动断开
        :return:
        """
        list_shape = []
        if "ismaxbounds" in self.opts:
            ismaxbounds = self.opts["ismaxbounds"]
            if ismaxbounds:
                # 最大范围添加至roi中
                for item in list(self.h_viewbox.scene().items()):
                    if isinstance(item, pg.ImageItem):
                        if item.image is not None:
                            list_shape = item.image.shape
                        break
                if (list_shape == self.maxbounds) or (list_shape == []):
                    return
                else:
                    self.maxbounds = list_shape
                    self.setbounds(self.maxbounds)
                    self.h_viewbox.scene().changed.disconnect(self.sceneChanged)

    def setbounds(self, image_shape):
        y, x = image_shape[:2]
        x = int(x)
        y = int(y)

        self.h_linear.setBounds([0, y])
        self.v_linear.setBounds([0, x])
        self.h_linear.setlinebound([x, 0])
        self.v_linear.setlinebound([y, 0])

    def get_list_region(self):
        """
        获得位置信息
        :return: [x1,x2,y1,y2]
        """
        tuple_v_region = self.v_linear.getRegion()
        tuple_h_region = self.h_linear.getRegion()
        list_info = [tuple_v_region[0], tuple_v_region[1], \
                     tuple_h_region[0], tuple_h_region[1]]
        list_info = [str(int(i)) for i in list_info]
        return list_info

    def get_str_region(self):
        list_info = self.get_list_region()
        s_info = ",".join(list_info)
        return s_info

    def roi_region_changed(self):
        # pos = self.roi.pos()
        # size = self.roi.size()
        #
        # x1 = unicode(int(pos[0]))
        # y1 = unicode(int(pos[1]))
        # x2 = unicode(int(pos[0]) + int(size[0]))
        # y2 = unicode(int(pos[1]) + int(size[1]))

        s_info = self.get_str_region()

        # 无法直接获得checkbox的值,通过这种方式获得
        if "isShow" in self.value():
            bool_value = self.value()["isShow"]
        else:
            pass
        self.setValue({"isShow": bool_value, "pos": s_info})
        self.sigposchanged.emit(self)

    def set_roi_visible(self, bool_visible):
        if bool_visible is not self.v_linear.isVisible():
            self.v_linear.setVisible(bool_visible)
        if bool_visible is not self.h_linear.isVisible():
            self.h_linear.setVisible(bool_visible)

        if self.opts["type"] == "crossinglines":
            self.crossingmode()

    def setValue(self, value, blockSignal=None):
        """
        Set the value of this Parameter; return the actual value that was set.
        (this may be different from the value that was requested)
        """
        try:
            if blockSignal is not None:
                self.sigValueChanged.disconnect(blockSignal)
            if self.opts['value'] == value:
                return value
            self.set_roi_visible(self.value_translate(value))  # 设置roi可见
            self.opts['value'] = value
            self.set_roi_pos()  # 以界面上的位置信息为标准
            self.sigValueChanged.emit(self, value)
        finally:
            if blockSignal is not None:
                self.sigValueChanged.connect(blockSignal)

    def value_translate(self, value):
        """
        用于判断显示roi的操作,参数树回刷字符串导致的问题
        :param opt_v:
        :param value:
        :return:
        """
        bool_new_value = value
        if "isShow" in value:
            bool_new_value = value["isShow"]

            if isinstance(bool_new_value, str):
                bool_new_value = (bool_new_value == "True") \
                                 or (bool_new_value == "True") \
                                 or (bool_new_value == "True")  # 有时上面两句判断错误,加这句保险
        return bool_new_value

    def set_roi_pos(self):
        if "pos" in self.value():
            # pos = self.roi.pos()
            # size = self.roi.size()
            #
            # x1 = unicode(int(pos[0]))
            # y1 = unicode(int(pos[1]))
            # x2 = unicode(int(pos[0]) + int(size[0]))
            # y2 = unicode(int(pos[1]) + int(size[1]))
            s_info = self.get_str_region()
            if s_info != self.value()["pos"]:
                list_pos = self.value()["pos"].split(",")
                list_pos = [int(i) for i in list_pos]
                self.v_linear.setRegion(list_pos[:2])
                self.h_linear.setRegion(list_pos[2:4])
                # w = list_pos[2] - list_pos[0]
                # h = list_pos[3] - list_pos[1]
                # self.roi.setPos(list_pos[0], list_pos[1])
                # self.roi.setSize(w,h)


kxregisterParameterType('boundinglines', KxBoundingParameter, override=True)
kxregisterParameterType('crossinglines', KxBoundingParameter, override=True)


class KxImageInfoParameterItem(WidgetParameterItem):
    """
    WidgetParameterItem subclass providing comboBox that lets the user select from a list of options.

    """

    # sigKxValueChange = QtCore.Signal(object, object)

    def __init__(self, param, depth):
        self.h_checkbox = QtGui.QCheckBox()
        self.h_label = QtGui.QLineEdit("image_info")
        WidgetParameterItem.__init__(self, param, depth)
        self.init_info_visible()

    def makeWidget(self):
        """
        Return a single widget that should be placed in the second tree column.
        The widget must be given three attributes:

        ==========  ============================================================
        sigChanged  a signal that is emitted when the checkbox's value is changed
        value       a function that returns the value [bool,str]
        setValue    a function that sets the value [bool,str]
        ==========  ============================================================

        This is a good function to override in subclasses.
        """
        opts = self.param.opts

        layout = QtGui.QHBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        h_widget = QtGui.QWidget()

        # self.h_label = QtGui.QLineEdit("roi_info")
        # self.h_label.textChanged.connect(self.kx_valuechange())
        # self.h_checkbox = QtGui.QCheckBox()
        # self.h_checkbox.toggled.connect(self.kx_valuechange)

        self.h_label.setReadOnly(True)
        self.h_checkbox.setEnabled(not opts.get('readonly', False))

        layout.addWidget(self.h_checkbox)
        layout.addWidget(self.h_label)
        h_widget.setLayout(layout)
        h_widget.sigChanged = self.h_checkbox.toggled
        h_widget.value = self.kx_value
        h_widget.setValue = self.kx_setvalue

        self.hideWidget = False

        return h_widget

    def init_info_visible(self, bool_visible=False):
        """
        用于设置信息是否可见,使用"infovisible":True
        默认不可见
        :param bool_visible:
        :return:
        """
        if "infovisible" in self.param.opts:
            bool_visible = self.param.opts["infovisible"]
        self.h_label.setVisible(bool_visible)

    def valueChanged(self, param, val, force=False):
        ## called when the parameter's value has changed
        #         ParameterItem.valueChanged(self, param, val)
        #         self.widget.sigChanged.disconnect(self.widgetValueChanged)

        ParameterItem.valueChanged(self, param, val)
        self.widget.sigChanged.disconnect(self.widgetValueChanged)

        try:
            if force or val != self.kx_value():
                if "isShow" in val:
                    if isinstance(val["isShow"], str):  # 从文件中读取回刷时势unicoide问题
                        val["isShow"] = (val["isShow"] == "True") or (val["isShow"] == "True")
                    self.h_checkbox.setChecked(val["isShow"])
                if "pos" in val:
                    self.h_label.setText(val["pos"])
            self.updateDisplayLabel(val)  ## always make sure label is updated, even if values match!
        finally:
            self.widget.sigChanged.connect(self.widgetValueChanged)
        self.updateDefaultBtn()

    def widgetValueChanged(self):
        ## called when the widget's value has been changed by the user
        val = self.kx_value()
        newVal = self.param.setValue(val)

    # def kx_valuechange(self):
    #     bool_value = self.h_checkbox.isChecked
    #     s_value = unicode(self.h_label.text())
    #     self.sigKxValueChange.emit([bool_value, s_value])
    #
    def kx_value(self):
        bool_value = self.h_checkbox.isChecked()
        s_value = str(self.h_label.text())
        return {"isShow": bool_value, "pos": s_value}

    #
    def kx_setvalue(self, bool_v=None, s_v=None):
        # if bool_v is not None:
        #     self.h_checkbox.setChecked(bool_v)
        # if s_v is not None:
        #     self.h_label.setText(unicode(s_v))
        pass


class KxImageInfoParameter(KxParameter):
    '''
    add2view(self, h_GraphicsView):
        需要添加的是pg.GraphicsView
    '''
    itemClass = KxImageInfoParameterItem

    def __init__(self, *args, **kargs):
        KxParameter.__init__(self, *args, **kargs)
        self.h_textItem = pg.TextItem(anchor=(0, 0), angle=0, color=(255, 0, 0))
        h_font = QtGui.QFont('Consolas', 15)
        self.h_textItem.setFont(h_font)
        self.h_view = None
        self.h_imgitem = None
        self.resolutionList = [1000, 1000]

    def add2view(self, h_GraphicsView, h_imgitem):
        '''
        需要添加的是pg.GraphicsView
        :param h_GraphicsView:
        :return:
        '''
        self.h_view = h_GraphicsView
        self.h_imgitem = h_imgitem
        if self.h_textItem not in list(self.h_view.scene().items()):
            h_GraphicsView.addItem(self.h_textItem)
            self.h_view.scene().sigMouseMoved.connect(self.textchange)
        self.set_roi_visible(self.value_translate(self.opts['value']))

    def setResolution(self, x_resolution, y_resolution):
        self.resolutionList = [x_resolution, y_resolution]

    def textchange(self, even):
        mousePoint = self.h_view.centralWidget.mapSceneToView(even)
        x = int(mousePoint.x())
        y = int(mousePoint.y())

        h_imgitem = self.h_imgitem

        s_pos = 'X:%.1fmm' % (x * self.resolutionList[0] * 0.001) + "  " + 'Y:%.1fmm' % (
                    y * self.resolutionList[1] * 0.001) + '  Pos:(%d, %d)' % (x, y)
        if h_imgitem is not None and h_imgitem.image is not None:
            shape = h_imgitem.image.shape
            if x >= 0 and y >= 0 and x < shape[1] and y < shape[0]:
                n_value = h_imgitem.image[y, x]
                if isinstance(n_value, np.uint8) is False:
                    s_pos = 'X:%.1fmm' % (x * self.resolutionList[0] * 0.001) + "  " + \
                            'Y:%.1fmm' % (y * self.resolutionList[1] * 0.001) + "  " + "[R,G,B]:" + str(
                        n_value) + '  Pos:(%d, %d)' % (x, y)
                else:
                    s_pos = 'Pos:(%d, %d)' % (x, y) + 'X:%.1fmm' % (x * self.resolutionList[0] * 0.001) + "  " + \
                            'Y:%.1fmm' % (y * self.resolutionList[1] * 0.001) + "  " + "[Gray]:" + str(
                        n_value) + '  Pos:(%d, %d)' % (x, y)
        self.h_textItem.setText(s_pos)

    def set_roi_visible(self, bool_visible):
        pass
        # if bool_visible is not self.h_textItem.isVisible():
        #     self.h_textItem.setVisible(bool_visible)

    def setValue(self, value, blockSignal=None):
        """
        Set the value of this Parameter; return the actual value that was set.
        (this may be different from the value that was requested)
        """
        try:
            if blockSignal is not None:
                self.sigValueChanged.disconnect(blockSignal)
            if self.opts['value'] == value:
                return value
            self.set_roi_visible(self.value_translate(value))  # 设置roi可见
            self.opts['value'] = value
            self.sigValueChanged.emit(self, value)
        finally:
            if blockSignal is not None:
                self.sigValueChanged.connect(blockSignal)

    def value_translate(self, value):
        """
        用于判断显示roi的操作,参数树回刷字符串导致的问题
        :param opt_v:
        :param value:
        :return:
        """
        bool_new_value = value
        if "isShow" in value:
            bool_new_value = value["isShow"]

            if isinstance(bool_new_value, str):
                bool_new_value = (bool_new_value == "True") \
                                 or (bool_new_value == "True") \
                                 or (bool_new_value == "True")  # 有时上面两句判断错误,加这句保险
        return bool_new_value


kxregisterParameterType('imageinfo', KxImageInfoParameter, override=True)


class KxLineRegionComboWidget(QtWidgets.QWidget):
    sigValChanged = QtCore.pyqtSignal()

    def __init__(self, min=0, max=10000, suffix='', parent=None):
        super(KxLineRegionComboWidget, self).__init__(parent)
        self.h_checkbox = QtGui.QCheckBox(self)
        self.h_valsLabel = QtGui.QLineEdit(self)
        self.h_intervalLabel = QtGui.QDoubleSpinBox(self)
        self.h_intervalLabel.setRange(min, max)
        self.h_intervalLabel.setSuffix(suffix)
        self.h_intervalLabel.setDecimals(1)

        layout = QtGui.QHBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.addWidget(self.h_checkbox, 1)
        layout.addWidget(self.h_intervalLabel, 3)
        layout.addWidget(self.h_valsLabel, 5)

        self.h_valsLabel.setReadOnly(True)
        self.h_valsLabel.setVisible(False)

        self.h_intervalLabel.valueChanged.connect(self.slotIntervalChanged)
        self.h_checkbox.stateChanged.connect(self.sigValChanged)

    def slotIntervalChanged(self, curInterval):
        curVals = str(self.h_valsLabel.text()).split(',')
        curVals = [float('%.1f' % float(val)) for val in curVals]
        if len(curVals) >= 2:
            curVals[1] = curVals[0] + curInterval - 1
        curVals = [str(val) for val in curVals]
        self.h_valsLabel.setText(','.join(curVals))
        self.sigValChanged.emit()

    def setLabelReadOnly(self, isReadonly):
        self.h_intervalLabel.setEnabled(not isReadonly)

    def setLabelVisible(self, isVisible):
        self.h_intervalLabel.setVisible(isVisible)

    def setCheckBoxEnabled(self, isEnabled):
        self.h_checkbox.setEnabled(isEnabled)

    def setIntervalLabelText(self, text):
        self.h_intervalLabel.setValue(float('%.1f' % float(text)))

    def setValsLabelText(self, text):
        self.h_valsLabel.setText(text)

    def getIntervalLabelText(self):
        return str(self.h_intervalLabel.value())

    def getValsLabelText(self):
        return self.h_valsLabel.text()

    def setCheckState(self, isCheck):
        self.h_checkbox.setChecked(isCheck)

    def getIsCheckboxChecked(self):
        return self.h_checkbox.isChecked()


class KxLineRegionParameterItem(WidgetParameterItem):
    """
    WidgetParameterItem subclass providing comboBox that lets the user select from a list of options.

    """

    def __init__(self, param, depth):
        WidgetParameterItem.__init__(self, param, depth)
        self.init_info_visible()

    def makeWidget(self):
        opts = self.param.opts

        h_widget = QtGui.QWidget()
        layout = QtGui.QHBoxLayout(h_widget)
        layout.setContentsMargins(0, 0, 0, 0)

        widget_opt = {}
        if opts.get('widget_opt', None):
            widget_opt = opts['widget_opt']
        widget_opt['parent'] = h_widget

        self.comboWidget = KxLineRegionComboWidget(**widget_opt)
        self.comboWidget.setLabelReadOnly(opts.get('readonly', False))
        self.comboWidget.setCheckBoxEnabled(not opts.get('readonly', False))

        layout.addWidget(self.comboWidget)
        h_widget.sigChanged = self.comboWidget.sigValChanged
        h_widget.value = self.kx_value
        h_widget.setValue = self.kx_setvalue

        self.hideWidget = True

        return h_widget

    def init_info_visible(self, bool_visible=False):
        if "infovisible" in self.param.opts:
            bool_visible = self.param.opts["infovisible"]
        self.comboWidget.setLabelVisible(bool_visible)

    def valueChanged(self, param, val, force=False):

        ParameterItem.valueChanged(self, param, val)
        self.widget.sigChanged.disconnect(self.widgetValueChanged)

        try:
            if force or val != self.kx_value():
                if "lineFixed" in val:
                    if isinstance(val["lineFixed"], str):  # 从文件中读取回刷时势unicoide问题
                        val["lineFixed"] = (val["lineFixed"] == "True") or (val["lineFixed"] == "True")
                    self.comboWidget.setCheckState(val["lineFixed"])
                if "values" in val:
                    self.comboWidget.setValsLabelText(val["values"])
                if "interval" in val:
                    self.comboWidget.setIntervalLabelText(val["interval"])
            self.updateDisplayLabel(val)  ## always make sure label is updated, even if values match!
        finally:
            self.widget.sigChanged.connect(self.widgetValueChanged)
        self.updateDefaultBtn()

    def widgetValueChanged(self):
        ## called when the widget's value has been changed by the user
        val = self.kx_value()
        newVal = self.param.setValue(val)

    def kx_value(self):
        bool_value = self.comboWidget.getIsCheckboxChecked()
        s_value = str(self.comboWidget.getValsLabelText())
        s_interval = str(self.comboWidget.getIntervalLabelText())
        return {"lineFixed": bool_value, "values": s_value, "interval": s_interval}

    def kx_setvalue(self, bool_v=None, s_v=None):
        pass

    def updateDisplayLabel(self, value=None):
        """Update the display label to reflect the value of the parameter."""
        text = self.comboWidget.getIntervalLabelText()
        self.displayLabel.setText(text + 'mm')


class KxLineRegionParameter(KxParameter):
    '''
    roi 需要有set_roi_pos接口
    '''
    itemClass = KxLineRegionParameterItem
    sigposchanged = QtCore.Signal(object)

    def __init__(self, *args, **kargs):
        KxParameter.__init__(self, *args, **kargs)

        roi_opt = self.opts.get("roi_opt", {})
        initVals = self.opts['value'].get('values', None)
        if initVals:
            initVals = initVals.split(',')
            initVals = [float(val) for val in initVals]
            roi_opt['values'] = initVals
        self.roi = KxLineRegionItem(**roi_opt)
        self.roi.sigLineRegionChange.connect(self.roi_region_changed)

        self.h_viewbox = None
        self.maxbounds = None

    def set_roi(self, roi):
        self.roi = roi
        self.roi.sigLineRegionChange.connect(self.roi_region_changed)

    def get_roi(self):
        return self.roi

    def add2view(self, h_view):
        if self.roi not in list(h_view.scene().items()):
            h_view.addItem(self.roi)
            self.h_viewbox = h_view
            #             self.h_viewbox.scene().changed.connect(self.sceneAddPitcure)
            if "ismaxbounds" in self.opts:
                if self.opts["ismaxbounds"]:
                    self.h_viewbox.scene().changed.connect(self.sceneChanged)

        self.set_roi_visible(True)

    #         if 'isShow' in self.opts['value']:
    #             self.set_roi_visible(self.value_translate(self.opts['value']['isShow']))

    def sceneAddPitcure(self):
        # 未加入图片则不显示
        self.set_roi_visible(False)
        for item in list(self.h_viewbox.scene().items()):
            if isinstance(item, pg.ImageItem):
                if item.image is not None:
                    self.set_roi_visible(True)
                #                     value = self.value()
                #                     if 'isShow' in value:
                #                         self.set_roi_visible(self.value_translate(value['isShow']))
                #                     self.h_viewbox.scene().changed.disconnect(self.sceneAddPitcure)
                break
            else:
                self.set_roi_visible(False)

    def sceneChanged(self):
        """
        场景变化中目前只用在设置roi的maxBounds,变动太频繁,会影响刷新,自动断开
        :return:
        """
        list_shape = []
        if "ismaxbounds" in self.opts:
            ismaxbounds = self.opts["ismaxbounds"]
            if ismaxbounds:
                # 最大范围添加至roi中
                for item in list(self.h_viewbox.scene().items()):
                    if isinstance(item, pg.ImageItem):
                        if item.image is not None:
                            list_shape = item.image.shape
                        break
                if (list_shape == self.maxbounds) or (list_shape == []):
                    return
                else:
                    self.maxbounds = list_shape
                    self.setbounds(self.maxbounds)
                    self.h_viewbox.scene().changed.disconnect(self.sceneChanged)

    def setbounds(self, image_shape):
        y, x = image_shape[:2]
        x = int(x)
        y = int(y)
        pointlt = QtCore.QPoint(0, 0)
        pointrb = QtCore.QPoint(x, y)
        if hasattr(self.roi, "roi_outside"):
            self.roi.roi_outside.maxBounds = QtCore.QRectF(pointlt, pointrb)
        else:
            self.roi.maxBounds = QtCore.QRectF(pointlt, pointrb)

    def roi_region_changed(self):
        curRoiVals = self.roi.getValues()
        curRoiVals = [str(i) for i in curRoiVals]
        strVals = ','.join(curRoiVals)
        curInterval = self.roi.getLineInterval()
        strInterval = str(curInterval)
        isFixed = self.value().get("lineFixed", False)
        self.setValue({"lineFixed": isFixed, "values": strVals, "interval": strInterval})
        self.sigposchanged.emit(self)

    def set_roi_visible(self, bool_visible):
        if bool_visible is not self.roi.isVisible():
            self.roi.setVisible(bool_visible)

    def setValue(self, value, blockSignal=None):
        """
        Set the value of this Parameter; return the actual value that was set.
        (this may be different from the value that was requested)
        """
        try:
            if blockSignal is not None:
                self.sigValueChanged.disconnect(blockSignal)
            if self.opts['value'] == value:
                return value
            if 'lineFixed' in value:
                self.setLineFixed(self.value_translate(value['lineFixed']))
            #             if 'isShow' in value:
            #                 self.set_roi_visible(self.value_translate(value['isShow']))
            self.opts['value'] = value
            self.__setLineVals()
            self.sigValueChanged.emit(self, value)
        finally:
            if blockSignal is not None:
                self.sigValueChanged.connect(blockSignal)

    def value_translate(self, value):
        if isinstance(value, str):
            value = (value == "true") or (value == "True") or (value == True)  # 有时上面两句判断错误,加这句保险
        return value

    def __setLineVals(self):
        if "values" in self.opts['value']:
            curRoiVals = self.roi.getValues()
            curRoiVals = [str(i) for i in curRoiVals]
            strVals = ','.join(curRoiVals)

            if strVals != self.opts['value']["values"]:
                strVals = self.opts['value']["values"]
                listVal = strVals.split(",")
                listVal = [float('%.1f' % float(i)) for i in listVal]
                self.roi.setValues(listVal)

    def setLineFixed(self, isFixed):
        if isFixed != self.roi.getIsLineFixed():
            self.roi.setLineFixed(isFixed)

    def getIsLineFix(self):
        return self.roi.getIsLineFixed()

    def setMovable(self, flag):
        self.roi.setAllFixed(not flag)

    def setLineVals(self, vals):
        self.roi.setValues(vals)

    def getLineVals(self):
        return self.roi.getValues()

    def setLineInterval(self, interval):
        if interval != self.roi.getLineInterval():
            self.roi.setLineInterval(interval)

    def getLineInterval(self):
        return self.roi.getLineInterval()

    def setLineLength(self, length):
        self.roi.setLineLength(length)

    def getLineLength(self):
        return self.roi.getLineLength()

    def isShow(self, bool_show):
        self.set_roi_visible(bool_show)

    def getRegion(self):
        return self.roi.getRegion()

    def setRegion(self, vals):
        self.roi.setRegion(vals)

    def setResolution(self, resolution):
        self.roi.setResolution(resolution)

    def setLinePos(self, posVal):
        self.roi.setLinePos(posVal)


kxregisterParameterType('lineregion', KxLineRegionParameter, override=True)


class KxSpinAndButtonItem(WidgetParameterItem):
    def __init__(self, param, depth):
        WidgetParameterItem.__init__(self, param, depth)
        self.hideWidget = False

    def makeWidget(self):
        opts = self.param.opts
        self.w = QtGui.QWidget()
        self.spinBox = SpinBox(self.w)
        defs = {
            'value': 0, 'min': None, 'max': None,
            'step': 1.0, 'bounds': [0, 100], 'int': True
        }

        for k in defs:
            if k in opts:
                defs[k] = opts[k]
        if 'limits' in opts:
            defs['bounds'] = opts['limits']

        self.spinBox.setOpts(**defs)
        self.testBtn = QtGui.QPushButton(self.w)
        self.testBtn.setFixedSize(20, 20)
        self.testBtn.clicked.connect(self.buttonClicked)
        layout = QtGui.QHBoxLayout(self.w)
        layout.setSpacing(100)
        layout.addWidget(self.spinBox, 5)
        layout.addWidget(self.testBtn, 1, QtCore.Qt.AlignHCenter | QtCore.Qt.AlignRight)
        self.w.sigChanged = self.spinBox.sigValueChanged
        self.w.sigChanging = self.spinBox.sigValueChanging
        self.w.value = self.spinBox.value
        self.w.setValue = self.spinBox.setValue

        return self.w

    def buttonClicked(self):
        self.param.activate()


class KxSpinAndButtonParameter(KxParameter):
    """Editable string; displayed as large text box in the tree."""
    itemClass = KxSpinAndButtonItem
    sigActivated = QtCore.Signal(object)

    def activate(self):
        self.sigActivated.emit(self)
        self.emitStateChanged('activated', None)


kxregisterParameterType('spinAndButton', KxSpinAndButtonParameter, override=True)


class KxFiniteLineWidget(QtWidgets.QWidget):
    sigValChanged = QtCore.pyqtSignal()

    def __init__(self, min=None, max=None, parent=None):
        super(KxFiniteLineWidget, self).__init__(parent)
        self.h_valueLabel = QtGui.QSpinBox(self)
        if min is not None:
            self.h_valueLabel.setMinimum(int(min))
        if max is not None:
            self.h_valueLabel.setMaximum(int(max))
        layout = QtGui.QHBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.addWidget(self.h_valueLabel)

        self.h_valueLabel.valueChanged.connect(self.sigValChanged)

    def setLabelReadOnly(self, isReadonly):
        self.h_valueLabel.setEnabled(not isReadonly)

    def setLabelVisible(self, isVisible):
        self.h_valueLabel.setVisible(isVisible)

    def setValsLabelVal(self, text):
        self.h_valueLabel.setValue(int(float(text)))

    def getValsLabelText(self):
        return self.h_valueLabel.value()


class KxFiniteLineParameterItem(WidgetParameterItem):
    """
    WidgetParameterItem subclass providing comboBox that lets the user select from a list of options.

    """

    def __init__(self, param, depth):
        WidgetParameterItem.__init__(self, param, depth)
        self.init_info_visible()

    def makeWidget(self):
        opts = self.param.opts

        h_widget = QtGui.QWidget()
        layout = QtGui.QHBoxLayout(h_widget)
        layout.setContentsMargins(0, 0, 0, 0)

        widget_opt = {}
        if opts.get('widget_opt', None):
            widget_opt = opts['widget_opt']
        widget_opt['parent'] = h_widget

        self.comboWidget = KxFiniteLineWidget(**widget_opt)
        self.comboWidget.setLabelReadOnly(opts.get('readonly', False))

        layout.addWidget(self.comboWidget)
        h_widget.sigChanged = self.comboWidget.sigValChanged
        h_widget.value = self.kx_value
        h_widget.setValue = self.kx_setvalue

        self.hideWidget = True

        return h_widget

    def init_info_visible(self, bool_visible=False):
        if "infovisible" in self.param.opts:
            bool_visible = self.param.opts["infovisible"]
        self.comboWidget.setLabelVisible(bool_visible)

    def valueChanged(self, param, val, force=False):

        ParameterItem.valueChanged(self, param, val)
        self.widget.sigChanged.disconnect(self.widgetValueChanged)

        try:
            if force or val != self.kx_value():
                if "pos" in val:
                    self.comboWidget.setValsLabelVal(val["pos"])
            self.updateDisplayLabel(val)  ## always make sure label is updated, even if values match!
        finally:
            self.widget.sigChanged.connect(self.widgetValueChanged)
        self.updateDefaultBtn()

    def widgetValueChanged(self):
        ## called when the widget's value has been changed by the user
        val = self.kx_value()
        newVal = self.param.setValue(val)

    def kx_value(self):
        value = self.comboWidget.getValsLabelText()
        return {"pos": value}

    def kx_setvalue(self, bool_v=None, s_v=None):
        pass

    def updateDisplayLabel(self, value=None):
        """Update the display label to reflect the value of the parameter."""
        text = str(self.comboWidget.getValsLabelText())
        self.displayLabel.setText(text)


class KxFiniteLineParameter(KxParameter):
    '''
    roi 需要有set_roi_pos接口
    '''
    itemClass = KxFiniteLineParameterItem

    def __init__(self, *args, **kargs):
        self.roi = None
        KxParameter.__init__(self, *args, **kargs)
        roi_opt = self.opts.get("roi_opt", {})
        initVal = self.opts['value'].get('pos', None)
        if initVal:
            initVal = int(initVal)
            roi_opt['pos'] = initVal
        # print ('in')
        self.roi = FiniteLineItem(**roi_opt)
        self.roi.sigLinePosChanged.connect(self.roi_region_changed)

        self.h_viewbox = None
        self.maxbounds = None

    def set_roi(self, roi):
        self.roi = roi
        self.roi.sigLinePosChanged.connect(self.roi_region_changed)

    def get_roi(self):
        return self.roi

    def add2view(self, h_view):
        if self.roi not in h_view.scene().items():
            h_view.addItem(self.roi)
            self.h_viewbox = h_view
            # self.h_viewbox.scene().changed.connect(self.sceneAddPitcure)
            if "ismaxbounds" in self.opts:
                if self.opts["ismaxbounds"]:
                    self.h_viewbox.scene().changed.connect(self.sceneChanged)

        self.set_roi_visible(True)

    #         if 'isShow' in self.opts['value']:
    #             self.set_roi_visible(self.value_translate(self.opts['value']['isShow']))

    def sceneAddPitcure(self):
        # 未加入图片则不显示
        self.set_roi_visible(False)
        for item in self.h_viewbox.scene().items():
            if isinstance(item, pg.ImageItem):
                if item.image is not None:
                    self.set_roi_visible(True)
                #                     value = self.value()
                #                     if 'isShow' in value:
                #                         self.set_roi_visible(self.value_translate(value['isShow']))
                # self.h_viewbox.scene().changed.disconnect(self.sceneAddPitcure)
                break
            else:
                self.set_roi_visible(False)

    def sceneChanged(self):
        """
        场景变化中目前只用在设置roi的maxBounds,变动太频繁,会影响刷新,自动断开
        :return:
        """
        list_shape = []
        if "ismaxbounds" in self.opts:
            ismaxbounds = self.opts["ismaxbounds"]
            if ismaxbounds:
                # 最大范围添加至roi中
                for item in self.h_viewbox.scene().items():
                    if isinstance(item, pg.ImageItem):
                        if item.image is not None:
                            list_shape = item.image.shape
                        break
                if (list_shape == self.maxbounds) or (list_shape == []):
                    return
                else:
                    self.maxbounds = list_shape
                    self.setbounds(self.maxbounds)
                    self.h_viewbox.scene().changed.disconnect(self.sceneChanged)

    def setbounds(self, image_shape):
        y, x = image_shape[:2]
        x = int(x)
        y = int(y)
        pointlt = QtCore.QPoint(0, 0)
        pointrb = QtCore.QPoint(x, y)
        if hasattr(self.roi, "roi_outside"):
            self.roi.roi_outside.maxBounds = QtCore.QRectF(pointlt, pointrb)
        else:
            self.roi.maxBounds = QtCore.QRectF(pointlt, pointrb)

    def roi_region_changed(self):
        curPos = self.roi.getPos()
        self.setValue({"pos": curPos})

    def set_roi_visible(self, bool_visible):
        if bool_visible is not self.roi.isVisible():
            self.roi.setVisible(bool_visible)

    def setValue(self, value, blockSignal=None):
        """
        Set the value of this Parameter; return the actual value that was set.
        (this may be different from the value that was requested)
        """
        try:
            if blockSignal is not None:
                self.sigValueChanged.disconnect(blockSignal)
            if self.opts['value'] == value:
                return value
            self.opts['value'] = value
            self.__setLineVals()
            self.sigValueChanged.emit(self, value)
        finally:
            if blockSignal is not None:
                self.sigValueChanged.connect(blockSignal)

    def value_translate(self, value):
        if isinstance(value, str):
            value = (value == "True") or (value == u"True") or (value == u"True")  # 有时上面两句判断错误,加这句保险
        return value

    def __setLineVals(self):
        if "pos" in self.opts['value'] and self.roi is not None:
            # print (type(self.roi), self.roi)
            curRoiPos = self.roi.getPos()
            optVal = int(float(self.opts['value']["pos"]))
            if int(curRoiPos) != optVal:
                # print ("posVal: ",optVal)
                self.roi.setPos(optVal)

    def setMovable(self, flag):
        self.roi.setMovable(flag)

    def setLinePos(self, pos):
        self.roi.setPos(int(pos))

    def getLinePos(self):
        return self.roi.getPos()

    def setLineLength(self, length):
        self.roi.setLineLength(length)

    def getLineLength(self):
        return self.roi.getLineLength()

    def isShow(self, bool_show):
        self.set_roi_visible(bool_show)


kxregisterParameterType('finiteline', KxFiniteLineParameter, override=True)


class TabGroupParameterItem(ParameterItem):
    """
    Group parameters are used mainly as a generic parent item that holds (and groups!) a set
    of child parameters. It also provides a simple mechanism for displaying a button or combo
    that can be used to add new parameters to the group.
    """

    def __init__(self, param, depth):
        ParameterItem.__init__(self, param, depth)
        self.updateDepth(depth)
        self.tempItem = ParameterItem(Parameter(name='none'))
        self.tabWidget = QtGui.QTabWidget()
        self.tabWidget.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)

    def addChild(self, child):  ## make sure added childs are actually inserted before add btn
        if child.param.opts.get('visible', True):
            if isinstance(child, WidgetParameterItem):
                self.tabWidget.setMinimumHeight(
                    max(self.tabWidget.height(), child.layoutWidget.height()) + self.tabWidget.tabBar().height())
                self.tabWidget.addTab(child.layoutWidget, child.param.name())
            elif isinstance(child, KxGroupParameterItem):
                widget = ParameterTree(self.tabWidget, False)
                p = KxParameter.create(**child.param.opts)
                widget.setParameters(p, showTop=False)
                widget.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAsNeeded)
                self.tabWidget.setMinimumHeight(
                    max(self.tabWidget.height(), widget.height()) + self.tabWidget.tabBar().height())
                self.tabWidget.addTab(widget, child.param.name())

    def treeWidgetChanged(self):
        """Called when this item is added or removed from a tree."""

        ParameterItem.treeWidgetChanged(self)
        ## add all widgets for this item into the tree

        ParameterItem.addChild(self, self.tempItem)
        tree = self.treeWidget()
        if tree is not None:
            tree.setFirstItemColumnSpanned(self.tempItem, True)
            tree.setItemWidget(self.tempItem, 0, self.tabWidget)

    def updateDepth(self, depth):
        ## Change item's appearance based on its depth in the tree
        ## This allows highest-level groups to be displayed more prominently.
        if depth == 0:
            for c in [0, 1]:
                self.setBackground(c, QtGui.QBrush(QtGui.QColor(69, 160, 178)))
                self.setForeground(c, QtGui.QBrush(QtGui.QColor(220, 220, 255)))
                font = self.font(c)
                font.setBold(True)
                font.setPointSize(font.pointSize() + 1)
                self.setFont(c, font)
                self.setSizeHint(0, QtCore.QSize(0, 25))
        else:
            for c in [0, 1]:
                self.setBackground(c, QtGui.QBrush(QtGui.QColor(220, 220, 220)))
                font = self.font(c)
                font.setBold(True)
                # font.setPointSize(font.pointSize()+1)
                self.setFont(c, font)
                self.setSizeHint(0, QtCore.QSize(0, 20))


class TabGroupParameter(KxParameter):
    itemClass = TabGroupParameterItem


kxregisterParameterType('tabgroup', TabGroupParameter, override=True)



