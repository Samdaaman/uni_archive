from matplotlib.pyplot import subplots, savefig
import numpy as np

fs = 200
f0 = 10

t = np.arange(200) / fs
x = 2 * np.sin(2 * np.pi * f0 * t)

fig, axes = subplots(figsize=(6, 2))
axes.plot(t * 1e3, x)
axes.set_xlabel('Time (ms)')
axes.set_ylabel('Voltage (V)')
axes.grid(True)

savefig(__file__.replace('.py', '.pgf'), bbox_inches='tight')
