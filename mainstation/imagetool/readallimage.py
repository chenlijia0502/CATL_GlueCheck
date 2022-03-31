import os

from PIL import Image
import cv2
import numpy as np


basepath = "F:\\2022-3-22\\"
s = "缩略图\\0.bmp"

list_subpath = os.listdir(basepath)

nindex  =0

for nindex, subpath in enumerate(list_subpath):

    path  =  basepath  + subpath + "\\" + s

    if os.path.isfile(path):


        img = np.array(Image.open(path))

        cv2.imwrite("d:\\img\\%d.bmp"%nindex, img)

    else:
        print("file not exist, ", path)
