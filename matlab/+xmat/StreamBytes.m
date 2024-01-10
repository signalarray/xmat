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
        buff = uint8([]);
      end
      if nargin < 3
        byteorder = 'n';
      end
      
      obj.mode = mode;
      obj.byteorder = byteorder;

      obj.buff = buff;
      obj.cursor = 1;
    end


    function close(obj)
      fprintf('');
    end


    function i = pos(obj)
      i = -1;
    end


    function [A, count] = read(obj, size, count, typename)
      if obj.mode ~= 'r'
        error('wrong mode for reading');
      end
      if nargin < 4
        typename = 'char*1';
      end

      A = obj.buff(obj.cursor:obj.cursor + size*count - 1);
      if strcmp(typename, 'char*1')
        A = char(A);
      else
        A = typecast(A, typename);
      end
      obj.cursor = obj.cursor + size*count;
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
      obj.buff(end+1:end+length(data_), 1) = data_;
      count = length(data_);
    end    
  end
end
