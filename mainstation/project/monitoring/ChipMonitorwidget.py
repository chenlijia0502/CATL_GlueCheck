from library.monitoring.BaseMonitoringWidget import KxBaseMonitoringWidget
from library.monitoring.BaseMonitoringWidget import registerkxmointorwidget
from library.monitoring.Display_interface.simplestatistics import StatisticsInterface
from PyQt5 import QtGui, QtWidgets
from library.common.KxImageBuf import KxImageBuf

import json

class ChipMonitorWidget(KxBaseMonitoringWidget, StatisticsInterface):
    def __init__(self, h_parent):
        super(ChipMonitorWidget, self).__init__(h_parent)
        self.fp = None

    def recmsg(self, n_stationid, n_msgtype, tuple_data):
        dict_result = json.loads(tuple_data)
        # print (dict_result)
        readimg = self._getimage(dict_result)
        list_feature = dict_result.get('defect feature', [])
        self.kxshow(readimg, dict_result['checkstatus'], list_feature)

    def _getimage(self, dict_result):
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
        return arrImg

    def clear(self):
        self.kxclear()

registerkxmointorwidget("ChipMonitorWidget", ChipMonitorWidget)