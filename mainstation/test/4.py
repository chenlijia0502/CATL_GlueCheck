import threading
import time

def tt(*list_info):
    print (list_info)


class thre(threading.Thread):
    def __init__(self):
        super(thre, self).__init__()


    def run(self):
        while(1):
            print ('run')
            time.sleep(1)

# list_input =[[1,2,3,4]]
# t = threading.Thread(target=tt, args=list_input)
# t.start()
a = thre()

a.start()

time.sleep(5)

a.join()

print ('sleep')
time.sleep(5)
print('restart')
a.start()

time.sleep(5)
a.join()

