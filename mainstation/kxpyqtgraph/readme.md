#适用于公司的pyqtgraph



##1，kxparameterTree重写的类主要为Paramter类，因其保存参数、载入参数以及对于item的字体大小未开放接口等问题，不得不重写该类；重写该类必须改写与其相关的各个子类，也即parameterTypes，并且由于我们需要使用到自定义的参数类，所以新加部分写于KxCustomWidget中

##2，kxItem为一些ROI类，这些类与paramter类搭配使用，也可直接显示与view上

##3, qtinterface 为依赖pyqt以及pyqtgraph编写的界面库