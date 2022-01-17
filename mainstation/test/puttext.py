import cv2


row  = 2

col = 3

img =cv2.imread("D:\\0.bmp", 1)

rotateimg = cv2.rotate(img, cv2.ROTATE_90_COUNTERCLOCKWISE)


ncurrow = col
ncurcol = row

h, w = rotateimg.shape[:2]
nsteph = int(h / ncurrow)
nstepw = int(w / ncurcol)

for i in range(ncurrow):
    for j in range(ncurcol):
        pos = (j * nstepw, i * nsteph + 300)
        word = str((ncurrow - i - 1) * ncurcol + j)
        rotateimg = cv2.putText(rotateimg, word, pos , cv2.FONT_HERSHEY_SIMPLEX, 8, (0, 255, 0), 10)
        print (word)



#new = cv2.putText(img, str('test'), (300, 300), cv2.FONT_HERSHEY_SIMPLEX, 10, (0, 255, 0), 8)



cv2.namedWindow("h", 0)
cv2.imshow("h", rotateimg)
cv2.waitKey(0)