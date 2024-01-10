classdef Util
  % about: char*1 datatype
  % https://uk.mathworks.com/help/matlab/ref/fwrite.html#buakf91-1-precision
  %

  properties (Constant)
    tbyte = 'bit8';
    uintmap = {@uint8, @uint16, 0, @uint32, 0, 0, 0, @uint64};
    uintmap_char = {'uint8', 'uint16', 0, 'uint32', 0, 0, 0, 'uint64'};
  end

  methods (Static)
    function s = ljust(s, len, ch)
      if nargin == 2
        ch = char(0);
      end
      if ischar(s)
        s = append(s, repmat(ch, 1, len - length(s)));
      else

      end
    end
    
    
    function s = uljust(s, ch)
      if nargin < 2 || isempty(ch)
        ch = char(0);
      end
      s(s == ch) = [];
    end


    function s = read_string(is, len, order)
      % read ASCII string
      % len: string length
      if nargin < 3
        order = 'little';
      end
      s = fread(is, len, 'bit8=>char')';
    end


    function write_string(os, value, order)
      % write string as ASCII
      % Note: doesn't add '0' character to the end
      if nargin < 3
        order = 'little';
      end
      fwrite(os, value);
    end


    function num = read_uintx(is, len, order)
      if nargin < 3
        order = 'little';
      end
      % num = fread(is, 1, [xmat.Util.uintmap_char{len} '=>' xmat.Util.uintmap_char{len}]);
      num0 = fread(is, len, 'uint8=>uint8');
      num = typecast(num0, xmat.Util.uintmap_char{len});
    end


    function write_uintx(os, value, len, order)
      if nargin < 3
        order = 'little';
      end
      if ~isscalar(value)
        error('value must be a scalar');
      end
      val = xmat.Util.uintmap{len}(value);
      num = typecast(val, 'uint8');
      fwrite(os, num, 'uint8');
    end


    function read_block_buffer(buff, block, order)
      if nargin < 3
        order = 'little';
      end
    end
  end
end
