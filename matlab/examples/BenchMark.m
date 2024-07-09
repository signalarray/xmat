classdef BenchMark
  % [1] BenchMark.nativeTCPServer() + BenchMark.nativeTCPClient()

  % [2] BenchMark.xmatTCPServer(classname=double)  + BenchMark.xmatTCPClient(classname=double)

  properties (Constant)
    message_repeat = 32
    size_range = [10 20]  % [2^10 : 2^20]
    msg_size = 2.^(BenchMark.size_range(1):BenchMark.size_range(2));
    ip_port = xmat.TCPSocket.k_xport;
  end

  methods (Static)

    % native tcp-byte transfer
    % ------------------------
    function nativeTCPServer()
      fprintf('TestBench.nativeTCPServer\n');

      N = length(BenchMark.msg_size);

      socket = tcpserver("::", BenchMark.ip_port);

      t = zeros(N, BenchMark.message_repeat);
      legend_ = cell(1, N);
      for n = 1:N
        msg_size = BenchMark.msg_size(n);
        legend_{n} = sprintf('m:%12d', msg_size);
        fprintf('new message size: %d\n', msg_size);

        for m = 1:BenchMark.message_repeat
          t0 = tic();
          data = read(socket, msg_size, 'char');
          t(n, m) = toc(t0);
          fprintf('msg[%4d]\n', length(data));
        end

        fprintf('finish message size: %d Kb\n, time: %3.3f\n', msg_size/2^10, mean(t(n, 2:end)));
      end

      plot_result(t, 'native-Server', legend_);
    end

    function nativeTCPClient(ipaddress)
      fprintf('TestBench.nativeTCPClient\n');
      if nargin < 1
        ipaddress = "localhost";
      end

      N = length(BenchMark.msg_size);

      socket = tcpclient(ipaddress, BenchMark.ip_port);

      t = zeros(N, BenchMark.message_repeat);
      legend_ = cell(1, N);
      for n = 1:N
        msg_size = BenchMark.msg_size(n);
        legend_{n} = sprintf('m:%12d', msg_size);
        fprintf('new message size: %d\n', msg_size);

        for m = 1:BenchMark.message_repeat
          data = repmat(char('a' + (n - 1)), 1, msg_size);

          t0 = tic();
          write(socket, data, 'char');
          t(n, m) = toc(t0);
        end

        fprintf('finish message size: %d Kb, time: %3.3f\n', msg_size/2^10, mean(t(n, 2:end)));
      end

      plot_result(t, 'native-Client', legend_)
    end


    % xmat-format tcp-transfer: numeric elements
    % ------------------------------------------
    function xmatTCPServer()
      fprintf('TestBench.xmatTCPServer\n');

      N = length(BenchMark.msg_size);

      xtcp = xmat.TCPSocket.server('::', BenchMark.ip_port);

      t = zeros(N, BenchMark.message_repeat);
      legend_ = cell(1, N);
      for n = 1:N
        for m = 1:BenchMark.message_repeat
          t0 = tic();
          xin = xtcp.recv();
          data = xin.getitem('data');
          t(n, m) = toc(t0);
          sum_ = sum(data);
          fprintf('msg[%4d]\n', length(data));
        end
        msg_size = numel(data);
        legend_{n} = sprintf('m:%12d', msg_size);
        fprintf('new message size: %d\n', msg_size);

        fprintf('finish message size: %d K\n, time: %3.3f\n', msg_size/2^10, mean(t(n, 2:end)));
      end

      plot_result(t, 'xmat-Server', legend_);
    end

    function xmatTCPClient(dtype, ipaddress)
      % Parameters
      % ----------
      % classname: function_handle, {@double, @single, @complex, @int32, ...}
      %   numeric supported type

      fprintf('TestBench.xmatTCPClient\n');
      if nargin < 1
        dtype = @double;
      end
      if nargin < 2
        ipaddress = "localhost";
      end
      N = length(BenchMark.msg_size);

      xtcp = xmat.TCPSocket.client(ipaddress, BenchMark.ip_port);

      t = zeros(N, BenchMark.message_repeat);
      legend_ = cell(1, N);
      for n = 1:N
        msg_size = BenchMark.msg_size(n);
        legend_{n} = sprintf('m:%12d', msg_size);
        fprintf('new message size: %d\n', msg_size);

        data = dtype(n:n+msg_size);
        for m = 1:BenchMark.message_repeat
          t0 = tic();
          xout = xmat.MapStreamOut.byte();
          xout.setitem('data', data);
          xout.close();
          xtcp.send(xout);
          t(n, m) = toc(t0);
        end

        fprintf('finish message size: %d K\n, time: %3.3f\n', msg_size/2^10, mean(t(n, 2:end)));
      end

      plot_result(t, 'xmat-Client', legend_);
    end
  end
end


function plot_result(t, name, legend_)
fig = figure('Name', name);
subplot(1, 2, 1);
plot(t(:, 2:end).')
legend(legend_)
grid('on')

subplot(1, 2, 2);
plot(mean(t(:, 2:end), 2))
grid('on')
title('avarage time, sec')
xlabel('message length, 2\^(10 + x - 1) elements');

% summary

end


