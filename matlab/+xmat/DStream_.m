classdef DStream_ < handle
  
  properties
    mode
    endian = xmat.Endian.native
  end
  
  methods (Abstract)
    do_write(obj, A)    
    do_read(obj, numel, tid)
  end

  methods 

    function count = write(obj, A)
      % Parameters
      % ----------
      % A
      if obj.mode ~= 'w'
        error('wrong mode for writing');
      end

      % check if registered
      typeinfo = xmat.DataType.by_value(A);
      if isempty(typeinfo)
        error("Wrong type %s\n", class(A));
      end

      % if complex
      if isnumeric(A) && ~isreal(A)
        A = transpose([real(A(:)), imag(A(:))]);
        A = reshape(A, [], 1);
      end

      % call abstract
      count = obj.do_write(A);
    end


    function [A, numel] = read(obj, numel, tid)
      % Parameters
      % ----------
      % numel: 
      % tid: number or string

      if obj.mode ~= "r"
        error('wrong mode for reading');
      end

      if isnumeric(tid)
        typeinfo = xmat.DataType.by_id(tid);
      else
        typeinfo = xmat.DataType.by_name(tid);
      end
      if isempty(typeinfo)
        error("wrong type\n");
      end

      % call abstract
      [A, numel] = obj.do_read(numel, typeinfo);

      if xmat.DataType.iscomplex(typeinfo.id)
        A = complex(A(1:2:end), A(2:2:end));
      end
    end
  end
end

