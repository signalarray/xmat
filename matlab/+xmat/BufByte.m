classdef BufByte< handle
    
  properties
    mode

    buf = uint8(zeros(0, 1));
    cursor = 0
    byteorder = 'n'
  end
  

  methods (Static)
    function bbout = out()
      bbout = xmat.BufByte('w');
    end

    function bbin = in()
      bbin = xmat.BufByte('r');
    end
  end


  methods
    function obj = BufByte(mode)
      % Parameters:
      % -----------
      % mode: {'r', 'w'}

      if ~any(mode == ["r", "w"])
        error('xmat.BufByte.ctor(..). wrong `mode` value')
      end
      obj.mode = mode;
    end

    % for deffered initialization
    function obj = set_buffer(obj, buf)
      if obj.mode ~= "r"
        error('xmat.BufByte.set_buffer(). wrong mode');
      end
      check_buffer(buf);

      obj.buf = buf;
      obj.cursor = 0;
    end

    function obj = push_buffer(obj, buf)
      if obj.mode ~= "r"
        error('xmat.BufByte.set_buffer(). wrong mode');
      end
      check_buffer(buf);

      obj.buf = [obj.buf; buf];
    end    

    % Bufxx interface methods
    % -----------------------
    function close(obj)
      % just for interface
    end
    
    function i = tell(obj)
      i = obj.cursor;
    end

    function status = seek(obj, offset, origin)
      % offset: matlab-style indexing (from 1 to N)
      % origin: 
      %   -1 beginning of file 
      %    0 current position
      %    1 end of file
      %
      % Returns:
      % 0 if ok, -1 if fail.

      cursor_ = obj.cursor;
      if origin == -1       
        obj.cursor = offset;
      elseif origin == 0
        obj.cursor = obj.cursor + offset;
      elseif origin == 1
        obj.cursor = length(obj.buf) - offset;
      else
        error('wrong origin');
      end

      status = (obj.cursor >= 0 && obj.cursor <= length(obj.buf));
      if status 
        status = 0;
      else
        status = -1;
        obj.cursor = cursor_;
      end
    end    

    
    function status = eof(obj)
      status = obj.cursor == length(obj.buf);
    end


    function n = size(obj)
      n = length(obj.buf);
    end


    function [A, count] = read(obj, size, count, typename)
      if obj.mode ~= "r"
        error('wrong mode for reading');
      end
      if nargin < 4
        typename = 'char*1';
      end
      
      begin_ = obj.cursor;
      end_ = begin_ + size*count;

      A = obj.buf(begin_+1:end_);
      if any(strcmp(typename, {'char', 'char*1'}))
        A = char(A);
      else
        A = typecast(A, typename);
      end
      obj.cursor = end_;
    end


    function count = write(obj, A)
      if obj.mode ~= 'w'
        error('wrong mode for writing');
      end

      if ischar(A) % strings
        data_ = uint8(A);
      elseif isstring(A)
        data_ = uint8(char(A));
      else
        data_ = typecast(A, 'uint8');
      end

      % Note: may be not the most optimal
      begin_ = obj.cursor;
      end_ = begin_ + length(data_);
      obj.buf(begin_+1:end_, 1) = data_;
      count = length(data_);
      obj.cursor = length(obj.buf);
    end    
  end
end


function check_buffer(buf)
  if ~iscolumn(buf)
    error('wrong shape of `buff` %s', mat2str(size(buf)));
  end
  if ~isa(buf, 'uint8')
    error('wrong data type of buffer :%s', class(buf));
  end
end
