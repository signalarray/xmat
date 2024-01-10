classdef Header < handle
  
  properties (Constant)
    k_DTYPE_INFO_MAP = make_dtype_info_map();
    
    k_FORMAT_SIGNATURE_SIZE = 4;
    k_FORMAT_SIGNATURE = ['XYZ'  char(0)];
    k_FORMAT_FOOTER = 'END'
    k_INTX_SIZE = 4
    k_MAX_BLOCK_NAME_LEN = 32
    k_MAX_TYPE_NAME_LEN = 8
    k_MAX_NDIM = 4
  end


  properties
    format_signature
    intx_size
    max_block_name_len
    max_type_name_len
    max_ndim

    intx_type
    intx_typename
  end


  methods (Static)
    function h = read(is)
      % is: input stream/file
      sig0 = is.read(1, xmat.Header.k_FORMAT_SIGNATURE_SIZE, 'char*1')';
      if ~strcmp(sig0, xmat.Header.k_FORMAT_SIGNATURE)
        error('xmat.header errof. wrong signature');
      end
      intx_size = is.read(1, 1, 'uint8');
      maxblocknamelen = is.read(1, 1, 'uint8');
      maxtypenamelen = is.read(1, 1, 'uint8');
      maxndim = is.read(1, 1, 'uint8');
      sig1 = is.read(1, xmat.Header.k_FORMAT_SIGNATURE_SIZE, 'char*1')';
      if ~strcmp(sig0, sig1)
        error('xmat.header error. wrong second signature');
      end
      h = xmat.Header(sig0, intx_size, maxblocknamelen, maxtypenamelen, maxndim);
    end
  end

  
  methods
    function obj = Header(signature, intx_size, max_block_name_len, max_type_name_len, max_ndim)
      if nargin < 1 || isempty(signature)
        signature = xmat.Header.k_FORMAT_SIGNATURE;
      end
      if nargin < 2 || isempty(intx_size)
        intx_size = xmat.Header.k_INTX_SIZE;
      end
      if nargin < 3 || isempty(max_block_name_len)
        max_block_name_len = xmat.Header.k_MAX_BLOCK_NAME_LEN;
      end
      if nargin < 4 || isempty(max_type_name_len)
        max_type_name_len = xmat.Header.k_MAX_TYPE_NAME_LEN;
      end
      if nargin < 5 || isempty(max_ndim)
        max_ndim = xmat.Header.k_MAX_NDIM;
      end

      obj.format_signature = signature;
      obj.intx_size = intx_size;
      obj.max_block_name_len = max_block_name_len;
      obj.max_type_name_len = max_type_name_len;
      obj.max_ndim = max_ndim;

      obj.intx_type = xmat.Util.uintmap{obj.intx_size};
      obj.intx_typename = xmat.Util.uintmap_char{obj.intx_size};
    end


   function write(obj, os)
      % os: output stream/file
      os.write(xmat.Header.k_FORMAT_SIGNATURE);
      os.write(uint8(obj.intx_size));
      os.write(uint8(obj.max_block_name_len));
      os.write(uint8(obj.max_type_name_len));
      os.write(uint8(obj.max_ndim));
      os.write(xmat.Header.k_FORMAT_SIGNATURE);
    end


    function n = byte_size(obj)
      n = 2 * xmat.Header.FORMAT_SIGNATURE_SIZE + 1 + 3 * obj.intx_size;
    end
  end
end


function dtype_info_map = make_dtype_info_map()
dtype_info_map = containers.Map();
dtype_info_map('char') = 'char';
end
