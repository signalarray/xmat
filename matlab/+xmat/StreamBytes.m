classdef StreamBytes < handle
    
  properties
    mode
    byteorder

    buff
    cursor
  end
  
  methods
    function obj = StreamBytes(mode, buff, byteorder)
      % Parameters:
      % -----------
      % mode: {'r', 'w'}
      % buff: uint8[n x 1]
      % byteorder: 

      if nargin < 2 || isempty(buff)
        buff = uint8(zeros(0, 1));
      end
      if nargin < 3
        byteorder = 'n';
      end

      if ~iscolumn(buff)
        error('wrong shape of `buff` %s', mat2str(size(buff)));
      end
      if ~isa(buff, 'uint8')
        error('wrong data type of buffer :%s', class(buff));
      end      
      
      obj.mode = mode;
      obj.byteorder = byteorder;

      obj.buff = buff;
      obj.cursor = 0;
    end


    function close(obj)
      fprintf('');
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
        obj.cursor = length(obj.buff) - offset;
      else
        error('wrong origin');
      end

      status = (obj.cursor >= 0 && obj.cursor <= length(obj.buff));
      if status 
        status = 0;
      else
        status = -1;
        obj.cursor = cursor_;
      end
    end    

    
    function status = eof(obj)
      status = obj.cursor == length(obj.buff);
    end


    function n = size(obj)
      n = length(obj.buff);
    end


    function [A, count] = read(obj, size, count, typename)
      if obj.mode ~= 'r'
        error('wrong mode for reading');
      end
      if nargin < 4
        typename = 'char*1';
      end
      
      begin_ = obj.cursor;
      end_ = begin_ + size*count;

      A = obj.buff(begin_+1:end_);
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
      obj.buff(begin_+1:end_, 1) = data_;
      count = length(data_);
      obj.cursor = length(obj.buff);
      % assert(length(obj.buff) == end_);
    end    
  end
end
