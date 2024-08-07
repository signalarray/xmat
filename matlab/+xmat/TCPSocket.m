classdef TCPSocket < handle

  properties (Constant)
    k_xsbuf_size = 1024
    k_xport = 27015
    k_xport_str = '27015'
  end


  properties
    socket
    isserver
    callback   % callback(xin: xmat.MapStreamIn)

    content__ = {};   % dont modify it manually. use method `pop`
    flag_verbal = false;
  end


  methods (Static)
    function socket = server(address, port, varargin)
      % See: matlab tcpserver arguments

      if ~any(strcmp("ConnectionChangedFcn", varargin))
        varargin(end+1:end+2) = {"ConnectionChangedFcn", @server_connection_fcn};
      end
      socket_ = tcpserver(address, port, varargin{:});
      socket = xmat.TCPSocket(socket_);
    end

    function socket = client(address, port, varargin)
      % See: matlab tcpclient arguments
      socket_ = tcpclient(address, port, varargin{:});
      socket = xmat.TCPSocket(socket_);
    end
  end


  methods
    function obj = TCPSocket(socket)
      obj.socket = socket;
      obj.isserver = isfield(socket, "ConnectionChangedFcn");
    end

    function delete(obj)
      if obj.flag_verbal
        fprintf('class `%s` destructor was called\n', mfilename);
      end
      obj.close();
    end

    function close(obj)
      delete(obj.socket);
      clear obj.socket;
    end 


    % send
    % ----------------------
    function send(obj, xout)
      % Parameters
      % ----------
      % xout: xmat.MapStream

      if ~isa(xout, 'xmat.MapStreamOut')
        error('xmat.TCP.send(..). wrong type: %s', class(xout));
      end
      if xout.is_open
        error('bugout must be closed');
      end
      if ~xout.head.total
        error('xmat.TCPSocket.send(..). bugout.h.total_size ~= 0 ::%d', xout.h.total_size);
      end
      if ~isa(xout.ods, 'xmat.DStreamByte')
        error('xmat.TCPSocket.send(..). wrong type bugout.obuf: %s', class(xout.ods));
      end
      if ~isa(xout.ods.buf, 'uint8')
        error('xmat.TCPSocket.send(..). wrong type bugout.obuf.buf: %s', class(xout.ods.buf));
      end

      write(obj.socket, xout.ods.buf, 'uint8');
    end


    function xin = recv(obj)
      xin = xmat.MapStreamIn.byte();

      hbuf = read(obj.socket, xmat.XHead.nbytes(), 'char');
      if length(hbuf) ~= xmat.XHead.nbytes()
        error('xmat.TCP.recv(): not enough data read');
      end
      xin.push_buffer(uint8(hbuf(:)));
      xin.scan_header();

      ndbuf = xin.head.total - xmat.XHead.nbytes();
      dbuf = read(obj.socket, ndbuf, 'char');
      if length(dbuf) ~= ndbuf
        error('xmat.TCP.recv(): not enough data read');
      end
      xin.push_buffer(uint8(dbuf(:)));
      xin.scan_data();
    end


    function res = isempty(obj)
      res = obj.socket.NumBytesAvailable == 0;
    end
    
    function wait(obj, timeout)
      % Parameters:
      % -----------
      % timeout: numeric, optional(default:=inf)
      %   time for waiting, sec.
      
      if nargin < 2
        timeout = 10.0;
      end

      t0 = tic;
      while obj.isempty() && toc(t0) < timeout
        pause(1e-4);
      end
      if obj.isempty()
        error('xmat.TCPSocket.wait(): timeout %.2g sec exceeds\n', timeout);
      end
    end
  end
end


function server_connection_fcn(src, ~)
if src.Connected
  fprintf('xmat.TCPSocket.server: connection accepted. ip: %s\n', src.ClientAddress);
else
  fprintf('xmat.TCPSocket.server: desconnected\n');
end
end
