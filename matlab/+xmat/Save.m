classdef Save < handle
    
  properties
    stream
    h
  end
  

  methods (Static)
    function xout = file(filename, byteorder)
      if nargin < 2 || isempty(byteorder)
        byteorder = [];
      end

      stream = xmat.StreamFile(filename, 'w', byteorder);
      xout = xmat.Save(stream);
    end


    function xout = bytes(byteorder)
      if nargin < 1 || isempty(byteorder)
        byteorder = [];
      end

      stream = xmat.StreamBytes('w', [], byteorder);
      xout = xmat.Save(stream);
    end
  end


  methods
    function obj = Save(stream)
      obj.stream = stream;
      obj.h = xmat.Header();

      obj.h.write(obj.stream);
    end


    function add(obj, name, A)
      if isstruct(X)
        obj.add_struct(obj, X);
      end
      
    end


    function add_struct(obj, name, A)
      if ~isstruct(A)
        error('A must be a struct');
      end
    end
  end
end
