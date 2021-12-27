import time

print(time.strftime("%Y-%m-%d-%H-%M-%S"))

a = "检测时间"
fieldnames = ["barcode",a.encode('utf-8'), "测量结果", "异物数量", "气泡数量", "涂胶面积"]

print(fieldnames)


import csv,codecs
import unicodecsv as ucsv

def savecsv():
    import time
    s_data = time.strftime("%Y-%m-%d")
    csvpath = "F:\\模组数据.csv"
    fieldnames = ["barcode", "检测时间", "测量结果", "异物数量", "气泡数量", "涂胶面积"]
    # with codecs.open(csvpath, 'w', 'utf-8') as csvfile:
    #     # 指定csv文件指定头部
    #
    #
    #     writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
    #     writer.writeheader()
    #     try:
    #         writer.writerow({fieldnames[0]: "None",
    #                          fieldnames[1]: time.strftime("%Y:%m:%d-%H:%M:%S"),
    #                          fieldnames[2]: "OK"})
    #     except Exception as e:
    #         pass
    with open(csvpath, "rb") as f:
        r  =ucsv.reader(f, encoding="gbk")
        print(r.line_num)

    with open(csvpath, "wb") as f:


        w = ucsv.writer(f, encoding="gbk")
        w.writerow(fieldnames)

savecsv()