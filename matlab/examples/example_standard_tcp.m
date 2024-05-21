function example_standard_tcp(mode)
% example for echo-server and client exchanging with native matlab char array.
% works in echo mode
%
% Parameters
% ----------
% mode: string
%   {"server", "client"}
% 
% Examples:
% ---------
% example_standard_tcp("client")
% example_standard_tcp("server")

close all

if mode == "client"
  client();
else
  server();
end
end


function server()
% server = tcpserver("localhost", 4096);
server = tcpserver("::", 4096);
fprintf('\nServer started\n');

state_connected = server.Connected;
while 1
  if state_connected ~= server.Connected
    state_connected = server.Connected;

    fprintf("server connection changed:\n------------\n")
    fprintf('server.Connected: %d\n', server.Connected)
    fprintf('server.ClientAddress: %s\n', server.ClientAddress)
  end

  if server.NumBytesAvailable
    data = read(server, server.NumBytesAvailable, 'char');

    % is command `stop`
    if strcmp(data, 'stop')
      fprintf('\n[command]: `stop`\n server closed\n');
      break
    end

    fprintf('\n[data]\n{%s}\n', data);
    write(server, data, 'char');
    fprintf('\n');
  end

  pause(0.05)
end

% to delete
delete(server)
clear server
end


function client()
client = tcpclient("localhost", 4096);
fprintf('client started\n')

% --------------------------
buff_out = 'abcdefg-1234567';
buff_len = length(buff_out);

fprintf('\nwrite:\n');
write(client, buff_out, 'char')
fprintf('read back:\n');
data_in = read(client, buff_len, 'char');
fprintf('%s\n', data_in);


% --------------------------
buff_out = 'xxxxxxx-1111111';
buff_len = length(buff_out);

fprintf('\nwrite:\n');
write(client, buff_out, 'char')
fprintf('read back:\n');
data_in = read(client, buff_len, 'char');
fprintf('%s\n', data_in);


% --------------------------
fprintf('\nsend command `stop`:\n');
write(client, 'stop', 'char')

fprintf('lol\n')

% to delete
delete(client)
clear client
end
