import serial
ser = serial.Serial("/dev/ttyS2",115200)    # 连接串口
print('serial test start ...')              
ser.write(b"Hello Wrold !!!\n")             # 输入需要通讯的内容
for i in range(3):
    tmp = ser.readline()
    print(tmp)
    ser.write(tmp)