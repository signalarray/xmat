% open tcp-server
xtcp = xmat.TCPSocket.server('::', xmat.TCPSocket.k_xport);

% receive message
xin = xtcp.recv();
msg = xin.getitem('msg')
data = xin.getitem('data')

% send own message back
xout = xmat.MapStreamOut.byte();
xout.setitem('msg', 'message from server: Matlab');
xout.setitem('data', data);

xout.close();
xtcp.send(xout);

pause(2.0)
xtcp.close();
