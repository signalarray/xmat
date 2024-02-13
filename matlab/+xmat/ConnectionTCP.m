classdef ConnectionTCP < handle
  % works in only with blocking mode
    
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

    function xin_bytes = resv(obj)
      % Returns
      % -------
      % xmat.Input<StreamBytes>

      obj.wait();
      xin_bytes = obj.pop();
    end

    function send(obj, xmatout)
      % Parameters
      % ----------
      % xmatout: xmat.Output.bytes
      %   xmatout.ostream: xmat.StreamBytes
      
      if ~isa(xmatout, 'xmat.Output') 
        error('wrong xout class: %s', class(xmatout));
      end
      if xmatout.h.total_size == 0 
        error('xmatout.h.total_size ~= 0 ::%d', xmatout.h.total_size);
      end
      if ~isa(xmatout.ostream, 'xmat.StreamBytes')
        error('wrong xmatout.ostream class: %s', class(xmatout.ostream));
      end
      if ~isa(xmatout.ostream.buff, 'uint8')
        error('wrong xmatout.ostream.buff class: %s', class(xmatout.ostream.buff));
      end
      if ~xmatout.isclosed
        error('xout is`nt closed');
      end
      
      write(obj.socket, xmatout.ostream.buff, 'uint8');
    end

    function resend(obj, xmatin)
      % Parameters
      % ----------
      % xin: xmat.Load.bytes
      %   xmatout.ostream: xmat.StreamBytes

      if ~isa(xmatin, 'xmat.Input') 
        error('xout class');
      end
      if xmatin.h.total_size == 0
        error('xmatout.h.total_size ~= 0 ');
      end
      if ~isa(xmatin.istream, 'xmat.StreamBytes')
        error('wrong xmatout.ostream class: %s', class(xmatin.istream));
      end
      write(obj.socket, xmatin.istream.buff, 'uint8');
    end


    % private methods
    % ---------------
    function count = read_buffer(obj)
      count = 0;
      while obj.socket.NumBytesAvailable
        xin = xmat.Input.from_bytes([]);
        hbuff = uint8(read(obj.socket, xmat.Header.k_SIZEB, 'char'))';
        xin.istream.buff = [xin.istream.buff; hbuff];
        xin.scan_header();
        
        dbuff = uint8(read(obj.socket, xin.h.total_size-xmat.Header.k_SIZEB, 'char'))';
        xin.istream.buff = [xin.istream.buff; dbuff];
        xin.scan_data();
        obj.content{end+1} = xin;
        count = count + 1;
      end
    end
  end
end
