classdef XBlock < handle

  properties
    morder    = xmat.DataType.k_morder  % o
    tid       = 0                       % t
    ndim      = 0                       % s
    namelen   = 0                       % b
    shape     = 0                       % Shape[ndim]
    name      = ''                      % Block Name[namelen]

    pos       = 0
  end

  methods
    function obj = XBlock()
      % pass
    end 

    function obj = make(obj, A, name, ndim_force)
      % ndim_force: uint
      %   requared ndims for array. matlab reduces last-one dimentions
      %   like bellow:
      %     ndims(ones(2, 2, 2, 1, 1)) := 3
      %   so, use `ndim_force` for fixing shape
      
      if nargin < 3
        name = 'default';
      end
      
      if nargin < 4
        ndim_force = [];
      end

      info = xmat.DataType.by_value(A);
      if isempty(info)
        error('wrong type %s\n', class(A));
      end
      
      % obj.morder
      obj.tid = info.id;
      obj.ndim = ndims(A);
      obj.namelen = strlength(name);
      obj.shape = size(A);
      obj.name = name;
      
      if ~isempty(ndim_force)   % ndim_force
        if ndim_force < ndims(A)
          error('xmat.XBlock.make(A). wrong ndim_force: %d', ndim_force);
        end
        obj.shape(end + 1 : end + (ndim_force - obj.ndim)) = 1;
        obj.ndim = ndim_force;
      end
    end

    function obj = dump(obj, ods)
      % Parameters: 
      % ods: DataStream_.Out
      assert(any(obj.morder == 'CF'))
      assert(~isempty(xmat.DataType.by_id(obj.tid)));
      % !! assert(obj.ndim == ndims(obj.shape));
      % !! may be: assert(obj.ndim == length(obj.shape));
      assert(obj.namelen == length(obj.name));

      ods.write(char(obj.morder));
      ods.write(uint8(obj.tid));
      ods.write(uint8(obj.ndim));
      ods.write(uint8(obj.namelen));
      ods.write(uint8(zeros(1, 4)));
      ods.write(xmat.DataType.k_xsize_t(obj.shape));
      ods.write(obj.name);
    end

    function obj = load(obj, ids)
      obj.pos = ids.tell();
      
      obj.morder = ids.read(1, 'char');
      if ~any(obj.morder == 'CF')
        error('xmat.XBlock.load(): wrong `morder`');
      end

      obj.tid = double(ids.read(1, 'uint8'));
      if isempty(xmat.DataType.by_id(obj.tid))
        error('xmat.XBlock.load(): wrong `tid`');
      end

      obj.ndim = double(ids.read(1, 'uint8'));
      obj.namelen = double(ids.read(1, 'uint8'));
      zeros_ = ids.read(4, 'char');
      if any(zeros_)
        error('xmat.XBlock.load(): error in -zeros- bytes %s\n', zeros);
      end
      obj.shape = ids.read(obj.ndim, xmat.DataType.k_xsize_typename).';
      obj.name = ids.read(obj.namelen, 'char').';
    end

    % getters
    % ------------------------
    function n = typesize(obj)
      n = xmat.DataType.by_id(obj.tid).size;
    end

    function n = nbytes(obj)
      n = 8 + xmat.DataType.k_sizeof_int * obj.ndim + strlength(obj.name);
    end

    function n = data_pos(obj)
      n = obj.pos + obj.nbytes();
    end

    function n = data_nbytes(obj)
      n = obj.typesize() * obj.numel();
    end

    function n = numel(obj)
      n = prod(obj.shape);
    end

    function print(obj, fid, args)
      % Example:
      % ones(5, 6) + 1j >> name: <cx_double, tid>, F:[5, 6]
      if nargin < 2
        fid = 1;
      end
      if nargin < 3
        args.span = 1;
        args.end_ = '\n';
      end
      template = sprintf('%%%ss: id:%%s %%12s %%s:%%s', num2str(args.span));
      info = xmat.DataType.by_id(obj.tid);
      info.typename = xmat.DataType.name(obj.tid);
      fprintf(fid, template, obj.name, ['0x' dec2hex(obj.tid, 2)], info.typename, ...
              obj.morder, mat2str(obj.shape));
      if isfield(args, 'end_')
        fprintf(fid, args.end_);
      end
    end
  end
end


