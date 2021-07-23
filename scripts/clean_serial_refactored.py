import sys, serial, argparse
from collections import deque
from enum import Enum
from typing import List
import matplotlib.pyplot as plt
import matplotlib.animation as animation

class ParseTypes(Enum):
    INT = 1
    FLOAT = 2
    CHAR = 3


class SerialLineParser:
    def __init__(self, delimeter=',', arg_num: int = 2):
        self.__delimeter = delimeter
        self.__arg_num = arg_num

    def __call__(self, msg: str):
        if self.__check_if_valid_msg(msg):
            return [0.0] * self.__arg_num
        else:
            return msg.split(self.__delimeter)

    def __check_if_valid_msg(self, msg: str) -> bool:
        data = msg.split(self.__delimeter)
        if not isinstance(data, list): return False
        if len(data) != self.__arg_num: return False
        bool_l = [d.isdigit() for d in data]
        return all(bool_l)

# plot class
class AnalogPlot:
    # constr
    def __init__(self, strPort='/dev/ttyACM0', baud=115200, timeout=10, maxLen=500):
        # open serial port
        self.ser = serial.Serial(strPort, baud, timeout=timeout)

        self.first = deque([0] * maxLen)
        self.second = deque([0] * maxLen)
        self.maxLen = maxLen

    # add to buffer
    def addToBuf(self, buf, val):
        if len(buf) < self.maxLen:
            buf.append(val)
        else:
            buf.pop()
            buf.appendleft(val)

    # add data
    def add(self, data):
        self.addToBuf(self.first, data[0])
        self.addToBuf(self.second, data[1])

    # update plot
    def update(self, frameNum, a0, a1):
        try:
            line = self.ser.readline().decode('utf-8').rstrip()
            is_valid_msg = self.check_if_valid_msg(line)
            data = [0, 0] if not is_valid_msg else [int(val) for val in line.split(',')]
            self.add(data)
            print(f'{data} [{is_valid_msg}]')

            a0.set_data(range(self.maxLen), self.first)
            a1.set_data(range(self.maxLen), self.second)
        except KeyboardInterrupt:
            print('exiting')


        # clean up

    def check_if_valid_msg(self, msg: str):
        data = msg.split(',')
        if not isinstance(data, list): return False
        if len(data) != 2: return False
        return data[0].isdigit() and data[1].isdigit()

    def close(self):
        # close serial
        self.ser.flush()
        self.ser.close()

    # main() function


def main():
    analogPlot = AnalogPlot()

    print('plotting data...')

    # set up animation
    fig = plt.figure()
    ax = plt.axes()

    a0, = ax.plot([], [])
    a1, = ax.plot([], [])
    anim = animation.FuncAnimation(fig, analogPlot.update,
                                   fargs=(a0, a1),
                                   interval=5)

    # show plot
    plt.show()

    # clean up
    analogPlot.close()

    print('exiting.')


# call main
if __name__ == '__main__':
    main()