classdef XBlock < handle

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
      % ndim_force: uint
      %   requared ndims for array. matlab does reduce last-one dimentions
      %   like bellow:
      %     ndims(ones(2, 2, 2, 1, 1)) := 3
      
      if nargin < 2
        name = 'default';
      end
      if nargin < 3
        ndim_force = [];
      end

      typename = xmat.XUtil.native2xmat_type(A);
      shape = size(A);
      typesize = xmat.XUtil.k_types_map_xmat.(typename){2};
      ndim = ndims(A);

      if ~isempty(ndim_force)
        if ndim_force < ndims(A)
          error('xmat.XBlock.make(A). wrong ndim_force: %d', ndim_force);
        end
        shape(end + 1 : end + (ndim_force - ndim)) = 1;
        ndim = ndim_force;
      end
      bd = xmat.XBlock.make_(name, typename, typesize, ndim, shape);
    end

    function bd = make_(name, typename, typesize, ndim, shape)
      bd = xmat.XBlock();
      bd.name = name;
      bd.typename = typename;
      bd.shape = shape;
      bd.numel = prod(shape);
      bd.typesize = typesize;
      bd.ndim = ndim;
    end
  end


  methods
    function obj = XBlock()
      % pass
    end

    function obj = dump(obj, os)
      % Parameters:
      % -----------
      % os: output stream
      % h: xmat.Handle
      if strlength(obj.name) > xmat.XUtil.k_max_block_name_len
        error('too long block-name: len := %d, name := %s', strlength(obj.name), obj.name);
      end
      shape_ = [obj.shape  zeros(1, xmat.XUtil.k_max_ndim - length(obj.shape))];

      os.write(xmat.XUtil.k_block_signature_begin);
      os.write(xmat.XUtil.ljust(obj.name, xmat.XUtil.k_max_block_name_len));
      os.write(xmat.XUtil.ljust(obj.typename, xmat.XUtil.k_max_type_name_len));
      os.write(cast(shape_, xmat.XUtil.k_xsize_t_typename));
      os.write(cast(obj.numel, xmat.XUtil.k_xsize_t_typename));
      os.write(uint8(obj.typesize));
      os.write(uint8(obj.ndim));
      os.write(xmat.XUtil.k_block_signature_end);
    end

    function obj = load(obj, is)
      % is: BufByte('r') or BufFile('r')
      obj.pos = is.tell();
      sig0 = is.read(1, xmat.XUtil.k_fmt_signature_size, 'char*1')';
      if ~strcmp(sig0, xmat.XUtil.k_block_signature_begin)
        error('xmat.XBlock.load(..). wrong signature begin: %s vs %', ...
          sig0, xmat.XUtil.k_block_signature_begin);
      end

      obj.name      = is.read(1, xmat.XUtil.k_max_block_name_len, 'char*1')';
      obj.typename  = is.read(1, xmat.XUtil.k_max_type_name_len, 'char*1')';
      
      obj.shape     = double(is.read(xmat.XUtil.k_xsize_t_size, ...
                                     xmat.XUtil.k_max_ndim, ...
                                     xmat.XUtil.k_xsize_t_typename))';

      obj.numel     = double(is.read(xmat.XUtil.k_xsize_t_size, 1, ...
                                     xmat.XUtil.k_xsize_t_typename));
      
      obj.typesize  = double(is.read(1, 1, 'uint8'));
      obj.ndim      = double(is.read(1, 1, 'uint8'));
      sig1          = is.read(1, xmat.XUtil.k_fmt_signature_size, 'char*1')';

      if ~strcmp(sig1, xmat.XUtil.k_block_signature_end)
        error('xmat.XBlock.load(..). wrong signature end: %s vs %', ...
          sig0, xmat.XUtil.k_block_signature_end);
      end

      obj.name = xmat.XUtil.uljust(obj.name);
      obj.typename = xmat.XUtil.uljust(obj.typename);
      obj.shape(obj.ndim+1:end) = [];
      if obj.numel ~= prod(obj.shape)
        error('xmat.Block error. file. wrong numel');
      end      
      obj.pos_data = is.tell();
    end 

    function print(obj, fid, args)
      if nargin < 2
        args.span = 1;
      end
      template = sprintf('%%%ss: <%%s, %%s> %%s', num2str(args.span));

      class_ = xmat.XUtil.k_types_map_xmat.(obj.typename){1};
      fprintf(fid, template, obj.name, class_, obj.typename, mat2str(obj.shape));
    end
  end
end
