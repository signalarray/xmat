classdef ConnectionTCP < handle
    
  properties (Constant)

  end


  properties
    socket
    isserver

    content = {};
    state = 'run'; % {'run', 'stop'}
  end
  

  methods (Static)
    function xmattcp = make(mode, address, port)
      % Parameters:
      % -----------
      % mode: {'server', 'client'}
      % address: 
      %   if mode == server default '::'
      % port:
      %
      % Return: 
      % -------
      % ConnectionTCP
      %
      
      if isempty(mode)
        mode = 'server';
      end
      
      if strcmp(mode, 'server') && isempty(address)
        address = '::';
      end

      if strcmp(mode, 'server')
        connection = tcpserver(address, port);
      elseif strcmp(mode, 'client')
        connection = tcpclient(address, port);
      else 
        error('wrong `mode` value');
      end

      xmattcp = xmat.ConnectionTCP(connection);
    end
  end


  methods
    function obj = ConnectionTCP(connection)
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


    function count = wait(obj)
      % return controll when has elements in stack
      if ~isempty(obj.content)
        count = length(obj.content);
        return
      end

      s_ = obj.socket;
      while ~s_.NumBytesAvailable
        pause(0.01);
      end

      count = obj.read_buffer();
    end


    function xm = pop(obj)
      if isempty(obj.content)
        xm = [];
        return
      end
      xm = obj.content{1};
      obj.content(1) = [];
    end


    function write(obj, xmatout)
      % Parameters
      % ----------
      % xmatout: xmat.Save.bytes
      %   xmatout.ostream: xmat.StreamBytes
      
      if ~(isa(xmatout, 'xmat.Save') ... 
          && isa(xmatout.ostream, 'xmat.StreamBytes') ...
          && isa(xmatout.ostream.buff, 'uint8') ...
          && xmatout.isclosed)
        error('wrong xmatout');
      end
      
      write(obj.socket, xmatout.ostream.buff, 'uint8');
    end


    % private methods
    % ---------------
    function count = read_buffer(obj)
      count = 0;
      while obj.socket.NumBytesAvailable
        hbuff = uint8(read(obj.socket, xmat.Header.k_SIZEB, 'char'))';
        h = xmat.Header.read(xmat.StreamBytes('r', hbuff));

        dbuff = uint8(read(obj.socket, h.total_size-xmat.Header.k_SIZEB, 'char'))';
        xmatin = xmat.Load.bytes(dbuff, [], h);
        obj.content{end+1} = xmatin;
        count = count + 1;
      end
    end
  end


  methods (Access=private)
  end
end
