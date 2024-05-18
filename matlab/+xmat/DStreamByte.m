classdef DStreamByte < xmat.DStream_
    
  properties
    buf = uint8(zeros(0, 1));
    cursor = 0
  end
  

  methods (Static)
    function ods = out()
      ods = xmat.DStreamByte('w');
    end

    function ids = in()
      ids = xmat.DStreamByte('r');
    end
  end


  methods
    function obj = DStreamByte(mode)
      % Parameters:
      % -----------
      % mode: {'r', 'w'}

      if ~any(mode == ["r", "w"])
        error('xmat.DStreamByte.ctor(..). wrong `mode` value')
      end
      obj.mode = mode;
    end

    % for deffered initialization
    function obj = set_buffer(obj, buf)
      if obj.mode ~= "r"
        error('xmat.DStreamByte.set_buffer(). wrong mode');
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

    % abstract implementation
    function count = do_write(obj, A)
      if ischar(A) % strings
        data_ = uint8(A);
      elseif isstring(A)
        data_ = uint8(char(A));
      else % numbers
        A = xmat.Endian.set(A, obj.endian);
        data_ = typecast(A, 'uint8');
      end

      % Note: may be not the most optimal
      begin_ = obj.cursor;
      end_ = begin_ + length(data_);
      obj.buf(begin_+1:end_, 1) = data_;
      count = length(data_);
      obj.cursor = length(obj.buf);
    end


    function [A, numel] = do_read(obj, numel, typeinfo)
      begin_ = obj.cursor;
      end_ = begin_ + typeinfo.size*numel;

      A = obj.buf(begin_ + 1 : end_);
      if any(strcmp(typeinfo.typename, {'char', 'char*1'}))
        A = char(A);
      else
        A = typecast(A, typeinfo.typename);
        A = xmat.Endian.set(A, obj.endian);
      end
      obj.cursor = end_;
    end


    function out = print(obj, width)
      if nargin < 2
        width = 8;
      end
      out = obj.buf;
      n = length(out);
      if rem(n, width) 
        out(end + 1: end + (width - rem(n, width))) = 256;
      end
      out = transpose(reshape(out, width, []));
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
