import sys, serial, argparse
from collections import deque

import matplotlib.pyplot as plt
import matplotlib.animation as animation


# plot class
class AnalogPlot:
    # constr
    def __init__(self, strPort, maxLen):
        # open serial port
        self.ser = serial.Serial(strPort, 115200, timeout=100)

        self.ax = deque([0] * maxLen)
        self.ay = deque([0] * maxLen)
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
        assert (len(data) == 2)
        self.addToBuf(self.ax, data[0])
        self.addToBuf(self.ay, data[1])

    # update plot
    def update(self, frameNum, a0, a1):
        try:
            line = self.ser.readline().decode('utf-8').rstrip()
            is_valid_msg = self.check_if_valid_msg(line)
            data = [0, 0] if not is_valid_msg else [int(val) for val in line.split(',')]
            self.add(data)
            print(f'{data} [{is_valid_msg}]')

            a0.set_data(range(self.maxLen), self.ax)
            a1.set_data(range(self.maxLen), self.ay)
        except KeyboardInterrupt:
            print('exiting')

        return a0,

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
    strPort = '/dev/ttyACM0'

    print('reading from serial port %s... ' % strPort)

    # plot parameters
    analogPlot = AnalogPlot(strPort, 1000)

    print('plotting data...')

    # set up animation
    fig = plt.figure()
    ax = plt.axes(xlim=[0,500], ylim=[50000,70000])

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