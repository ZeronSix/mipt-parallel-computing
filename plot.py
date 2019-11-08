#!/usr/bin/python3
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

df = pd.read_csv('plot.csv')
plt.plot(df['x'], df['y'], label='numerical')

x = np.linspace(0, 3 * np.pi, 10000)
y = -2.0 / (3 * np.pi) * x - np.sin(x)
plt.plot(x, y, label='exact', linestyle='dashed')
plt.legend(loc='best')
plt.show()