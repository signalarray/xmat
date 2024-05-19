classdef MapStreamIn < handle
    
  properties
    ids
    head
    is_open

    map

    % format options
    flipshape = false
  end
  

  methods (Static)
    function xin = file(filename)
      ids = xmat.DStreamFile.in(filename);
      xin = xmat.MapStreamIn(ids, true);
    end

    function xin = byte(buf)
      % Parameters:
      %   buf: uint8<1d>, optional
      ids = xmat.DStreamByte.in();
      xin = xmat.MapStreamIn(ids, false);
      if nargin < 1
        return
      end
      xin.push_buffer(buf);
      xin.scan();
    end
  end


  methods
    function obj = MapStreamIn(data_stream_in, ready_for_scan)
      % ready_for_scan: logical(,true)
      %   used for deffered bytebufer initialization.
      if nargin < 2
        ready_for_scan = true;
      end

      obj.ids = data_stream_in;
      obj.head = xmat.XHead();
      obj.map = containers.Map();

      if (ready_for_scan)
        obj.scan();
      end
    end

    function delete(obj)
      if obj.is_open
        obj.close();
      end
    end

    function obj = push_buffer(obj, buf)
      obj.ids.push_buffer(buf);
    end

    function A = getitem(obj, name)
      if ~xmat.XUtil.isstringlike(name)
        error('xmat.MapStreamIn.getitem(): wrong `name` type: %s, string-like expected', class(name));
      end
      
      if ~isKey(obj.map, name)
        A = [];
        return;
      end

      % load data block
      % -----------------
      bd = obj.map(name);
      obj.ids.seek(bd.data_pos(), -1);
      A = obj.ids.read(bd.numel(), bd.tid);

      % process morder and shape
      if bd.morder == 'C'
        if obj.flipshape
          bd.shape = flip(bd.shape);
          A = reshape(A, bd.shape);
        else
          A = reshape(A, flip(bd.shape));
          A = permute(A, length(bd.shape):-1:1);
        end
      elseif bd.morder == 'F'
        A = reshape(A, bd.shape);
      else
        error("");
      end
    end

    % content access
    % -------------------------------
    function bd = getblock(obj, name)
      bd = obj.map(name);
    end

    function flag = has(field_name)
      flag = isfield(obj.map, field_name);
    end

    function scan(obj)
      obj.scan_header();
      obj.scan_data();
    end

    function scan_header(obj)
      obj.head = obj.head.load(obj.ids);
    end

    function scan_data(obj)
      while obj.ids.tell() < obj.head.total
        bd_ = xmat.XBlock().load(obj.ids);
        obj.map(bd_.name) = bd_;
        
        % advance buffer cursor for data block size
        status = obj.ids.seek(bd_.data_nbytes(), 0);
        assert(status == 0);
      end
      if obj.ids.tell() ~= obj.head.total
        error('xmat.MapStreamIn.scan_data(): failed');
      end
    end

    function close(obj)
      obj.ids.close();
      obj.is_open = false;
    end

    function print(obj, fid)
      if nargin < 2
        fid = 1;
      end
      fprintf('xmat.MapStreamIn:\n');
      obj.head.print(fid);
      fields = keys(obj.map);
      span = 0;
      for n = 1:length(fields)
        span = max(length(fields{n}), span);
      end
      args.span = span + 2;
      args.end_ = '\n';
      fprintf('blocks:\n');
      for n = 1:length(fields)
        fprintf(fid, '[%4.d]', n);
        bd_ = obj.map(fields{n});
        bd_.print(fid, args);
      end
      fprintf(fid, '\n');
    end
  end
end
