classdef Output < handle
    
  properties
    ostream
    h
    isclosed = false
  end
  

  methods (Static)
    function xout = from_file(filename, byteorder)
      if nargin < 2 || isempty(byteorder)
        byteorder = [];
      end

      stream = xmat.StreamFile(filename, 'w', byteorder);
      xout = xmat.Output(stream);
    end


    function xout = from_bytes(byteorder)
      if nargin < 1 || isempty(byteorder)
        byteorder = [];
      end

      stream = xmat.StreamBytes('w', [], byteorder);
      xout = xmat.Output(stream);
    end
  end


  methods
    function obj = Output(output_stream)
      obj.ostream = output_stream;
      obj.h = xmat.Header();

      obj.h.write(obj.ostream);
    end

    function setitem(obj, name, A)
      % write A to ostream
      % Parameters:
      % -----------
      % name: char
      % A: value

      if isstruct(A)
        obj.add_struct(obj, A);
        return
      end

      if isstring(A)
        A = char(A);
      end

      % write block descriptor
      bd = xmat.Block.make(A, name);
      bd.write(obj.ostream, obj.h);

      % write data
      if isreal(A)
        obj.ostream.write(A(:));
      else  % if complex
        A_ = transpose([real(A(:)), imag(A(:))]);
        obj.ostream.write(A_(:));
      end
    end


    function add_struct(obj, name, A)
      if ~isstruct(A)
        error('A must be a struct');
      end
    end


    function close(obj)
      % save header.total_size
      total_size = obj.ostream.size();
      obj.h.total_size = total_size;
      obj.ostream.seek(0, -1);
      obj.ostream.write(uint64(total_size));
      
      obj.ostream.close();
      obj.isclosed = true;
    end
  end
end
