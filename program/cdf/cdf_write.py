# coding:GBK
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
import math
import matplotlib
from numpy import cumsum


# 数据输入
def get_trace_reference(filename):

    file_base = open(filename)

    flag = 0

    base_list = []

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

                    print(base_line.split(), len(base_line.split()))

                if(int(base_line.split()[3]) == 0):

                    base_list.append(int(base_line.split()[6]))

    file_base.close()

    return base_list


def get_hist(latencys):

    # 分割为1000份
    max_latency = max(latencys)

    min_latency = min(latencys)

    granularity = 1000000

    bins = math.ceil((max_latency-min_latency) /
                     granularity)  # ceil() 函数返回数字的上入整数。

    print("间隔 ", bins)
    counters = [0 for i in range(granularity)]

    for item in latencys:

        i = math.ceil((item-min_latency)/bins)

        if(i < 0 or i > (granularity-1)):

            print("overflow:", i)

        if(i == granularity):

            i = i-1

        if(i == granularity+1):

            i = i-2

        counters[i] = counters[i]+1

    sums = sum(counters)

    probability_counters = list(map(lambda x: x*1.0/sums, counters))

    # print(probability_counters,sum(probability_counters))

    probability_counters = cumsum(probability_counters)

    return probability_counters, [(min_latency+i*bins)/1e6 for i in range(granularity)]


base1 = get_trace_reference(sys.argv[1])
base2 = get_trace_reference(sys.argv[2])
base3 = get_trace_reference(sys.argv[3])
gc = get_trace_reference(sys.argv[4])

y, x = get_hist(base1)
for (i, j) in zip(x, y):

    if(abs(j-0.5) < 0.01 or j > 0.5):

        print("50% x轴坐标:", i)

        label = i

        break


for (i, j) in zip(x, y):

    if(abs(j-0.9) < 0.01 or j > 0.9):

        print("90% x轴坐标:", i)
        break

for (i, j) in zip(x, y):

    if(abs(j-0.95) < 0.01 or j > 0.95):

        print("95% x轴坐标:", i)

        val_x = i

        break
for (i, j) in zip(x, y):

    if(abs(j-0.99) < 0.01 or j > 0.99):

        print("99% x轴坐标:", i)

        break
index = x.index(val_x)
x1 = x[index:]
y1 = y[index:]

y, x = get_hist(base2)
for (i, j) in zip(x, y):

    if(abs(j-0.5) < 0.01 or j > 0.5):

        print("50% x轴坐标:", i)

        label = i

        break


for (i, j) in zip(x, y):

    if(abs(j-0.9) < 0.01 or j > 0.9):

        print("90% x轴坐标:", i)
        break

for (i, j) in zip(x, y):

    if(abs(j-0.95) < 0.01 or j > 0.95):

        print("95% x轴坐标:", i)

        val_x = i

        break
for (i, j) in zip(x, y):

    if(abs(j-0.99) < 0.01 or j > 0.99):

        print("99% x轴坐标:", i)

        break
index = x.index(val_x)
x2 = x[index:]
y2 = y[index:]

y, x = get_hist(base3)
for (i, j) in zip(x, y):

    if(abs(j-0.5) < 0.01 or j > 0.5):

        print("50% x轴坐标:", i)

        break
for (i, j) in zip(x, y):

    if(abs(j-0.9) < 0.01 or j > 0.9):

        print("90% x轴坐标:", i)

        break
for (i, j) in zip(x, y):

    if(abs(j-0.95) < 0.01 or j > 0.95):

        print("95% x轴坐标:", i)

        val_x = i

        break
for (i, j) in zip(x, y):

    if(abs(j-0.99) < 0.01 or j > 0.99):

        print("99% x轴坐标:", i)

        break

index = x.index(val_x)
x3 = x[index:]
y3 = y[index:]

y, x = get_hist(gc)
for (i, j) in zip(x, y):

    if(abs(j-0.5) < 0.01 or j > 0.5):

        print("50% x轴坐标:", i)

        break
for (i, j) in zip(x, y):

    if(abs(j-0.9) < 0.01 or j > 0.9):

        print("90% x轴坐标:", i)

        break
for (i, j) in zip(x, y):

    if(abs(j-0.95) < 0.01 or j > 0.95):

        print("95% x轴坐标:", i)

        val_x = i

        break
for (i, j) in zip(x, y):

    if(abs(j-0.99) < 0.01 or j > 0.99):

        print("99% x轴坐标:", i)

        break

index = x.index(val_x)
x4 = x[index:]
y4 = y[index:]

plt.rcParams['xtick.direction'] = 'in'
plt.rcParams['ytick.direction'] = 'in'
# tick_params(direction='in')
ax = plt.gca()
# ax.spines['right'].set_color('none')
# ax.spines['top'].set_color('none')
ax.spines['top'].set_visible(True)
ax.spines['right'].set_visible(True)
plt.ylabel("CDF", fontsize=20)
plt.xlabel(sys.argv[6][:-4] + u" 写请求延迟(单位：毫秒)", fontsize=20)
plt.plot(x1, y1, 'r', label="GR-GC")
plt.plot(x2, y2, 'g', label="DS-GC")
plt.plot(x3, y3, 'b', label="TP-GC")
plt.plot(x4, y4, 'pink', label="LA-GC")
plt.legend(loc="lower right")
plt.tight_layout()

print(sys.argv[5]+"write.png")
plt.savefig(sys.argv[5]+"write.png", bbox_inches='tight', dpi=1000)

plt.show()
