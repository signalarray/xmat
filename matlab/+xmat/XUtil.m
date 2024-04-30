classdef XUtil
  
  properties (Constant)
    % general
    k_xsize_t_type = @uint64
    k_xsize_t_typename = 'uint64'
    k_xsize_t_size = 8
    k_fmt_signature_size = 4;

    % head
    k_fmt_signature       = ['XYZ' char(0)];
    k_max_block_name_len  = 32;
    k_max_type_name_len   = 8;
    k_max_ndim            = 8;
    k_head_size = 8 + 2 * xmat.XUtil.k_fmt_signature_size + 4;

    % block
    k_block_signature_begin = ['<#>' char(0)];
    k_block_signature_end   = ['>#<' char(0)];
    k_block_size = ...
      xmat.XUtil.k_max_block_name_len + ...
      xmat.XUtil.k_max_type_name_len + ...
      (xmat.XUtil.k_max_ndim + 1) * xmat.XUtil.k_xsize_t_size + ...
      (1 + 1)*1 + 2 * xmat.XUtil.k_fmt_signature_size


    % type stuff
    % ----------
    k_types_map_native = struct( ...
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

    k_types_map_xmat = make_types_map_xmat();
  end
  

  methods (Static)
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

    function B = change_order(A)
      B = permute(reshape(A(:), flip(size(A))), ndims(A):-1:1);
    end

    function typename = native2xmat_type(A)
      % transform native Matlab class(A) to xmat type-names
      % Return:
      % -------
      % typename: char
      class_ = class(A);
      typename = xmat.XUtil.k_types_map_native.(class_){1};
      if ischar(A) || isstring(A)
        return
      end

      if isnumeric(A) && isreal(A)
        typename(1) = 'r';
      else
        typename(1) = 'c';
      end
    end
  end
end


% fills xmat.XUtil.k_types_map_xmat()
function S = make_types_map_xmat()
  S = struct();
  fields_ = fields(xmat.XUtil.k_types_map_native);
  for n = 1:length(fields_)
    a = fields_{n};
    b = xmat.XUtil.k_types_map_native.(a);
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
