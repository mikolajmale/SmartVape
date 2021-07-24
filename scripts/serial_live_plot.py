import sys, serial, argparse
from collections import deque
from enum import Enum
from typing import List
import matplotlib.pyplot as plt
import matplotlib.animation as animation


class SerialLineParser:
    def __init__(self, delimeter=',', arg_num: int = 2,
                 msg_min_val: List[float] = None, msg_max_val: List[float] = None):
        self.__delimeter = delimeter
        self.__arg_num = arg_num
        self.__last_valid = False
        self.__last_value = None
        self.__msg_min_val = msg_min_val
        self.__msg_max_val = msg_max_val
        if msg_min_val is not None:
            assert len(msg_min_val) == arg_num, 'Min param list does not have proper dimensions'
        if msg_max_val is not None:
            assert len(msg_max_val) == arg_num, 'Max param list does not have proper dimensions'

    def __call__(self, msg: str):
        self.__last_valid = self.__check_if_valid_msg(msg)
        if self.__last_valid:
            self.__last_value = [float(x) for x in msg.split(self.__delimeter)]
            self.__truncate_msg_data(self.__last_value)
            return self.__last_value
        else:
            return self.__last_value if self.__last_value else [0.0] * self.__arg_num

    def __truncate_msg_data(self, data: List[float]) -> List[float]:
        if self.__msg_min_val is not None:
            for i, min in enumerate(self.__msg_min_val):
                if data[i] < min: data[i] = min
        if self.__msg_max_val is not None:
            for i, max in enumerate(self.__msg_max_val):
                if data[i] > max: data[i] = max
        return data

    def __check_if_valid_msg(self, msg: str) -> bool:
        data = msg.split(self.__delimeter)
        if not isinstance(data, list): return False
        if len(data) != self.__arg_num: return False
        bool_l = [d.isdigit() for d in data]
        return all(bool_l)

    @property
    def valid(self):
        return self.__last_valid

    @property
    def param_num(self):
        return self.__arg_num


class AnalogPlot:
    def __init__(self, ax, parser=SerialLineParser(), str_port='/dev/ttyACM0', baud=115200, timeout=10, max_buf_len=500):
        # open serial port
        self.ser = serial.Serial(str_port, baud, timeout=timeout)

        self.parser = parser
        self.signals = [deque([0] * max_buf_len) for i in range(parser.param_num)]
        self.ax = ax
        self.maxLen = max_buf_len
        self.axes = []
        for i in range(parser.param_num):
            a, = ax.plot([], [])
            self.axes.append(a)

    def __add_to_buf(self, buf, val):
        if len(buf) < self.maxLen:
            buf.append(val)
        else:
            buf.pop()
            buf.appendleft(val)

    def add(self, data):
        for i, s in enumerate(self.signals):
            self.__add_to_buf(s, data[i])

    # update plot
    def update(self, frameNum):
        try:
            line = self.ser.readline().decode('utf-8').rstrip()
            data = self.parser(line)
            self.add(data)
            print(f'{data}')

            for i, ax in enumerate(self.axes):
                ax.set_data(range(self.maxLen), self.signals[i])

            self.ax.relim()
            self.ax.autoscale_view(True, True, True)

        except KeyboardInterrupt:
            print('exiting')

    def close(self):
        # close serial
        self.ser.flush()
        self.ser.close()


def main():
    # set up animation
    fig = plt.figure()
    ax = plt.axes()
    parser = SerialLineParser(delimeter=',', arg_num=2, msg_min_val=[0.0, 0.0], msg_max_val=[75000, 75000])
    analog_plot = AnalogPlot(ax=ax, parser=parser, max_buf_len=100)

    anim = animation.FuncAnimation(fig, analog_plot.update, interval=5)

    # show plot
    plt.show()

    # clean up
    analog_plot.close()

    print('exiting.')


# call main
if __name__ == '__main__':
    main()