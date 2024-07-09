function example_tcpsocket(mode)
% Example of simplest blocking echo-server and client
%
% Parameters:
% -----------
% mode: string
%   {"server", "client"}
%
% Examples:
% ---------
% example_tcpsocket("client")
% example_tcpsocket("server")

if nargin < 1
  mode = 'server';
end

[folder_script, filename_script, ~] = fileparts(mfilename('fullpath'));
fprintf('%s\n', folder_script);
fprintf('%s\n', filename_script);

if mode == "server"
  server_0();
elseif mode == "client"
  client_0();
else
  error('wrong mode: `%s`\n', mode);
end
end


% store msg-es in the queue
% -------------------------
function server_0()
fprintf("start xmat.TCPSocket.server_0()\n----------------------------\n");

xtcp = xmat.TCPSocket.server('::', xmat.TCPSocket.k_xport);
xtcp.flag_verbal = true;

pause(4);

fprintf('\nreceive messages\n\n');
while ~xtcp.isempty()
  xin = xtcp.recv();
  xin.print();
end
end


function client_0()
fprintf("start xmat.TCPSocket.client_0()\n----------------------------\n");

xtcp = xmat.TCPSocket.client('localhost', xmat.TCPSocket.k_xport);
xtcp.flag_verbal = true;

xout = xmat.MapStreamOut.byte();
xout.setitem('a0', 0);
xout.close();
xtcp.send(xout);

xout = xmat.MapStreamOut.byte();
xout.setitem('a1', 1);
xout.close();
xtcp.send(xout);
end
