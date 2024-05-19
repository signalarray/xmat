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

clc

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

end


function server()
fprintf("start xmat.TCPSocket.server()\n");
xtcp = xmat.TCPSocket.server(xmat.TCPSocket.k_xport);

end


function client()
fprintf("start xmat.TCPSocket.client()\n");

end
