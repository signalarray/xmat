classdef Util
  % about: char*1 datatype
  % https://uk.mathworks.com/help/matlab/ref/fwrite.html#buakf91-1-precision
  %

  properties (Constant)
    uintmap = {@uint8, @uint16, [], @uint32, [], [], [], @uint64};
    uintmap_char = {'uint8', 'uint16', '', 'uint32', '', '', '', 'uint64'};

    k_TYPES_MAP_NATIVE = struct( ...
      'char',   {{'qchar', 1}}, ...
      'int8',   {{'xi08', 1}}, ...
      'int16',  {{'xi16', 2}}, ...
      'int32',  {{'xi32', 4}}, ...
      'int64',  {{'xi64', 8}}, ...
      'uint8',  {{'xu08', 1}}, ...
      'uint16', {{'xu16', 2}}, ...
      'uint32', {{'xu32', 4}}, ...
      'uint64', {{'xu64', 8}}, ...
      'single', {{'xf32', 4}}, ...
      'double', {{'xf64', 8}});

    k_TYPES_MAP_XMAT = make_type_info_table();
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


    function typename = native2xmat_type(A)
      class_ = class(A);
      typename = xmat.Util.k_TYPES_MAP_NATIVE.(class_){1};
      if ischar(A) || isstring(A)
        return
      end

      if isnumeric(A) && isreal(A)
        typename(1) = 'r';
      else
        typename(1) = 'c';
      end
    end


    function B = change_order(A)
      B = permute(reshape(A(:), flip(size(A))), ...
                  ndims(A):-1:1);
    end
  end
end


function S = make_type_info_table()
  S = struct();
  fields_ = fields(xmat.Util.k_TYPES_MAP_NATIVE);
  for n = 1:length(fields_)
    a = fields_{n};
    b = xmat.Util.k_TYPES_MAP_NATIVE.(a);
    key = b{1};
    if key(1) == 'q'
      S.(key) = {a, b{2}};
    else
      key(1) = 'r';
      S.(key) = {a, b{2}};
      key(1) = 'c';
      S.(key) = {a, 2*b{2}};
    end
  end
end
