"""start
TCP-server
^^^^^^^^^^
Open xmat-server and receive / reply message
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

print('receive message')
xin = connection.recv()
msg = xin.getitem('msg')
data = xin.getitem('data')
print('msg: ', msg)
print('data: ', data)

print('reply message')
xout = xmat.MapStreamOut.byte()
xout.setitem('msg', 'message from server: Python')
xout.setitem('data', data)
xout.close()
connection.send(xout)

time.sleep(2.0)
xtcp.close()
