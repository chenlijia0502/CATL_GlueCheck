from PyQt5 import QtWidgets




if __name__ == "__main__":
    a = QtWidgets.QApplication([])
    w = ZSTableWidget()
    w.show()
    a.exec_()