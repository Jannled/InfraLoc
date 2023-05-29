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
import matplotlib.colors as col

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
	def __init__(self, ax, scopeChannels = list(range(16)), maxt=2, dt=0.02, maxv=100000):
		self.ax = ax
		self.scopeChannels = scopeChannels
		self.dt = dt
		self.maxt = maxt
		self.maxv = maxv

		self.tdata = [0]
		self.yData = [[0] for x in range(0, len(scopeChannels))]

		self.lines = [None] * len(scopeChannels)
		for l in range(0, len(scopeChannels)):
			self.lines[l] = Line2D(self.tdata, self.yData[l], color=col.hsv_to_rgb((1/len(scopeChannels)*l, 1, 1)), label="D"+str(scopeChannels[l]))
			self.ax.add_line(self.lines[l])

		self.ax.set_ylim(-.1, self.maxv)
		self.ax.set_xlim(0, self.maxt)
		self.ax.legend(handles=self.lines)

	def update(self, y):
		if y is None:
			return []

		lastt = self.tdata[-1]
		if lastt >= self.tdata[0] + self.maxt:  # reset the arrays
			self.tdata = [self.tdata[-1]]

			for l in range(0, len(self.scopeChannels)):
				self.yData[l] = [self.yData[l][-1]]

			self.ax.set_xlim(self.tdata[0], self.tdata[0] + self.maxt)
			self.ax.figure.canvas.draw()

		# This slightly more complex calculation avoids floating-point issues
		# from just repeatedly adding `self.dt` to the previous value.
		t = self.tdata[0] + len(self.tdata) * self.dt

		self.tdata.append(t)

		for l in range(0, len(self.scopeChannels)):
			self.yData[l].append(y[self.scopeChannels[l]])
			self.lines[l].set_data(self.tdata, self.yData[l])

		return self.lines

def emitter(p=0.1):
	"""Return an array from the serial console"""
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
					#print(data)
					yield data
					return

	yield None

def main():
	global serialStream

	print("Starting Serial Plotter")
	fig, ax = plt.subplots()
	scope = Scope(ax, [1, 5, 9, 13])

	# Start Serial
	print("Connecting to " + devices[device].name)
	with serial.Serial(devices[device].name, 115200, timeout=5) as ser:
		serialStream = ser
		#ani = MyAnimation(fig, eventSource, scope.update, blit=False)

		# pass a generator in "emitter" to produce data for the update func
		ani = animation.FuncAnimation(fig, scope.update, emitter, interval=50, blit=False, save_count=100)

		plt.show()

	
	serialStream = None

if __name__ == "__main__":
	main()