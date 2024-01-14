classdef Load < handle
  
  
  properties
    istream
    h

    map = struct();
    block_list = cell(0, 2);
  end
  
  
  methods (Static)
    function xin = file(filename, byteorder)
      if nargin < 2 || isempty(byteorder)
        byteorder = [];
      end

      stream = xmat.StreamFile(filename, 'r', byteorder);
      xin = xmat.Load(stream);
    end


    function xin = bytes(buff, byteorder, h)
      if nargin < 2 || isempty(byteorder)
        byteorder = [];
      end

      if nargin < 3 || isempty(h)
        h = [];
      end

      stream = xmat.StreamBytes('r', buff, byteorder);
      xin = xmat.Load(stream, h);
    end
  end


  methods
    function obj = Load(input_stream, header)
      obj.istream = input_stream;
      if nargin < 2 || isempty(header)
        header = xmat.Header.read(obj.istream);
        neof = header.total_size;
      else
        neof = header.total_size - xmat.Header.k_SIZEB;
      end
      obj.h = header;

      % make a list of all blocks
      while obj.istream.tell() < neof
        bd_ = xmat.Block.read(obj.istream, obj.h);
        obj.map.(bd_.name) = bd_;
        status = obj.istream.seek(bd_.typesize * bd_.numel, 0);
        assert(status == 0);
      end      
    end


    function A = load(obj, name)
      assert(isfield(obj.map, name));

      bd_ = obj.map.(name);
      obj.istream.seek(bd_.pos_data, -1);
      class_ = xmat.Util.k_TYPES_MAP_XMAT.(bd_.typename){1};
      if bd_.typename(1) == 'c' % complex
        A = obj.istream.read(bd_.typesize, bd_.numel, class_);
        A = complex(A(1:2:end), A(2:2:end));
      else
        A = obj.istream.read(bd_.typesize, bd_.numel, class_);        
      end
      A = reshape(A, bd_.shape);
    end
  end
end
