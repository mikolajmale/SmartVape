import serial
import os
import re

arduino_port = "/dev/ttyACM0"
baud = 115200
w_dir = os.path.dirname(os.path.abspath(__file__))
fileName = os.path.join(w_dir, "ppg.csv")

ser = serial.Serial(arduino_port, baud)
print("Connected to Arduino port:" + arduino_port)
file = open(fileName, "w+")

samples = 10000
msg_num = 0 #start at 0 because our header is 0 (not real data)
while msg_num < samples:
    getData = str(ser.readline())
    data = getData[0:][:-2]
    print(data)
    if msg_num > 10:
        l = data.split(',')
        ir = re.findall("\d+", l[0])[0]
        red = re.findall("\d+", l[1])[0]
        file.write(f'{ir},{red}\n')  # write data with a newline
    msg_num = msg_num + 1

print("Data collection complete!")
file.close()