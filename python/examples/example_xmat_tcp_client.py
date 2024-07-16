"""start
TCP-client
^^^^^^^^^^
Open xmat-client and send / receive message
stop"""

import common

# start example
import xmat

ipaddress = xmat.IPLOCALHOST
ipport = xmat.PORT

print('create connection')
xtcp = xmat.TCPService()
connection = xtcp.connection((ipaddress, ipport))

print('send message')
xout = xmat.MapStreamOut.byte()
xout.setitem('msg', 'message from client: Python')
xout.setitem('data', [float(n) for n in range(1, 9)])
xout.close()
connection.send(xout)

print('receive message')
xin = connection.recv()
msg = xin.getitem('msg')
data = xin.getitem('data')
print('msg: ', msg)
print('data: ', data)

xtcp.close()
