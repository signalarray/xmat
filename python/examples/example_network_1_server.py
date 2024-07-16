"""start
TCP-communication: example 0. echo server
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Open xmat-server and receive / reply message until command 'stop'
stop"""

import common

# start example
import time
import xmat

ipaddress = xmat.IPHOST   # OR '127.0.0.1' OR other ip-string
ipport = xmat.PORT        # OR 27015 OR other port-number

print('create connection')
xtcp = xmat.TCPService()
listener = xtcp.listener((ipaddress, ipport))
connection = xtcp.accept(listener)

print('sleeps for a 2.0 sec')
time.sleep(2.0)

print('start receiving')
while True:
	listeners_, connections_ = xtcp.wait('connections', 1.0)   # listeners aren't used
	if not connections_:
		print('no messages to receive')
		break

	connection_ = connections_[0]
	xin = connection_.recv()
	if 'stop' in xin:
		print('command `stop` received')
		break

	print('xin.getitem(`n`): ', n := xin.getitem('n'))
	print('xin.getitem(`msg`): ', msg := xin.getitem('msg').decode())

	xout = xmat.MapStreamOut.byte()
	xout.setitem('n', n)
	xout.setitem('msg', msg + '.reply')
	xout.close()
	connection_.send(xout)

print('finish successfully')

xtcp.close()
