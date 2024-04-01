from library.monitoring.BaseMonitoringWidget import KxBaseMonitoringWidget
from library.monitoring.BaseMonitoringWidget import registerkxmointorwidget
from UI.ui_kxtestmonitor import Ui_ParamPYWidget
import pyqtgraph as pg
import imc_msg
import json
from library.common.KxImageBuf import KxImageBuf
from library.ipc.ipc_tool import sendmsg

class testmoin(KxBaseMonitoringWidget):
    def __init__(self, h_parent):
        KxBaseMonitoringWidget.__init__(self, h_parent)
        self.ui = Ui_ParamPYWidget()
        self.ui.setupUi(self)
        self.fp = None
        self._completeui()

    def _completeui(self):
        self.view = pg.ViewBox(invertY=True, enableMenu=False)
        self.ui.widget_5.setCentralItem(self.view)
        self.view.setAspectLocked(True)
        self.img = pg.ImageItem()
        self.view.addItem(self.img)

    def _clearlabel(self):
        self.ui.label_width.clear()
        self.ui.label_dot.clear()
        self.ui.label_id.clear()

    def recmsg(self, n_stationid, n_msgtype, tuple_data):
        if n_msgtype == imc_msg.MSG_CHECK_RESULT:
            self._clearlabel()
            #
            # dict_result = json.loads(tuple_data)
            # self.ui.label_dot.setText(str(dict_result["dots"]))
            # self.ui.label_width.setText(str(n_stationid))
            # self.ui.label_id.setText(str(dict_result["id"]))
            # if n_stationid == 0:
            #     self.showimg(dict_result)

            # tuple_data = "{'ID': 1, 'LEVEL':0, 'defectname': 'OK' ,'dots': 100, 'x':0, 'y':0, 'width':100, 'height':100}"

    def showimg(self, dict_result):
        try:
            readimagepath = dict_result['imagepath']
            startoffset = dict_result['startoffset']
            offsetlen = dict_result['imageoffsetlen']
        except AttributeError:
            return None
        if self.fp is None:
            try:
                self.fp = open(readimagepath, "rb")
            except IOError:
                return None
        self.fp.seek(startoffset)
        data = self.fp.read(offsetlen)
        Img = KxImageBuf()
        Img.unpack(data)
        arrImg = Img.Kximage2npArr()
        self.img.setImage(arrImg)

    def slotA(self):
        sendmsg(0, imc_msg.MSG_A, "1")



registerkxmointorwidget("testmoin", testmoin)