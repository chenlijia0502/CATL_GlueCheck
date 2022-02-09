# import numpy as np
#
#
# #array = np.array([[ 283,  281],[3054, 1006],[1621, 1614],[4926, 3057]])
# array = np.array([[ 283,  281],[3054, 1006, 100],[1601, 1614],[4926, 3057]])
#
#
# list_sortindex = sorted(range(len(array[1])), key=lambda k: array[1][k], reverse=False)
#
# print (list_sortindex)

import time

curtime = time.time()
strtime = time.strftime('%Y-%m-%d %X', time.localtime(curtime))
strtime = strtime + "." + str(curtime).split('.')[1][0:2]
print(strtime)
time.sleep(1.1)

curtime = time.time()
print(time.strftime('%Y-%m-%d %X', time.localtime(curtime)),"." + str(curtime).split('.')[1][0:2])
