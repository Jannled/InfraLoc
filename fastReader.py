# https://matplotlib.org/stable/gallery/animation/strip_chart.html
#
#
#

from matplotlib.figure import Figure
import numpy as np
from matplotlib.lines import Line2D
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import matplotlib as mpl

import serial
import serial.tools.list_ports as sertools

import threading

# pip install wxpython

devices = sertools.comports()
device = 1
serialStream = None

receiveBuffer: str = ""

test = 0

class NakedEventSource:
	def __init__(self, callbacks = None) -> None:
		self.callbacks = [] if callbacks is None else callbacks.copy()

	def start(self):
		pass

	def stop(self):
		pass

	def add_callback(self, func, *args, **kwargs):
		self.callbacks.append((func, args, kwargs))
		return func

	def remove_callback(self, func):
		funcs = [c[0] for c in self.callbacks]
		if func in funcs:
			self.callbacks.pop(funcs.index(func))

	def _fire_event(self):
		"""
		Runs all function that have been registered as callbacks. Functions
		can return False (or 0) if they should not be called any more. If there
		are no callbacks, the timer is automatically stopped.
		"""
		for func, args, kwargs in self.callbacks:
			ret = func(*args, **kwargs)

			if ret == 0:
				self.callbacks.remove((func, args, kwargs))

		if len(self.callbacks) == 0:
			self.stop()

	def cleanup(self):
		self.stop()

	def __del__(self):
		"""Need to stop timer and possibly disconnect timer."""
		self.cleanup()

class MyAnimation(animation.Animation):
	def __init__(self, fig: Figure, event_source: NakedEventSource, updateFunc: callable, fArgs=None, blit: bool = False) -> None:
		if fArgs:
			self._args = fArgs
		else:
			self._args = ()
		self._func = updateFunc

		super().__init__(fig, event_source, blit)

	def _draw_frame(self, framedata):
		self._drawn_artists = self._func(framedata, *self._args)

		if self._blit:
			err = RuntimeError('The animation function must return a sequence '
							   'of Artist objects.')
			try:
				# check if a sequence
				iter(self._drawn_artists)
			except TypeError:
				raise err from None

			# check each item if it's artist
			for i in self._drawn_artists:
				if not isinstance(i, mpl.artist.Artist):
					raise err

			self._drawn_artists = sorted(self._drawn_artists,
										 key=lambda x: x.get_zorder())

			for a in self._drawn_artists:
				a.set_animated(self._blit)

class Scope:
	def __init__(self, ax, maxt=2, dt=0.02):
		self.ax = ax
		self.dt = dt
		self.maxt = maxt
		self.tdata = [0]

		self.ydata1 = [0]
		self.ydata2 = [0]
		self.ydata3 = [0]
		self.ydata4 = [0]
		self.ydata5 = [0]
		self.ydata6 = [0]
		self.ydata7 = [0]
		self.ydata8 = [0]

		self.line1 = Line2D(self.tdata, self.ydata1)
		self.line2 = Line2D(self.tdata, self.ydata2)
		self.line3 = Line2D(self.tdata, self.ydata3)
		self.line4 = Line2D(self.tdata, self.ydata4)
		self.line5 = Line2D(self.tdata, self.ydata5)
		self.line6 = Line2D(self.tdata, self.ydata6)
		self.line7 = Line2D(self.tdata, self.ydata7)
		self.line8 = Line2D(self.tdata, self.ydata8)

		self.ax.add_line(self.line1, color=)
		self.ax.add_line(self.line2, color=)
		self.ax.add_line(self.line3, color=)
		self.ax.add_line(self.line4, color=)
		self.ax.add_line(self.line5, color=)
		self.ax.add_line(self.line6, color=)
		self.ax.add_line(self.line7, color=)
		self.ax.add_line(self.line8, color=)

		self.ax.set_ylim(-.1, 200000)
		self.ax.set_xlim(0, self.maxt)

	def update(self, y):
		if y is None:
			return []

		lastt = self.tdata[-1]
		if lastt >= self.tdata[0] + self.maxt:  # reset the arrays
			self.tdata = [self.tdata[-1]]

			self.ydata1 = [self.ydata1[-1]]
			self.ydata2 = [self.ydata2[-1]]
			self.ydata3 = [self.ydata3[-1]]
			self.ydata4 = [self.ydata4[-1]]
			self.ydata5 = [self.ydata5[-1]]
			self.ydata6 = [self.ydata6[-1]]
			self.ydata7 = [self.ydata7[-1]]
			self.ydata8 = [self.ydata8[-1]]

			self.ax.set_xlim(self.tdata[0], self.tdata[0] + self.maxt)
			self.ax.figure.canvas.draw()

		# This slightly more complex calculation avoids floating-point issues
		# from just repeatedly adding `self.dt` to the previous value.
		t = self.tdata[0] + len(self.tdata) * self.dt

		self.tdata.append(t)

		self.ydata1.append(y[0])
		self.ydata2.append(y[1])
		self.ydata3.append(y[2])
		self.ydata4.append(y[3])
		self.ydata5.append(y[4])
		self.ydata6.append(y[5])
		self.ydata7.append(y[6])
		self.ydata8.append(y[7])

		self.line1.set_data(self.tdata, self.ydata1)
		self.line2.set_data(self.tdata, self.ydata2)
		self.line3.set_data(self.tdata, self.ydata3)
		self.line4.set_data(self.tdata, self.ydata4)
		self.line5.set_data(self.tdata, self.ydata5)
		self.line6.set_data(self.tdata, self.ydata6)
		self.line7.set_data(self.tdata, self.ydata7)
		self.line8.set_data(self.tdata, self.ydata8)

		return self.line1, 

def emitter(p=0.1):
	"""Return a random value in [0, 1) with probability p, else 0."""
	global serialStream
	global receiveBuffer

	if serialStream.in_waiting > 0:
		receiveBuffer += serialStream.read_all().decode('ASCII')

		splitted = receiveBuffer.split('\n')
		receiveBuffer = splitted[len(splitted) - 1]

		if(len(splitted) > 1):
			for l in splitted[ : -1]:
				stripped = l.strip()
				if not stripped.startswith('['):
					print("[Serial] " + l)
					continue
				else:
					data = [float(i) for i in stripped.removeprefix('[').removesuffix(']').split(',')]
					print(data)
					yield data
					return

	yield None

def main():
	global serialStream

	print("Starting Serial Plotter")
	fig, ax = plt.subplots()
	scope = Scope(ax)

	# Start Serial
	print("Connecting to " + devices[device].name)
	with serial.Serial(devices[device].name, 115200, timeout=5) as ser:
		serialStream = ser
		eventSource = NakedEventSource()
		#ani = MyAnimation(fig, eventSource, scope.update, blit=False)

		# pass a generator in "emitter" to produce data for the update func
		ani = animation.FuncAnimation(fig, scope.update, emitter, interval=50, blit=False, save_count=100)

		plt.show()

	
	serialStream = None

if __name__ == "__main__":
	main()