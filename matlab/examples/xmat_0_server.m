function xmat_0_server()
xmattcp = xmat.ConnectionTCP.make('server', [], 3000);
% xmattcp = xmat.ConnectionTCP.make('client', 'localhost', 3000);

while true
  xmattcp.wait();   % waits until got messages
  
  xmatin = xmattcp.pop();
  if isfield(xmatin.map, 'stop')
    break
  else
    fprintf('--------------\n');
    disp(xmatin.map)
    A = xmatin.load('A');
    disp(A);
  end
end

fprintf('loop was stoped\n');
end
