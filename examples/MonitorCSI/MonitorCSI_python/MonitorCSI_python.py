# Monitor CSI example for the Approximate Library (Python client)
# -
# David Chatting - github.com/davidchatting/Approximate
# MIT License - Copyright (c) February 2021
#
# Adapted from: http://www.mikeburdis.com/wp/notes/plotting-serial-port-data-using-python-and-matplotlib/

import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import style
import numpy as np
import random
from serial import Serial

#initialize serial port
ser = Serial()
ser.port = '/dev/tty.SLAB_USBtoUART' #ESP32 serial port
ser.baudrate = 9600
ser.timeout = 10 #specify timeout when using readline()
ser.open()
if ser.is_open==True:
    # Create figure for plotting
    fig = plt.figure()
    ax = fig.add_subplot(1, 1, 1)
    xs = []
    ys = []

    # This function is called periodically from FuncAnimation
    def animate(i, xs, ys):
        line=ser.readline().strip()
        line_as_list = line.split(b'\t')

        if len(line_as_list)==52:
            for n, csipacket in enumerate(line_as_list):            
                v = np.array(csipacket.split(b','))     #real,imaginary  
                xs.append(n-26)                         #subcarrier
                ys.append(np.linalg.norm(v))            #magnitude

            # Limit x and y lists to 52 items
            xs = xs[-52:]
            ys = ys[-52:]

            # Draw x and y lists
            ax.clear()
            ax.plot(xs, ys)

    # Set up plot to call animate() function periodically
    ani = animation.FuncAnimation(fig, animate, fargs=(xs, ys), interval=50)
    plt.show()