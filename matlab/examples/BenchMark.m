classdef BenchMark
    
  properties (Constant)
    message_number = 32
    size_range = [2^10 2^20]
    ip_port = xmat.TCPSocket.k_xport;
  end
  
  methods (Static)
    function StandardTCPServer()
      fprintf('TestBench.StandardTCPServer\n');
      
      N = log2(BenchMark.size_range(2) / BenchMark.size_range(1));

      socket = tcpserver("::", BenchMark.ip_port);

      t = zeros(N, BenchMark.message_number);
      leg_ = cell(1, N);
      for n = 1:N+1
        msg_size = BenchMark.size_range(1) * 2 ^ (n-1);
        leg_{n} = sprintf('m:%6d', msg_size);
        fprintf('new message size: %d\n', msg_size);

        for m = 1:BenchMark.message_number
          t0 = tic();
          data = read(socket, msg_size, 'char');
          t(n, m) = toc(t0);
          % fprintf('msg[%4d]: sum: %4.4f\n', m, data(16) + data(32));
          fprintf('msg[%4d]\n', length(data));
        end

        fprintf('finish message size: %d Kb\n, time: %3.3f', msg_size/2^10, mean(t(n, 2:end)));
        % fprintf('time mean: %4.4f, std: %4.4f\n', mean(t), std(t));
      end

      fig = figure('Name', 'Server');
      subplot(1, 2, 1);
      plot(t(:, 2:end).')
      legend(leg_)

      subplot(1, 2, 2);
      plot(mean(t(:, 2:end), 2))
      grid('on')
    end

    function StandardTCPClient()
      fprintf('TestBench.StandardTCPClient\n');
      
      N = log2(BenchMark.size_range(2) / BenchMark.size_range(1));

      socket = tcpclient("localhost", BenchMark.ip_port);

      t = zeros(N, BenchMark.message_number);
      leg_ = cell(1, N);      
      for n = 1:N+1
        msg_size = BenchMark.size_range(1) * 2 ^ (n-1);
        leg_{n} = sprintf('m:%6d', msg_size);
        fprintf('new message size: %d\n', msg_size);

        for m = 1:BenchMark.message_number
          data = repmat(char('a' + (n - 1)), 1, msg_size);

          t0 = tic();
          write(socket, data, 'char');
          t(n, m) = toc(t0);
          % fprintf('msg[%4d]\n', m);
        end

        fprintf('finish message size: %d Kb, time: %3.3f\n', msg_size/2^10, mean(t(n, 2:end)));
        % fprintf('time mean: %4.4f, std: %4.4f\n', mean(t), std(t));
      end

      fig = figure('Name', 'Client');
      subplot(1, 2, 1);
      plot(t(:, 2:end).')
      legend(leg_)

      subplot(1, 2, 2);
      plot(mean(t(:, 2:end), 2))
      grid('on')
    end
  end
end
