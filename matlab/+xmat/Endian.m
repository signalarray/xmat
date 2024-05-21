classdef Endian

  properties (Constant)
    little  = 'l'
    big     = 'b'
    native  = byteorder()
    notnative = xmat.Endian.other(xmat.Endian.native)
  end


  methods (Static)
    function X = set(X, endian)
      % Parameters:
      % X: numeric value non-complex:
      %   a scalar, vector, matrix, or multidimensional array. 
      %   The swapbytes operation is elementwise when X is nonscalar.
      % endian: {'l', 'b'}
      % 
      % single|double|int8|int16|int32|int64|uint8|uint16|uint32|uint64

      if endian ~= xmat.Endian.native
        X = swapbytes(X);
      end
    end

    function endian = other(endian)
      if endian == xmat.Endian.little
        endian = xmat.Endian.big;
      else
        endian = xmat.Endian.little;
      end
    end
  end
end


function out = byteorder()
x = uint16(1);
x = typecast(x, 'uint8');
if x(1) == 1
  out = 'l';
else
  out = 'b';
end
end
