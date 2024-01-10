classdef Block < handle

  properties (Constant)
    k_SIGNATURE_BEGIN = ['<#>' char(0)];
    k_SIGNATURE_END = ['>#<' char(0)];
  end


  properties
    name
    typename
    typesize
    shape    
    numel
    ndim
    pos
  end


  methods (Static)
    function make(name, X)
      typename = class(X);
      shape = size(X);
      numel = numel(X);
      ndim = ndims(X);

      % check

    end


    function bd = read(is, h)
      % h: xmat.Header
      pos = is.pos();
      sig0 = is.read(1, xmat.Header.k_FORMAT_SIGNATURE_SIZE, 'char*1')';
      if ~strcmp(sig0, xmat.Block.k_SIGNATURE_BEGIN)
        error('xmat.block error. wrong signature');
      end

      name      = is.read(1, h.max_block_name_len, 'char*1')';
      typename  = is.read(1, h.max_type_name_len, 'char*1')';
      shape     = is.read(h.intx_size, h.max_ndim, h.intx_typename);
      numel_    = is.read(h.intx_size, 1, h.intx_typename);
      typesize  = is.read(1, 1, 'uint8');
      ndim      = is.read(1, 1, 'uint8');
      sig1      = is.read(1, xmat.Header.k_FORMAT_SIGNATURE_SIZE, 'char*1')';

      if ~strcmp(sig1, xmat.Block.k_SIGNATURE_END)
        error('xmat.block error. wrong signature');
      end

      shape(ndim+1:end) = [];
      if numel_ ~= prod(shape)
        filename = fopen(is);
        error('xmat.Block error. file %s. wrong numel', filename);
      end      

      bd = xmat.Block(name, typename, typesize, ndim, shape, pos);
    end    
  end


  methods
    function obj = Block(name, typename, typesize, ndim, shape, pos)
      if nargin < 6
        pos = -1;
      end
      obj.name = name;
      obj.typename = typename;
      obj.shape = shape;
      obj.numel = prod(shape);
      obj.typesize = typesize;
      obj.ndim = ndim;
      obj.pos = pos;    % position in file
    end

    function write(obj, os, h)
      % Parameters:
      % -----------
      % os: output stream
      % h: xmat.Handle

      shape_ = [obj.shape  zeros(1, h.max_ndim - length(obj.shape))];

      os.write(xmat.Block.k_SIGNATURE_BEGIN);
      os.write(xmat.Util.ljust(obj.name, h.max_block_name_len));
      os.write(xmat.Util.ljust(obj.typename, h.max_type_name_len));
      os.write(cast(shape_, h.intx_typename));
      os.write(cast(obj.numel, h.intx_typename));
      os.write(uint8(obj.typesize));
      os.write(uint8(obj.ndim));
      os.write(xmat.Block.k_SIGNATURE_END);
    end    
  end
end
