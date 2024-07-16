from abc import ABC, abstractmethod
from enum import Enum
from typing import Union, Any
from itertools import chain

import time
import socket
import select

from . import datastream as xds


PORT = 27015
BUFSIZE = 1024

IPANY = '0.0.0.0'
IPHOST = IPANY
IPLOCALHOST = '127.0.0.1'


class SocketMode(Enum):
	LISTENER = 0
	CONNECTION = 1
	INVALID = 2		# not-opend or already closed


class _TCPSocket(ABC):
	"""
		See
		---
		docs.python.org/3/howto/sockets.html#socket-howto
		manpages.debian.org/bookworm/manpages-ru-dev/recv.2.ru.html
	"""

	def __init__(self, mode: SocketMode, *, accepted_socket=None):
		if accepted_socket is None:
			self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		else:
			if mode != SocketMode.CONNECTION:
				RuntimeError(f"xmat.TCPSocket_(): in accept case. wrong SocketMode.\
											Must be SocketMode.CONNECTION")
			self.socket = accepted_socket

		self.mode = mode
		self.callback: SocketCallback = None

	def close(self):
		if self.mode != SocketMode.INVALID:
			self.socket.close()
			self.mode = SocketMode.INVALID

	def islistener(self):
		return self.mode == SocketMode.LISTENER

	def isconnection(self):
		return self.mode == SocketMode.CONNECTION

	def isvalid(self):
		return self.mode != SocketMode.INVALID


class TCPListener(_TCPSocket):

	def __init__(self, address: tuple[str, int], backlog: int):
		_TCPSocket.__init__(self, SocketMode.LISTENER)
		self.socket.bind(address)
		self.socket.listen(backlog)

	def accept(self):
		newsock, address = self.socket.accept()
		return TCPConnection(None, accepted_socket=newsock)


class TCPConnection(_TCPSocket):

	def __init__(self, address: tuple[str, int] = None, *, accepted_socket = None):
		_TCPSocket.__init__(self, SocketMode.CONNECTION, accepted_socket=accepted_socket)
		if accepted_socket is None:
			# if it's client and it will be connected
			self.socket.connect(address)
		else:
			# if it's accepted by TCPListener. and it's ready for using
			if address is not None:
				raise ValueError("xmat.TCPConnection(): wrong using `accept_socket` arg")

	def send_(self, bytes: Union[bytearray, bytes]):
		self.socket.sendall(bytes)

	def recv_(self, nbytes: int) -> bytearray:
		buf = bytearray(nbytes)
		buffer = memoryview(buf)
		total = nbytes
		while total:
			nrcvd = self.socket.recv_into(buffer, total)
			buffer = buffer[nrcvd:]
			total -= nrcvd
		return buf

	# xmat.MapStream send-recv
	def send(self, xout: xds.MapStreamOut):
		if not isinstance(xout, xds.MapStreamOut):
			raise TypeError(f"xmat.TCPConnection.send(out): wrong type(out): {type(xout)}.\
												must be a :xmat.MapStreamOut")
		if not isinstance(xout.dstream, xds.DStreamByte):
			raise TypeError(f"xmat.TCPConnection.send(out): xout must be based on xmat.xds.DStreamByte data-stream")
		if xout.is_open:
			raise ValueError(f"xmat.TCPConnection.send(out): must be closed")

		self.send_(xout.dstream.buf)

	def recv(self) -> xds.MapStreamIn:
		xin = xds.MapStreamIn.byte()
		hbuf = self.recv_(xds.XHead.nbytes())
		xin.push_buffer(hbuf)
		xin.scan_head()

		ndbuf = int(xin.head.total - xds.XHead.nbytes())
		dbuf = self.recv_(ndbuf)
		xin.push_buffer(dbuf)
		xin.scan_data()
		return xin


