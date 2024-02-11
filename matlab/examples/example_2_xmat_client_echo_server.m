function example_2_xmat_client_echo_server(mode)
% Example of simplest blocking echo-server and client
% 
% Parameters
% ----------
% mode: string
%   {"server", "client"}

clc
% clear
close all
fclose('all');

[folder_script, filename_script, ~] = fileparts(mfilename('fullpath'));
fprintf('%s\n', folder_script);
fprintf('%s\n', filename_script);

if mode == "client"
  client();
else
  server();
end
end % ---------------------------------------------------------------------


function client()
fprintf("start xmat.NetConnection.client for echo-server\n");

xmattcp = xmat.ConnectionTCP.make('client', 'localhost', 3000);

for n = 1:4
  fprintf("iter %d\n========\n", n);
  xmatout = xmat.Save.bytes();
  xmatout.save('A', uint8(n*(1:4)))
  xmatout.close();
  xmattcp.write(xmatout)
  fprintf("data sent. waiting for back-message\n");
  
  xmattcp.wait();
  xmatin = xmattcp.pop();
  % if isequal(xmatin.istream.buff, xmatout.ostream.buff)
  %   fprintf("success: back-message buffer ok\n");
  % else
  %   error("fail: back-message buffer is NOT equal to original one\n");
  % end
end

xmatout = xmat.Save.bytes();
xmatout.save('command', 'stop')
xmatout.close();
xmattcp.write(xmatout)
end % ---------------------------------------------------------------------


function server()
fprintf("start xmat.NetConnection.echo-server\n");

xmattcp = xmat.ConnectionTCP.make('server', [], 3000);

while true
  xmattcp.wait();   % waits until got messages
  xmatin = xmattcp.pop();

  % process 'stop' condition
  command = xmatin.load('command');
  if ~isempty(command)
    if command == "stop"
      fprintf("server was closed by [command]: 'stop'\n");
      break
    end
  else
    fprintf('--------------\n');
    disp(xmatin.map)
    % A = xmatin.load('A');
    % disp(A);
    xmattcp.resend(xmatin);
  end
end

fprintf('loop was stoped\n');
end % ---------------------------------------------------------------------
