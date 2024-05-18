classdef Endian

  properties (Constant)
    little  = 'l'
    big     = 'b'
    native  = xmat.byteorder()
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
  end
end