class SocketCallback(ABC):

	def __init__(self):
		self.timelast = None
		self.timeout = 1.0
		self.counter: int = 0

	def isused(self):
		if not self.timelast:
			return False
		else:
			return time

	def run(self, connection, xin: xds.MapStreamIn = None) -> bool:
		out = self.__run__(connection, xin)
		self.timelast = time.time()
		self.counter += 1
		return out

	@abstractmethod
	def __run__(self, connection: TCPConnection, xin: xds.MapStreamIn = None) -> bool:
		"""
		Parameters
		----------
		connection: TCPConnection
		xin: xmat.MapStreamIn, default: None
			if callback is for Listener, xin := None

		Example
		-------
		def __run__(self, connection, xin):
			if 'command' in xin and xin['command'] == 'stop':
				connection.close()		# connection will be closed and removed right after the call

			# access to elements
			print([xin.keys()])

			# reply
			xout = xmat.MapStream.byte()
			xout['a0'] = np.array([1, 2, 3])
			xout.close()
			connection.send(xout)
		"""
		pass


class TCPService:

	LISTENERS = 'listeners'
	CONNECTIONS = 'connections'
	ALL = 'all'

	def __init__(self):
		self.connections = []
		self.listeners = []

		self.timeout = None

	def connection(self, address: tuple[str, int]) -> TCPConnection:
		connection = TCPConnection(address)
		self.connections.append(connection)
		return connection

	def listener(self, address: tuple[str, int], backlog: int = 4) -> TCPListener:
		"""
		Parameters:
			address: (IP: str, PORT: int)
			bakclog: int, = 4
				number of pending connections the queue will hold
		"""
		listener = TCPListener(address, backlog)
		self.listeners.append(listener)
		return listener

	def accept(self, listener: TCPListener) -> Union[TCPConnection, None]:
		if listener not in self.listeners:
			ValueError(f"xmat.TCPServes")
		connection: TCPConnection = listener.accept()
		# process callback
		if listener.callback:
			listener.callback.run(connection)
			if connection.mode == SocketMode.INVALID:
				return None
		self.connections.append(connection)
		return connection

	def remove(self, socket: _TCPSocket):
		if socket in self.listeners:
			self.listeners.remove(socket)
		elif socket in self.connections:
			self.connections.remove(socket)
		else:
			raise ValueError(f"xmat.TCPService.remove(socket): socket is not in _")
		socket.close()

	def close(self):
		for _ in chain(self.listeners, self.connections):
			self.remove(_)
		self.listeners = []
		self.connections = []

	def wait(self, mode: str = ALL, timeout: float = None) -> tuple[list[TCPListener], list[TCPConnection]]:
		"""
		Parameters:
		mode: str {'listeners', 'connections', 'all'}. 'all'
			if 'listeners': waits for only .listeners
			if 'connections': waits for only .connections
			if 'all': waits for both
		timeout: float
			seconds

		Return:
		tuple[list of listeners, list of connections]
			list of selected listeners thar are ready for accept
			list of selected connections that are ready for receive
		"""
		listeners_, connections_ = [], []
		if mode not in (self.LISTENERS, self.CONNECTIONS, self.ALL):
			raise ValueError(f"xmat.TCPService.wait(mode, ): wrong mode: {mode}. \
													expected ('listeners', 'connections', 'all')")
		if mode in (self.ALL, self.LISTENERS):
			dict_ = {item.socket: item for item in self.listeners}
			listeners_, *_ = select.select(dict_.keys(), [], [], timeout)
			listeners_ = [dict_[it] for it in listeners_]
		if mode in (self.ALL, self.CONNECTIONS):
			dict_ = {item.socket: item for item in self.connections}
			connections_, *_ = select.select(dict_.keys(), [], [], timeout)
			connections_ = [dict_[it] for it in connections_]
		return listeners_, connections_

	def process(self, mode: str = ALL, timeout: float = None) -> list[TCPConnection]:
		"""
		Parameters:
		same as .wait(mode, timeout) method

		Return:
			list of accepted connections
		"""

		listeners_, connections_ = self.wait(mode, timeout)
		newconnections = []

		for listener in listeners_:
			newconnections.append(self.accept(listener))
		for connection in connections_:
			if not connection.callback:
				continue
			xin = connection.recv()
			connection.callback.run(connection, xin)
			if not connection.isvalid():
				self.remove(connection)
		return newconnections
