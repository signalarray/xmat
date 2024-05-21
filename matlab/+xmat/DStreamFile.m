classdef DStreamFile < xmat.DStream_

  properties
    filename
    fid = -1
  end

  
  methods (Static)
    function ods = out(filename)
      ods = xmat.DStreamFile(filename, 'w');
    end

    function ids = in(filename)
      ids = xmat.DStreamFile(filename, 'r');
    end
  end


  methods
    function obj = DStreamFile(filename, mode)
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

    function count = do_write(obj, A)
      if isstring(A) || ischar(A)
        count = fwrite(obj.fid, A, 'char*1', 0, obj.endian);
      else
        typeinfo = xmat.DataType.by_value(A);
        count = fwrite(obj.fid, A, typeinfo.typename, 0, obj.endian);
      end
    end

    function [A, count] = do_read(obj, count, typeinfo)
      if any(strcmp(typeinfo.typename, {'char', 'char*1'}))        
        [A, count] = fread(obj.fid, typeinfo.size*count, 'char*1=>char*1', obj.endian);
      else % for numbers
        [A, count] = fread(obj.fid, typeinfo.size*count, 'uint8=>uint8', obj.endian);
        A = typecast(A, typeinfo.typename);
      end
    end
  end
end
