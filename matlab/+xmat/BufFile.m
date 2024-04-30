classdef BufFile < handle

  properties
    filename
    mode
    
    fid = -1;
    byteorder = 'n'
  end

  
  methods (Static)
    function bfout = out(filename)
      bfout = xmat.BufFile(filename, 'w');
    end

    function bfin = in(filename)
      bfin = xmat.BufFile(filename, 'r');
    end
  end


  methods
    function obj = BufFile(filename, mode)
      % filename: str
      % mode: {'r', 'w'}

      if ~any(mode == ["r", "w"])
        error('xmat.BufByte.ctor(..). wrong `mode` value')
      end

      obj.filename = filename;
      obj.mode = mode;
      obj.fid = fopen(obj.filename, obj.mode);
    end


    function close(obj)
      fclose(obj.fid);
    end


    function i = tell(obj)
      i = ftell(obj.fid);
    end


    function status = seek(obj, offset, origin)
      status = fseek(obj.fid, offset, origin);
    end


    function status = eof(obj)
      status = feof(obj.fid);
    end


    function n = size(obj)
      pos = ftell(obj.fid);
      fseek(obj.fid, 0, 1);
      n = ftell(obj.fid);
      fseek(obj.fid, pos, -1);
    end
    
    
    function [A, count] = read(obj, size, count, typename)
      if obj.mode ~= 'r'
        error('wrong mode for reading');
      end
      if nargin < 4
        typename = 'char*1';
      end

      % if strcmp(typename, 'char*1')
      if any(strcmp(typename, {'char', 'char*1'}))        
        [A, count] = fread(obj.fid, size*count, 'char*1=>char*1', obj.byteorder);
      else % for numbers
        [A, count] = fread(obj.fid, size*count, 'uint8=>uint8', obj.byteorder);
        A = typecast(A, typename);
      end
    end


    function count = write(obj, A)
      if obj.mode ~= 'w'
        error('wrong mode for writing');
      end

      if isstring(A) || ischar(A) % strings
        count = fwrite(obj.fid, A, 'char*1', 0, obj.byteorder);
      else
        data = typecast(A, 'uint8');
        count = fwrite(obj.fid, data, 'uint8', 0, obj.byteorder);
      end
    end
  end
end
