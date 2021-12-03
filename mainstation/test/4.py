import threading


def tt(*list_info):
    print (list_info)



list_input =[[1,2,3,4]]
t = threading.Thread(target=tt, args=list_input)
t.start()
