import cv2

import numpy as np

img = np.zeros((100, 20), np.uint8)
R = cv2.rotate(img, cv2.ROTATE_90_COUNTERCLOCKWISE)

print(img.shape)

pos = [100, 100]
print(10)