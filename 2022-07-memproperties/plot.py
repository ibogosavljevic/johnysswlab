#!/usr/bin/env python3
#coding=utf-8

# Helper script to automatically plot performance data

import matplotlib.pyplot as plt
import numpy as np
import sys

def plot(sumDataFile, divDataFile):
    sumX, sumY = np.loadtxt(sumDataFile, delimiter=',', unpack=True)
    divX, divY = np.loadtxt(divDataFile, delimiter=',', unpack=True)
    N = len(sumX)
    xlabel = ['4K', '8K', '16K', '', '64K', '', '256K', '', '1M', '2M', '4M', '8M', '16M', '', '64M', '', '256M']
    x2 = np.arange(N)

    plt.plot(x2, sumY, label = 'sum', linewidth=3, color='b')
    plt.plot(x2, divY, label = 'div', linewidth=3, color='r')
    plt.xticks(x2, xlabel[:N])
    plt.xlabel('Array size')
    plt.ylabel('runtime(s)')
    plt.title('Time vs ArraySize on doubles, Desktop System')
    plt.legend()

    plt.grid(axis='y')
    plt.show()
    #plt.savefig('sum-div.png')

if __name__ == '__main__':
    sumFile = sys.argv[1]
    divFile = sys.argv[2]
    plot(sumFile, divFile)
