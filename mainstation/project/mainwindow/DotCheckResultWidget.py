from PyQt5 import QtWidgets
from UI.ui_dotcheckresultwidget import Ui_DotCheckWidget

class DotCheckResultWidget(QtWidgets.QDialog):
    def __init__(self):
        super(DotCheckResultWidget, self).__init__()
        self.ui = Ui_DotCheckWidget()
        self.ui.setupUi(self)

    def setvalue(self, value1, value2):
        sword1 = str(value1) + "mm"
        self.ui.label_horizon.setText(sword1)
        sword2 = str(value2) + "mm"
        self.ui.label_vertical.setText(sword2)
