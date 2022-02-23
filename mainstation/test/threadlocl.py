import threading
import time



class SqQueue(object):
    def __init__(self, maxsize):
        self.queue = [None] * maxsize
        self.maxsize = maxsize
        self.front = 0
        self.rear = 0

    # 返回当前队列的长度
    def QueueLength(self):
        return (self.rear - self.front + self.maxsize) % self.maxsize

    # 如果队列未满，则在队尾插入元素，时间复杂度O(1)
    def push(self, data):
        if (self.rear + 1) % self.maxsize == self.front:
            print("The queue is full!")
        else:
            self.queue[self.rear] = data
           # self.queue.insert(self.rear,data)
            self.rear = (self.rear + 1) % self.maxsize

    # 如果队列不为空，则删除队头的元素,时间复杂度O(1)
    def pop(self):
        if self.rear == self.front:
            print("The queue is empty!")
        else:
            data = self.queue[self.front]
            self.queue[self.front] = None
            self.front = (self.front + 1) % self.maxsize
            return data

    def isempty(self):
        return self.rear == self.front

    # 输出队列中的元素
    def ShowQueue(self):
        for i in range(self.maxsize):
            print(self.queue[i],end=',')
        print(' ')



class testlock(object):
    def __init__(self):
        self.list_test = SqQueue(20)
        self.thread1 = threading.Thread(target=self._cycle_write)
        self.thread2 = threading.Thread(target=self._cycle_del)
        self.thread1.start()
        self.thread2.start()

    def _cycle_write(self):
        nlocktime = 1000
        while (nlocktime > 0):
            nlocktime -= 1
            self.list_test.push(nlocktime)
            print ('push: ', nlocktime)
            time.sleep(0.01)


    def _cycle_del(self):
        nlocktime = 1000
        while (nlocktime > 0):
            nlocktime -= 1
            if not self.list_test.isempty():
                print("pop: ", self.list_test.pop())
            time.sleep(0.01)


if __name__ == "__main__":

    o = testlock()
    time.sleep(20)
    print (o.list_test)