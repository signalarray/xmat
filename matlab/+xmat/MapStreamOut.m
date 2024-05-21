classdef MapStreamOut < handle
  % Properties:
  % -----------
  % morder: 'C' or 'F', optional(default := obj.morder)
  %   major element order
  %   `F` - default. `F` - says save element order as F-order, and flip shape instead.
  %
  % flipshape: logical, optional(default := false)
  %   if `morder` ~= 'F' && flipshape == true, it will not permute
  %   elemente, but flip size(A) instread.
  %
  % Desctiption:
  % ------------
  %      \flipshape:  true          false
  % morder---------:--------------------------
  %             'C':  flip(shape)   permute(A)
  %             'F':  nothing       nothing

  properties
    ods     % output data stream
    head
    is_open = true
    names_list = ""
    
    % format options
    morder = xmat.DataType.k_morder
    flipshape = false
  end

  properties (Dependent)
    endian
  end

  methods (Static)
    function xout = file(filename, endian)
      if nargin < 2
        endian = xmat.Endian.native;
      end
      stream = xmat.DStreamFile(filename, 'w');
      stream.endian = endian;
      xout = xmat.MapStreamOut(stream);
    end

    function xout = byte(endian)
      if nargin < 1
        endian = xmat.Endian.native;
      end
      stream = xmat.DStreamByte('w');
      stream.endian = endian;
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

    function e = get.endian(obj)
      e = obj.ods.endian;
    end

    function set.endian(obj, endian)
      if ~any(endian == 'lb')
        errror("xmat.MapStreamOut.set.endian: wrong `endian` value");
      end
      obj.ods.endian = endian;
    end

    function setitem(obj, name, A, ndim_force)
      % Parameters:
      % -----------
      % name: string-like
      %
      % A: value
      %
      % ndim_force: int, optional(default := [])
      %   matlab reduce last 1th dimentions (ndims(ones(2, 2, 1) -> 2, but not 3)).
      %   `ndim_force` set dim explicitly

      if nargin < 4
        ndim_force = [];
      end

      if strlength(name) > obj.head.maxname
        error('xmat.MapStreamOut.setitem(): too long block-name: max := %d, len := %d, name := %s', ...
          xmat.head.maxname, strlength(name), name);
      end
      
      % ----
      if any(name == obj.names_list)
        warning("name is already in: %s", name);
      end
      obj.names_list(end + 1) = name;

      if ~any(strcmp(obj.morder, ["C", "F"]))
        error('wrong order tag: %s', obj.morder);
      end

      % dump 1d
      bd = xmat.XBlock().make(A, name, ndim_force);
      bd.morder = obj.morder;

      % process morder and shape
			if bd.morder == 'C'
        if obj.flipshape
				  bd.shape = flip(bd.shape);
        else
				  A = permute(A, ndims(A):-1:1);
        end
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

