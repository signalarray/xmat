function example_tcpsocket(mode, case_)
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

clc
% clear
close all
fclose('all');

if nargin < 1
  mode = 'server';
end

if nargin < 2
  case_ = 1;
end

[folder_script, filename_script, ~] = fileparts(mfilename('fullpath'));
fprintf('%s\n', folder_script);
fprintf('%s\n', filename_script);


servers = {@server_0, @server_1};
clients = {@client_0, @client_1};

if mode == "server"
  servers{case_}();
elseif mode == "client"
  clients{case_}();
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

fprintf([repmat('-', 1, 32) '\n']);
fprintf('check content\n');
fprintf([repmat('-', 1, 32) '\n']);
while ~xtcp.isempty()
  xin = xtcp.pop();
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


% callback msg-es
% -----------------
function server_1()
fprintf("start xmat.TCPSocket.server_1()\n----------------------------\n");

app = example_tcp_callback();

xtcp = xmat.TCPSocket.server('::', xmat.TCPSocket.k_xport);
xtcp.callback = @app.run;

app.exec(10.0);

fprintf('server closed\n');
end


function client_1()
fprintf("start xmat.TCPSocket.client_1()\n----------------------------\n");

xtcp = xmat.TCPSocket.client('localhost', xmat.TCPSocket.k_xport);

N = 256;
t0 = tic;
while true
  dt = toc(t0);
  xout = xmat.MapStreamOut.byte();
  xout.setitem('x', exp(1i*2*pi*(dt/4 + linspace(0, 1, N))) ...
                    + 1/16*complex(randn(1, N), randn(1, N)));
  xout.close();
  xtcp.send(xout);
  pause(1/8);
end
end


