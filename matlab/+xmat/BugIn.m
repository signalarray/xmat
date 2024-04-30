classdef BugIn < handle
  
  properties
    ibuf
    head
    is_open = true

    map;
    block_list = cell(0, 2);
  end


  methods (Static)
    function xin = file(filename)
      stream = xmat.BufFile(filename, 'r');
      xin = xmat.BugIn(stream, true);
    end

    function xin = byte(buf)
      % Parameters:
      %   buf: Bufxxx, optional
      stream = xmat.BufByte('r');
      xin = xmat.BugIn(stream, false);
      if nargin < 1
        return
      end
      xin.ibuf.set_buffer(buf);
      xin.scan();
    end
  end


  methods
    function obj = BugIn(input_stream, ready_for_scan)
      % ready_for_scan: logical(,true)
      %   used for deffered bytebufer initialization.
      if nargin < 2
        ready_for_scan = true;
      end

      obj.ibuf = input_stream;
      obj.head = xmat.XHead();
      obj.map = containers.Map();

      if (ready_for_scan)
        obj.scan();
      end
    end

    function obj = push_buffer(obj, buf)
      if ~ismethod(obj.ibuf, 'push_buffer')
        error('xmat.BugIn.push_buffer(..). xmat.BugIn.ibuf has no push_buff() method');
      end
      obj.ibuf.push_buffer(buf);
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
        obj.ibuf.seek(bd_.pos_data, -1);
        class_ = xmat.XUtil.k_types_map_xmat.(bd_.typename){1};
        if bd_.typename(1) == 'c' % complex
          if contains(class_, 'int')
            A = [];
            warning('complex<integer:%s> types aren`t supported', class);
            return;
          end
          A = obj.ibuf.read(bd_.typesize, bd_.numel, class_);
          A = complex(A(1:2:end), A(2:2:end));
        else
          A = obj.ibuf.read(bd_.typesize, bd_.numel, class_);        
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

    % scan for content
    % ----------------
    function scan(obj)
      obj.scan_header();
      obj.scan_data();
    end

    function scan_header(obj)
      obj.head = obj.head.load(obj.ibuf);
    end

    function scan_data(obj)
      neof = obj.head.total_size;
      while obj.ibuf.tell() < neof
        bd_ = xmat.XBlock();
        bd_.load(obj.ibuf);
        obj.map(bd_.name) = bd_;
        
        % advance buffer cursor for data block size
        status = obj.ibuf.seek(bd_.typesize * bd_.numel, 0);
        assert(status == 0);
      end
    end

    % print
    % -----
    function print(obj, fid)
      if nargin < 2
        fid = 1;
      end
      fprintf('xmat.BugIn:\n');
      obj.head.print(fid);
      fields = keys(obj.map);
      span = 0;
      for n = 1:length(fields)
        span = max(length(fields{n}), span);
      end
      args.span = span + 2;
      fprintf('blocks:\n');
      for n = 1:length(fields)
        fprintf(fid, '[%4.d]', n);
        bd_ = obj.map(fields{n});
        bd_.print(fid, args);
        fprintf(fid, '\n');
      end
      fprintf(fid, '\n');
    end
  end
end
