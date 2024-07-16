"""start
TCP-client
^^^^^^^^^^
Open xmat-client and send / receive message
stop"""

import common

# start example
import time
import xmat

ipaddress = xmat.IPLOCALHOST
ipport = xmat.PORT

print('create connection')
xtcp = xmat.TCPService()
connection = xtcp.connection((ipaddress, ipport))

Nmsg = 8
Nlast = Nmsg - 1
for n in range(Nmsg):
	print('send iter: ', n)

	xout = xmat.MapStreamOut.byte()
	xout.setitem('n', n)
	if n == Nlast:
		xout.setitem('stop', 1)
	else:
		xout.setitem('msg', 'string content')
	xout.close()
	connection.send(xout)

	_, connections_ = xtcp.wait('connections', 1.0)
	if connections_:
		xin = connections_[0].recv()
		print('recv while sending n:', xin.getitem('n'))
	else:
		print('reply message isn`t ready')

print('end sending:')
print('receive remaining replies:')
while True:
	_, [connection_, ] = xtcp.wait('connections', 1.0)
	xin = connection_.recv()
	print('recv after sending n:', n := xin.getitem('n'))
	if n == Nlast - 1:
		break
