classdef BugOut < handle
  % Example
  % --------
  % xout = xmat.BugOut.byte();
  % xout.setitem('a0', 1i);
  % xout.close();
  %
  % xout = xmat.BugOut.file(filename_out);
  % xout.setitem('a0', 1);
  % xout.setitem('a1', 1i);
  % xout.close();
  % 

    
  properties
    obuf
    head
    is_open = true
  end
  

  methods (Static)
    function xout = file(filename)
      stream = xmat.BufFile(filename, 'w');
      xout = xmat.BugOut(stream);
    end

    function xout = byte()
      stream = xmat.BufByte('w');
      xout = xmat.BugOut(stream);
    end
  end


  methods
    function obj = BugOut(output_stream)
      obj.obuf = output_stream;
      obj.head = xmat.XHead();
      obj.head.dump(obj.obuf);
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
      % order_tag: 'c' or 'f'
      %   `f` - default. `c` - says save element order as F-order, and flip shape instead.

      if strlength(name) > xmat.XUtil.k_max_block_name_len
        error('too long block-name: max := %d, len := %d, name := %s', ...
          xmat.XUtil.k_max_block_name_len, strlength(name), name);
      end

			if nargin < 4
				ndim_force = [];
			elseif ~isnumeric(ndim_force)
				error('wrong `ndim_force`.type');
			end
			if nargin < 5 || isempty(order_tag)
				order_tag = 'f';
			elseif ~(ischar(order_tag) || isstring(order_tag))
				error('wrong `order_tag`.type');
			end

			if ~any(strcmp(order_tag, ["c", "f"]))
				error('wrong order tag: %s', order_tag);
			end

      if isstring(A)
        A = char(A);
      end

      % dump block descriptor
      bd = xmat.XBlock.make(A, name, ndim_force);
			if order_tag == 'c'
				bd.shape = flip(bd.shape);
			else
				A = permute(A, ndims(A):-1:1);
			end
      bd.dump(obj.obuf);

      % dump data
      if isreal(A)
        obj.obuf.write(A(:));
      else  % if complex
        A_ = transpose([real(A(:)), imag(A(:))]);
        obj.obuf.write(A_(:));
      end
    end

    function close(obj)
      % save header.total_size
      total_size = obj.obuf.size();
      obj.head.total_size = total_size;
      obj.obuf.seek(0, -1);
      obj.obuf.write(uint64(total_size));
      
      obj.obuf.close();
      obj.is_open = false;
    end
  end
end
