# coding:utf-8
# 输入SSDsim的输出文件 trace out
# 输出trace名字和相应的read latency
import numpy as np
import seaborn as sns
import sys
from openpyxl import Workbook
from openpyxl.utils import get_column_letter
import pandas as pd
import scipy
import os
import sys
import matplotlib.pyplot as plt
from pylab import *
import math
from numpy import cumsum


# 数据输入
def get_trace_reference(filename):

    file_base = open(filename)

    flag = 0

    x = []
    y = []

    while 1:

        base_lines = file_base.readlines(100000)

        if (not base_lines):

            break

        for base_line in base_lines:

            if str(base_line).startswith("      arrive"):

                flag = 1

                continue

            if str(base_line).startswith("the 0 channel,"):

                flag = 0

            if(flag == 1):

                if(len(base_line.split()) != 7):
                    print("error\n")
                if(int(base_line.split()[6]) != 1000):
                    x.append(int(base_line.split()[4])/1e9/3600)
                    y.append(int(base_line.split()[6])/1e6)
    file_base.close()

    return x, y


# 解决保存图像是负号'-'显示为方块的问题
mpl.rcParams['axes.unicode_minus'] = False
plt.rcParams['xtick.direction'] = 'in'
plt.rcParams['ytick.direction'] = 'in'
tick_params(direction='in')
ax = plt.gca()


x1, y1 = get_trace_reference(sys.argv[1])
x2, y2 = get_trace_reference(sys.argv[2])
x3, y3 = get_trace_reference(sys.argv[3])
x4, y4 = get_trace_reference(sys.argv[4])
plt.plot(x1, y1, 'r--', label="GR-GC")
plt.plot(x2, y2, 'g--', label="DS-GC")
plt.plot(x3, y3, 'b--', label="TP-GC")
plt.plot(x4, y4, color='pink', linestyle='--', label="LA-GC")
plt.ylabel(u"请求延迟（单位：毫秒）", fontsize=20)
plt.xlabel(sys.argv[6][:-4] + u" 运行时间（单位：小时）", fontsize=20)
plt.legend(loc='upper right')
plt.tight_layout()
plt.savefig(sys.argv[5]+"op_write.png", bbox_inches='tight', dpi=1000)
plt.show()
