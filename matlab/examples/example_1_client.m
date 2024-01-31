function example_1_client()
% supposed to be used with echo-server

clc
clear all
close all

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
