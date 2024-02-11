function example_1_server_echo()
% basic connection-server with standart features of matlab
% works in echo mode

clc
clear all
close all

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
