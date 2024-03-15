classdef Input < handle
  
  properties
    istream
    h
    isclosed = false

    map;
    block_list = cell(0, 2);
  end


  methods (Static)
    function xin = from_file(filename, byteorder)
      if nargin < 2 || isempty(byteorder)
        byteorder = [];
      end

      stream = xmat.StreamFile(filename, 'r', byteorder);
      xin = xmat.Input(stream);
    end


    function xin = from_bytes(buff, byteorder)
      if nargin < 2 || isempty(byteorder)
        byteorder = [];
      end

      stream = xmat.StreamBytes('r', buff, byteorder);
      xin = xmat.Input(stream);
    end
  end


  methods
    function obj = Input(input_stream)
      obj.map = containers.Map();
      obj.istream = input_stream;
      if ~(isa(input_stream, 'xmat.StreamBytes') && isempty(input_stream.buff))
        obj.scan();
      end
    end


    function A = getitem(obj, name, order_tag)
		  % order_tag: 'c' or 'f'
      %   `f` - default. `c` - says save element order as F-order, and flip shape instead.
			
      if nargin < 3
        order_tag = 'f';
      end
      if ~any(strcmp(order_tag, ["c", "f"]))
        error('wrong order tag: %s', order_tag);
      end

      if ~(isstring(name) || ischar(name))
        errror('wront input parameter type: %s, `string` expected', class(name));
      end

      if ~isKey(obj.map, name)
        A = [];
      else
        bd_ = obj.map(name);
        obj.istream.seek(bd_.pos_data, -1);
        class_ = xmat.Util.k_TYPES_MAP_XMAT.(bd_.typename){1};
        if bd_.typename(1) == 'c' % complex
          if contains(class_, 'int')
            A = [];
            warning('complex<integer:%s> types aren`t supported', class);
            return;
          end
          A = obj.istream.read(bd_.typesize, bd_.numel, class_);
          A = complex(A(1:2:end), A(2:2:end));
        else
          A = obj.istream.read(bd_.typesize, bd_.numel, class_);        
        end

        %%% supposes that all arrays are stored in xmat in C-elemen-order
        if length(bd_.shape) == 1
          bd_.shape = [1 bd_.shape];  % 1d-vector is a row<1, N>
        end

        if order_tag == 'c'
          bd_.shape = flip(bd_.shape);
          A = reshape(A, bd_.shape);
        else
          A = permute(reshape(A, flip(bd_.shape)), length(bd_.shape):-1:1);
        end
      end
    end

    % exploration
    % -----------
    function flag = has(field_name)
      flag = isfield(obj.map, field_name);
    end

    % access
    % ------
    function print(obj)
      fprintf('xmat.Input:\n');
      fields = keys(obj.map);
      span = 0;
      for n = 1:length(fields)
        span = max(length(fields{n}), span);
      end
      args.span = span + 2;
      for n = 1:length(fields)
        bd_ = obj.map(fields{n});
        bd_.print(args);
        fprintf('\n');
      end
      fprintf('\n');
    end

    function scan(obj)
      obj.scan_header();
      obj.scan_data();
    end

    function scan_header(obj)
      obj.h = xmat.Header.read(obj.istream);
    end

    function scan_data(obj)
      neof = obj.h.total_size;
      while obj.istream.tell() < neof
        bd_ = xmat.Block.read(obj.istream, obj.h);
        obj.map(bd_.name) = bd_;
        status = obj.istream.seek(bd_.typesize * bd_.numel, 0);
        assert(status == 0);
      end
    end
  end
end
