from library.common.usermanager.Permission_Management import kxprivilege_management
from PyQt5 import QtWidgets


if __name__ == "__main__":
    app = QtWidgets.QApplication([])

    w = kxprivilege_management()

    w.show()

    app.exec_()