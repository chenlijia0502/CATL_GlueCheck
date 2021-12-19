from PyQt5 import  QtWidgets, QtGui, QtCore
import time

class TimeStatus:
    g_curtime = 0

class App(QtWidgets.QApplication):

    def notify(self, eventobject: QtCore.QObject, event: QtCore.QEvent):
        """
        本次重写notify是为了截获应用的所有事件，并针对鼠标和键盘按下事件输出事件相关的信息
        :param eventobject: 事件接收对象
        :param event: 具体事件
        :return: True表示事件已经处理，False表示没有处理，需要继续往下传递
        """

        eventtype = event.type()


        flag = False
        if eventtype == QtCore.QEvent.MouseButtonPress:
            flag = True

        if flag:
            TimeStatus.g_curtime = time.time()
            #print(f"In app notify:事件类型值={eventtype}，事件接收者:{eventobject},parent={eventobject.parent()},child={eventobject.children()}")

        ret = super().notify(eventobject, event)
        # if flag:
        #     print(f"App notify end,事件接收者:{eventobject}，事件返回值={ret},app={self},parent={eventobject.parent()}")
        return ret
