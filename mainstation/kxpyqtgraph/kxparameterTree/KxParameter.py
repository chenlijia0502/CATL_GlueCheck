from pyqtgraph.parametertree import Parameter, ParameterTree, ParameterItem, registerParameterType
from collections import OrderedDict
from PyQt5 import QtWidgets, QtGui
KX_PARAM_TYPES = {}
KX_PARAM_NAMES = {}

def kxregisterParameterType(name, cls, override=False):#HYH 2020.03.02
    global KX_PARAM_TYPES
    if name in KX_PARAM_TYPES and not override:
        raise Exception("Parameter type '%s' already exists (use override=True to replace)" % name)
    KX_PARAM_TYPES[name] = cls
    KX_PARAM_NAMES[cls] = name

class KxParameter(Parameter):
    """
    这个是重写Parameter，在之基础上实现了几个多的功能。为了做到这个效果，必须重写几个原先包含Parameter的函数，而对于
    ParameterTypes更是全部重写，这些都是为了更充分的功能并且不改动源码
    （1）点击保存时，以当前预保存值为默认值，将item的标志置灰
    （2）
    """
    def __init__(self, **opts):
        super(KxParameter, self).__init__(**opts)

    @staticmethod
    def create(**opts):
        """
        Static method that creates a new Parameter (or subclass) instance using
        opts['type'] to select the appropriate class.

        All options are passed directly to the new Parameter's __init__ method.
        Use registerParameterType() to add new class types.
        """
        typ = opts.get('type', None)
        if typ is None:
            cls = KxParameter
        else:
            cls = KX_PARAM_TYPES[opts['type']]
        return cls(**opts)

    def saveState(self, filter=None):
        """
        Return a structure representing the entire state of the parameter tree.
        The tree state may be restored from this structure using restoreState().

        If *filter* is set to 'user', then only user-settable data will be included in the
        returned state.
        """
        if filter is None:
            state = self.opts.copy()
            if state['type'] is None:
                global PARAM_NAMES
                state['type'] = PARAM_NAMES.get(type(self), None)
        elif filter == 'user':
            state = {'value': self.value()}
        else:
            raise ValueError("Unrecognized filter argument: '%s'" % filter)

        ch = OrderedDict([(ch.name(), ch.saveState(filter=filter)) for ch in self])
        if len(ch) > 0:
            state['children'] = ch
        self.restoreState(state)#HYH
        return state

    def restoreState(self, state, recursive=True, addChildren=True, removeChildren=True, blockSignals=True):
        """
        Restore the state of this parameter and its children from a structure generated using saveState()
        If recursive is True, then attempt to restore the state of child parameters as well.
        If addChildren is True, then any children which are referenced in the state object will be
        created if they do not already exist.
        If removeChildren is True, then any children which are not referenced in the state object will
        be removed.
        If blockSignals is True, no signals will be emitted until the tree has been completely restored.
        This prevents signal handlers from responding to a partially-rebuilt network.
        """
        state = state.copy()
        childState = state.pop('children', [])

        ## list of children may be stored either as list or dict.
        if isinstance(childState, dict):
            cs = []
            for k, v in childState.items():
                cs.append(v.copy())
                cs[-1].setdefault('name', k)
            childState = cs

        if blockSignals:
            self.blockTreeChangeSignal()

        try:
            # self.setOpts(**state)
            self.setOptsforRestore(**state)# (1)HYH
            if not recursive:
                return

            ptr = 0  ## pointer to first child that has not been restored yet
            foundChilds = set()
            # print "==============", self.name()

            for ch in childState:
                name = ch['name']
                # typ = ch.get('type', None)
                # print('child: %s, %s' % (self.name()+'.'+name, typ))

                ## First, see if there is already a child with this name
                gotChild = False
                for i, ch2 in enumerate(self.childs[ptr:]):
                    # print "  ", ch2.name(), ch2.type()
                    if ch2.name() != name:  # or not ch2.isType(typ):
                        continue
                    gotChild = True
                    # print "    found it"
                    if i != 0:  ## move parameter to next position
                        # self.removeChild(ch2)
                        self.insertChild(ptr, ch2)
                        # print "  moved to position", ptr
                    ch2.restoreState(ch, recursive=recursive, addChildren=addChildren, removeChildren=removeChildren)
                    foundChilds.add(ch2)

                    break

                if not gotChild:
                    if not addChildren:
                        # print "  ignored child"
                        continue
                    # print "    created new"
                    ch2 = KxParameter.create(**ch)
                    self.insertChild(ptr, ch2)
                    foundChilds.add(ch2)

                ptr += 1

            if removeChildren:
                for ch in self.childs[:]:
                    if ch not in foundChilds:
                        # print "  remove:", ch
                        self.removeChild(ch)
        finally:
            if blockSignals:
                self.unblockTreeChangeSignal()

    def setOptsforRestore(self, **opts):
        """
        函数功能是抄setOpts，改动了一处
        """
        changed = OrderedDict()
        for k in opts:
            if k == 'value':
                # print ('set default: ', opts[k], k, opts)
                self.setValue(opts[k])
                self.setDefault(opts[k])
            elif k == 'name':
                self.setName(opts[k])
            elif k == 'limits':
                self.setLimits(opts[k])
            elif k == 'default':
                self.setDefault(opts[k])
            elif k not in self.opts or self.opts[k] != opts[k]:
                self.opts[k] = opts[k]
                changed[k] = opts[k]

        if len(changed) > 0:
            self.sigOptionsChanged.emit(self, changed)

    def insertChild(self, pos, child, autoIncrementName=None):
        """
        Insert a new child at pos.
        If pos is a Parameter, then insert at the position of that Parameter.
        If child is a dict, then a parameter is constructed using
        :func:`Parameter.create <pyqtgraph.parametertree.Parameter.create>`.

        By default, the child's 'autoIncrementName' option determines whether
        the name will be adjusted to avoid prior name collisions. This
        behavior may be overridden by specifying the *autoIncrementName*
        argument. This argument was added in version 0.9.9.
        """
        if isinstance(child, dict):
            child = KxParameter.create(**child)

        name = child.name()
        if name in self.names and child is not self.names[name]:
            if autoIncrementName is True or (autoIncrementName is None and child.opts.get('autoIncrementName', False)):
                name = self.incrementName(name)
                child.setName(name)
            else:
                raise Exception("Already have child named %s" % str(name))
        if isinstance(pos, KxParameter):
            pos = self.childs.index(pos)

        with self.treeChangeBlocker():
            if child.parent() is not None:
                child.remove()

            self.names[name] = child
            self.childs.insert(pos, child)

            child.parentChanged(self)
            child.sigTreeStateChanged.connect(self.treeStateChanged)
            self.sigChildAdded.emit(self, child, pos)
        return child

    def setValue(self, value, blockSignal=None):
        """
        Set the value of this Parameter; return the actual value that was set.
        (this may be different from the value that was requested)
        """
        try:
            if blockSignal is not None:
                self.sigValueChanged.disconnect(blockSignal)
            # HYH 2020.3.24 打了个补丁，当值为bool型时无法正常调用_interpretValue转换为正确的值
            if value == "True" or value == "true":
                value = True
            elif value == "False" or value == "false":
                value = False
            else:
                value = self._interpretValue(value)# 这个版本pyqtgraph多了这个

            if self.opts['value'] == value:
                return value
            self.opts['value'] = value
            self.sigValueChanged.emit(self, value)
        finally:
            if blockSignal is not None:
                self.sigValueChanged.connect(blockSignal)

        return value

    def makeTreeItem(self, depth):
        """
        Return a TreeWidgetItem suitable for displaying/controlling the content of
        this parameter. This is called automatically when a ParameterTree attempts
        to display this Parameter.
        Most subclasses will want to override this function.
        """
        if hasattr(self, 'itemClass'):
            #print "Param:", self, "Make item from itemClass:", self.itemClass
            item = self.itemClass(self, depth)
            item.setFont(0, QtGui.QFont('Consolas', 11, 60))
            return item
        else:
            item = ParameterItem(self, depth=depth)
            item.setFont(0, QtGui.QFont('Consolas', 11, 60))
            return