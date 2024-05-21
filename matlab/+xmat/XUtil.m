classdef XUtil
  
  methods (Static)
    function out = isstringlike(str)
      out = ischar(str) || isstring(str);
    end

    function str = ljust(str, len, fillchar)
      % analog for Python.str.ljust:
      % Return the string left justified in a string of length width
 
      if nargin == 2
        fillchar = char(0);
      end
      str = append(str, repmat(fillchar, 1, len - strlength(str)));
    end

    function str = uljust(str, fillchar)
      % opposite for ljust
      % Return the string with removed all tail elements eq to fillchar
      if nargin < 2 || isempty(fillchar)
        fillchar = char(0);
      end
      str(str == fillchar) = [];
    end
  end
end

