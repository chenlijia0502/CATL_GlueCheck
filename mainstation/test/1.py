import cv2


img = cv2.imread("d:\\9.bmp", 1)

dst = cv2.cvtColor(img, cv2.COLOR_RGB2HSV)

hsv = cv2.split(dst)

cv2.imwrite("d:\\hsv.bmp", dst)

cv2.imwrite("d:\\h.bmp", hsv[0])
cv2.imwrite("d:\\s.bmp", hsv[1])
cv2.imwrite("d:\\v.bmp", hsv[2])