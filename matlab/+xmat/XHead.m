classdef XHead < handle

  properties
    fmt_signature       = xmat.XUtil.k_fmt_signature
    xsize_t_size        = xmat.XUtil.k_xsize_t_size
    max_block_name_len  = xmat.XUtil.k_max_block_name_len
    max_type_name_len   = xmat.XUtil.k_max_type_name_len
    max_ndim            = xmat.XUtil.k_max_ndim

    total_size          = 0

    xsize_t_type        = xmat.XUtil.k_xsize_t_type
    xsize_t_typename    = xmat.XUtil.k_xsize_t_typename
  end


  methods
    function obj = XHead()
    end

    function obj = dump(obj, os)
      % os: output stream/file
      os.write(uint64(0));
      os.write(xmat.XUtil.k_fmt_signature);
      os.write(uint8(obj.xsize_t_size));
      os.write(uint8(obj.max_block_name_len));
      os.write(uint8(obj.max_type_name_len));
      os.write(uint8(obj.max_ndim));
      os.write(xmat.XUtil.k_fmt_signature);
    end

    function obj = load(obj, is)
      % is: input stream/file
      obj.total_size      = double(is.read(8, 1, 'uint64'));
      obj.fmt_signature   = is.read(1, xmat.XUtil.k_fmt_signature_size, 'char*1')';
      if ~strcmp(obj.fmt_signature, xmat.XUtil.k_fmt_signature)
        error('xmat.XHead.load(). wrong fmt_signature: %s vs %', ...
          obj.fmt_signature, xmat.XUtil.k_fmt_signature_size);
      end
      obj.xsize_t_size        = double(is.read(1, 1, 'uint8'));
      obj.max_block_name_len  = double(is.read(1, 1, 'uint8'));
      obj.max_type_name_len   = double(is.read(1, 1, 'uint8'));
      obj.max_ndim            = double(is.read(1, 1, 'uint8'));
      
      sig1 = is.read(1, xmat.XUtil.k_fmt_signature_size, 'char*1')';
      if ~strcmp(sig1, obj.fmt_signature)
        error('xmat.XHead.load(). wrong fmt_signature. (in the end)');
      end

      % check header is consistent
    end

    function print(obj, fid)
      if nargin < 2
        fid = 1;
      end
      fprintf(fid, 'xmat.XHead:\n');
      fprintf(fid, '%24s: %d\n', 'total', obj.total_size);
      fprintf(fid, '%24s: `%s`\n', 'signature', obj.fmt_signature);
      fprintf(fid, '%24s: %d\n', 'sizeof(xsize_t)', obj.xsize_t_size);
      fprintf(fid, '%24s: %d\n', 'max_block_name', obj.max_block_name_len);
      fprintf(fid, '%24s: %d\n', 'max_type_name', obj.max_type_name_len);
      fprintf(fid, '%24s: %d\n', 'max_ndim', obj.max_ndim);
    end
  end
end
