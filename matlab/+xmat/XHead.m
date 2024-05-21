classdef XHead < handle

  properties
    % See: Header Format
    signature   = xmat.DataType.k_signature       % Sign
    bom         = xmat.DataType.k_bom             % BOM
    total       = 0                               % Total Size
    sizeof_int  = xmat.DataType.k_sizeof_int      % I
    maxndim     = xmat.DataType.k_max_ndim        % S
    maxname     = xmat.DataType.k_max_name        % B
  end

  methods (Static)
    function n = nbytes()
      n = 17;
    end
  end

  methods
    function obj = XHead()
    end

    function obj = dump(obj, ods)
      % Parameters: 
      % ods: DataStream_.Out
      if length(obj.signature) ~= xmat.DataType.k_signature_size
        error("xmat.XHead.dump(): wrong size length %d\n", ...
              length(length(obj.signature)));
      end

      ods.write(obj.signature);
      ods.write(uint16(obj.bom));
      ods.write(xmat.DataType.k_xsize_t(obj.total));
      ods.write(uint8(obj.sizeof_int));
      ods.write(uint8(obj.maxndim));
      ods.write(uint8(obj.maxname));
    end

    function obj = load(obj, ids)
      % ods: DataStream_.In
      obj.signature = ids.read(xmat.DataType.k_signature_size, 'char')';
      if ~strcmp(obj.signature, xmat.DataType.k_signature)
        error('xmat.XHead.load(): wrong signature `%s`\n', obj.signature);
      end

      % endianess issues
      obj.bom = ids.read(1, 'uint16');
      if obj.bom ~= xmat.DataType.k_bom
        ids.endian = xmat.Endian.other(ids.endian);
        bom_ = xmat.Endian.set(obj.bom, ids.endian);
        if bom_ ~= xmat.DataType.k_bom
          error('xmat.XHead.load(): wrong `bom` value\n');
        end
        warning('xmat.XHead.load():  wrong endian value `%s` changed to: `%s`\n', ...
                 xmat.Endian.other(ids.endian), ids.endian);
        obj.bom = bom_;
      end
      obj.bom = double(obj.bom);

      obj.total = double(ids.read(1, xmat.DataType.k_xsize_typename));
      obj.sizeof_int = double(ids.read(1, 'uint8'));
      obj.maxndim = double(ids.read(1, 'uint8'));
      obj.maxname = double(ids.read(1, 'uint8'));

      if obj.sizeof_int ~= xmat.DataType.k_sizeof_int
        error("xmat.XHead.load(): wrong sizeof_int %d\n", obj.sizeof_int);
      end
    end

    function print(obj, fid)
      if nargin < 2
        fid = 1;
      end
      fprintf(fid, 'xmat.XHead:\n');
      fprintf(fid, '%24s: `%s`\n', 'signature', obj.signature);
      fprintf(fid, '%24s: %d\n', 'bom',         obj.bom);
      fprintf(fid, '%24s: %d\n', 'total',       obj.total);
      fprintf(fid, '%24s: %d\n', 'sizeof_int',  obj.sizeof_int);
      fprintf(fid, '%24s: %d\n', 'maxndim',     obj.maxndim);
      fprintf(fid, '%24s: %d\n', 'maxname',     obj.maxname);
    end
  end
end
