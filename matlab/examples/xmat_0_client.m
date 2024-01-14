function xmat_0_client()
xmattcp = xmat.ConnectionTCP.make('client', 'localhost', 3000);
% xmattcp = xmat.ConnectionTCP.make('server', [], 3000);


for n = 1:4
  % send message
  xmatout = xmat.Save.bytes();
  xmatout.save('A', uint8(n*(1:4)))
  xmatout.close();
  xmattcp.write(xmatout)
end

xmatout = xmat.Save.bytes();
xmatout.save('stop', 1)
xmatout.close();
xmattcp.write(xmatout)
end
