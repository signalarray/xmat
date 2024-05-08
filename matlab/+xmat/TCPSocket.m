classdef TCPSocket_ < handle
    
  properties
    socket
    % socket.UserData - packets received

    isserver

    content__ = {};   % dont modify it manually. use method `pop`
    state = 'run';    % {'run', 'stop'}
  end
  

  methods (Static)
    function tcpsock = server(address, port, varargin)
      % Parameters
      % ----------
      % address: {string/char, []} OR int
      %   if {string/char, []} -> used as address. if [] OR '' -> address := '::'
      %   if int, address will be used as port. (but it's the only argument)
      %
      % port: int
      % 
      % varargin: pair<string, value>
      %   optional parameters for `tcpserver` constructor
      % 
      % Examples:
      % ---------
      % server = xmat.TCPSocket.server('0.0.0.0', 5001);
      % server = xmat.TCPSocket.server('::', 5001)
      % server = xmat.TCPSocket.server(5001);
      % server = xmat.TCPSocket.server([], 5001, 'Timeout', 20, 'EnableTransferDelay', false);
     
      if nargin == 1
        if ~isnumeric(address)
          error('`address` argument must be a nummeric when used as `port`');
        else
          port = address;
          address = '::';
        end
      end
      tspsock = xmat.TCPSocket_('server', address, port, varargin{:});
    end

    function tcpsock = client(address, port, varargin)
      % just for symmetry with xmat.TCPSocket.server(...)
      % Parameters:
      % address: string
      %   use "localhost" with local server

      tcpsock = xmat.TCPSocket_('client', address, port, varargin{:});
    end
  end


  methods
    function obj = TCPSocket_(mode, address, port, varargin)
      % Parameters:
      % -----------
      % mode: {'server', 'client'}
      % address: 
      %   if mode == server default '::'
      % port:
      
      if isempty(mode)
        mode = 'server';
      end
      
      if strcmp(mode, 'server') && isempty(address)
        address = '::';
      end

      if strcmp(mode, 'server')
        connection = tcpserver(address, port, varargin{:});
      elseif strcmp(mode, 'client')
        connection = tcpclient(address, port, varargin{:});
      else 
        error('wrong `mode` value');
      end

      % ---------------------
      obj.socket = connection;
      obj.isserver = isa(obj.socket, 'tcpserver');
    end


    function delete(obj)
      fprintf('class `%s` destructor was called\n', mfilename);
      obj.close();      
    end


    function close(obj)
      delete(obj.socket);
      clear obj.connection;
    end    

    
    function res = isempty(obj)
      res = isempty(obj.content) && obj.socket.NumBytesAvailable == 0;
    end


    function xm = pop(obj)
      
    end


    function send(obj, bugout)
      % Parameters
      % ----------
      % bugout: xmat.BugOut.bytes
      %   bugout.obuf: xmat.BufByte
      
      if ~isa(bugout, 'xmat.BugOut') 
        error('xmat.TCPSocket.send(..). wrong type: %s', class(bugout));
      end
      if bugout.h.total_size == 0 
        error('xmat.TCPSocket.send(..). bugout.h.total_size ~= 0 ::%d', bugout.h.total_size);
      end
      if ~isa(bugout.obuf, 'xmat.BufByte')
        error('xmat.TCPSocket.send(..). wrong type bugout.obuf: %s', class(bugout.ostream));
      end
      if ~isa(bugout.obuf.buf, 'uint8')
        error('xmat.TCPSocket.send(..). wrong type bugout.obuf.buf: %s', class(bugout.ostream.buff));
      end
      if bugout.is_open
        error('bugout must be closed');
      end
      
      write(obj.socket, bugout.ostream.buff, 'uint8');
    end


    function xin = recv(obj)
      xin = xmat.BugIn();

      % read header
      hbuf = uint8(read(obj.socket, xmat.XUtil.k_head_size, 'char'))';
      xin.push_buffer(hbuf);
      xin.scan_header();

      dbuff = uint8(read(obj.socket, xin.head.total_size-xmat.XUtil.k_head_size, 'char'))';
      xin.push_buffer(dbuff);
      xin.scan_data();
    end


    function recv_push(obj)
      xin = obj.recv();
      obj.content__{end + 1} = xin;
    end


    % private methods
    % ---------------
    function callback(obj, mode)
      % mode: logical
      %  if mode := true    callback is on
      %  if mode := false   callback is off

      if mode
        configureCallback(obj.socket, "byte", xmat.XUtil.k_head_size, @obj.recv_push);
      else
        configureCallback(t, "off");
      end
    end


    function count = read_buffer(obj)
      count = 0;
      while obj.socket.NumBytesAvailable
        xin = xmat.Input.from_bytes([]);
        hbuff = uint8(read(obj.socket, xmat.XUtil.k_head_size, 'char'))';
        xin.istream.buff = [xin.istream.buff; hbuff];
        xin.scan_header();
        
        dbuff = uint8(read(obj.socket, xin.h.total_size-xmat.XUtil.k_head_size, 'char'))';
        xin.istream.buff = [xin.istream.buff; dbuff];
        xin.scan_data();
        obj.content{end+1} = xin;
        count = count + 1;
      end
    end
  end
end
