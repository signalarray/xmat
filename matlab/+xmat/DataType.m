classdef DataType
  
  
  properties (Constant)
      k_by_type_re = struct(...
        "char",      struct("typename", "char",     "id", hex2dec('0x01'), "label", 'c', "size", 1), ...
        "logical",   struct("typename", "logical",  "id", hex2dec('0x02'), "label", '?', "size", 1), ...
        ...
        "int8",      struct("typename", "int8",     "id", hex2dec('0x10'), "label", 'i0', "size", 1), ...
        "int16",     struct("typename", "int16",    "id", hex2dec('0x11'), "label", 'i1', "size", 2), ...
        "int32",     struct("typename", "int32",    "id", hex2dec('0x12'), "label", 'i2', "size", 4), ...
        "int64",     struct("typename", "int64",    "id", hex2dec('0x13'), "label", 'i3', "size", 8), ...
        ...
        "uint8",     struct("typename", "uint8",    "id", hex2dec('0x30'), "label", 'u0', "size", 1), ...
        "uint16",    struct("typename", "uint16",   "id", hex2dec('0x31'), "label", 'u1', "size", 2), ...
        "uint32",    struct("typename", "uint32",   "id", hex2dec('0x32'), "label", 'u2', "size", 4), ...
        "uint64",    struct("typename", "uint64",   "id", hex2dec('0x33'), "label", 'u3', "size", 8), ...
        ...
        "single",    struct("typename", "single",   "id", hex2dec('0x52'), "label", 'f2', "size", 4), ...
        "double",    struct("typename", "double",   "id", hex2dec('0x53'), "label", 'f3', "size", 8))


      k_by_type_cx = struct(...
        "int8",      struct("typename", "int8",     "id", hex2dec('0x20'), "label", 'I0', "size", 2), ...
        "int16",     struct("typename", "int16",    "id", hex2dec('0x21'), "label", 'I1', "size", 4), ...
        "int32",     struct("typename", "int32",    "id", hex2dec('0x22'), "label", 'I2', "size", 8), ...
        "int64",     struct("typename", "int64",    "id", hex2dec('0x23'), "label", 'I3', "size", 16), ...
        ...
        "uint8",     struct("typename", "uint8",    "id", hex2dec('0x40'), "label", 'U0', "size", 2), ...
        "uint16",    struct("typename", "uint16",   "id", hex2dec('0x41'), "label", 'U1', "size", 4), ...
        "uint32",    struct("typename", "uint32",   "id", hex2dec('0x42'), "label", 'U2', "size", 8), ...
        "uint64",    struct("typename", "uint64",   "id", hex2dec('0x43'), "label", 'U3', "size", 16), ...
        ...
        "single",    struct("typename", "single",   "id", hex2dec('0x62'), "label", 'F2', "size", 8), ...
        "double",    struct("typename", "double",   "id", hex2dec('0x63'), "label", 'F3', "size", 16))


      k_by_id = make_table_id()

      % --------
      % head
      k_xsize_t         = @uint64
      k_xsize_typename  = class(xmat.DataType.k_xsize_t(1))
      k_xuint8_t        = @uint8

      k_signature       = 'xmat'
      k_signature_size  = 4
      k_bom             = hex2dec('01')
      k_sizeof_int      = length(typecast(xmat.DataType.k_xsize_t(0), 'int8'))

      k_morder          = 'F'
      k_max_ndim        = 8   % for default value
      k_max_name        = 32  % for default value
  end
  

  methods (Static)
    function info = by_value(A)
      info = [];
      typename = class(A);
      if isnumeric(A) && ~isreal(A) % complex
        if isfield(xmat.DataType.k_by_type_cx, typename)
          info = xmat.DataType.k_by_type_cx.(typename);
        end
      else % char and other
        if isfield(xmat.DataType.k_by_type_re, typename)
          info = xmat.DataType.k_by_type_re.(typename);
        end
      end
    end


    function info = by_id(id)
      info = xmat.DataType.k_by_id{id};
    end


    function info = by_name(typename)
      if length(typename) > 3 && strcmp(typename(1:3), 'cx_')
        info = xmat.DataType.k_by_type_cx.(typename(4:end));
      else
        info = xmat.DataType.k_by_type_re.(typename);
      end
    end

    function s = name(id)
      info = xmat.DataType.by_id(id);
      if xmat.DataType.iscomplex(info.id)
        s = strcat('cx_', info.typename);
      else
        s = info.typename;
      end
    end

    function out = iscomplex(id)
      info = xmat.DataType.k_by_id{id};
      out = any(strcmp(info.label(1), ["I", "U", "F"]));
    end


    function out = isreal(id)
      out = ~xmat.DataType.iscomplex(id);
    end
  end
end



function out = make_table_id()
  out = cell(256, 1);
   
  fields = fieldnames(xmat.DataType.k_by_type_re);
  for n = 1:length(fields)
    item = xmat.DataType.k_by_type_re.(fields{n});
    out{item.id} = item;
  end

  fields = fieldnames(xmat.DataType.k_by_type_cx);
  for n = 1:length(fields)
    item = xmat.DataType.k_by_type_cx.(fields{n});
    out{item.id} = item;
  end
end
