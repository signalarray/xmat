classdef MapStreamOut < handle

  properties
    ods     % output data stream
    head
    is_open = true
    names_list = ""
  end


  methods (Static)
    function xout = file(filename)
      stream = xmat.DStreamFile(filename, 'w');
      xout = xmat.MapStreamOut(stream);
    end

    function xout = byte()
      stream = xmat.DStreamByte('w');
      xout = xmat.MapStreamOut(stream);
    end
  end


  methods
    function obj = MapStreamOut(data_stream_out)
      obj.ods = data_stream_out;
      obj.head = xmat.XHead();
      obj.head.dump(obj.ods);
    end

    function delete(obj)
      if obj.is_open
        obj.close();
      end
    end

    function setitem(obj, name, A, ndim_force, order_tag)
      % write A to ostream
      % Parameters:
      % -----------
      % name: char
      % A: value
      % ndim_force: int
      %   matlab reduce last 1th dimentions (ndims(ones(2, 2, 1) -> 2, but not 3)). `ndim_force` set dim explicitly
      % order_tag: 'C' or 'F'
      %   `F` - default. `F` - says save element order as F-order, and flip shape instead.

      % check
      % -----   

      if nargin < 4
        ndim_force = [];
      end

      if nargin < 5 || isempty(order_tag)
        order_tag = 'F';
      elseif ~xmat.XUtil.isstringlike(order_tag)
        error('wrong `order_tag`.type');
      end

      if strlength(name) > obj.head.maxname
        error('xmat.MapStreamOut.setitem(): too long block-name: max := %d, len := %d, name := %s', ...
          xmat.head.maxname, strlength(name), name);
      end

      if any(name == obj.names_list)
        warning("name is already in: %s", name);
      end
      obj.names_list(end + 1) = name;

      if ~any(strcmp(order_tag, ["C", "F"]))
        error('wrong order tag: %s', order_tag);
      end

      % dump
      % ----
      bd = xmat.XBlock().make(A, name, ndim_force);
			if order_tag == 'C'
				bd.shape = flip(bd.shape);
			else
				A = permute(A, ndims(A):-1:1);
			end
      bd.dump(obj.ods);

      % dump data
      obj.ods.write(A(:));
    end

    function close(obj)
      % save header.total_size
      total_size = obj.ods.size();
      obj.head.total = total_size;
      obj.ods.seek(6, -1);
      obj.ods.write(xmat.DataType.k_xsize_t(total_size));

      obj.ods.close();
      obj.is_open = false;
    end
  end
end

