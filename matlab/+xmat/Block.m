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
    pos_data
  end


  methods (Static)
    function bd = make(A, name, ndim_force)
      if nargin < 2
        ndim_force = [];
      end

      if nargin < 2
        name = 'default';
      end
      typename = xmat.Util.native2xmat_type(A);
      shape = size(A);
      typesize = xmat.Util.k_TYPES_MAP_XMAT.(typename){2};
      ndim = ndims(A);

      if ~isempty(ndim_force)
        if ndim_force < ndims(A)
          error('wrong ndim_force: %d', ndim_force);
        end
        shape(end + 1 : end + (ndim_force - ndim)) = 1;
        ndim = ndim_force;
      end
      bd = xmat.Block(name, typename, typesize, ndim, shape);
    end


    function bd = read(is, h)
      % h: xmat.Header
      pos = is.tell();
      sig0 = is.read(1, xmat.Header.k_FORMAT_SIGNATURE_SIZE, 'char*1')';
      if ~strcmp(sig0, xmat.Block.k_SIGNATURE_BEGIN)
        error('xmat.block error. wrong signature begin');
      end

      name      = is.read(1, h.max_block_name_len, 'char*1')';
      typename  = is.read(1, h.max_type_name_len, 'char*1')';
      shape     = double(is.read(h.intx_size, h.max_ndim, h.intx_typename))';
      numel_    = double(is.read(h.intx_size, 1, h.intx_typename));
      typesize  = double(is.read(1, 1, 'uint8'));
      ndim      = double(is.read(1, 1, 'uint8'));
      sig1      = is.read(1, xmat.Header.k_FORMAT_SIGNATURE_SIZE, 'char*1')';

      if ~strcmp(sig1, xmat.Block.k_SIGNATURE_END)
        error('xmat.block error. wrong signature end');
      end

      name = xmat.Util.uljust(name);
      typename = xmat.Util.uljust(typename);
      shape(ndim+1:end) = [];
      if numel_ ~= prod(shape)
        error('xmat.Block error. file. wrong numel');
      end      

      bd = xmat.Block(name, typename, typesize, ndim, shape);
      bd.pos = pos;
      bd.pos_data = is.tell();
    end    
  end


  methods
    function obj = Block(name, typename, typesize, ndim, shape)
      obj.name = name;
      obj.typename = typename;
      obj.shape = shape;
      obj.numel = prod(shape);
      obj.typesize = typesize;
      obj.ndim = ndim;
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


    function print(obj, args)
      if nargin < 2
        args.span = 1;
      end
      template = sprintf('%%%ss: <%%s, %%s> %%s', num2str(args.span));

      class_ = xmat.Util.k_TYPES_MAP_XMAT.(obj.typename){1};
      fprintf(template, obj.name, class_, obj.typename, mat2str(obj.shape));
    end
  end
end
