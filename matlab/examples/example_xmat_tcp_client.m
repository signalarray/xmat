% open tcp-client
xtcp = xmat.TCPSocket.client('localhost', xmat.TCPSocket.k_xport);

% send message first
xout = xmat.MapStreamOut.byte();
xout.setitem('msg', 'message from client: Matlab');
xout.setitem('data', 1:8);
xout.close();
xtcp.send(xout);

% receive message back
xin = xtcp.recv();
msg = xin.getitem('msg')
data = xin.getitem('data')

xtcp.close();
