classdef example_tcp_callback < handle

  properties
    fig
    ax

    counter = 0
    last_call
    timeout = 2
  end


  methods (Static)
    function example()
      N = 128;
      t0 = 1;
      it = example_tcp_callback();

      xout = xmat.MapStreamOut.byte();
      xout.setitem('x', exp(1i*2*pi*(t0 + linspace(0, 1, N))) ...
                        + 0.1*complex(rand(1, N), rand(1, N)));
      xout.close();
      xin = xmat.MapStreamIn.byte(xout.ods.buf);

      it.run(xin);
      pause(1);

      it.run(xin);
    end
  end


  methods
    function obj = example_tcp_callback()
      obj.fig = figure();
      obj.ax = axes(obj.fig);
    end

    function out = isused(obj)
      if isempty(obj.last_call)
        out = false;
      else
        out = toc(obj.last_call) < obj.timeout;
      end
    end

    function exec(obj, timewait)
      pause(timewait);
      while obj.isused()
        pause(obj.timeout);
      end
    end

    function run(obj, xin)
      % Parameters:
      % -----------
      % xin: xmat.MapStreamIn
      %   with fields: x
      %

      % print something
      fprintf('message received: n:= %d \n', obj.counter);

      % update axes
      cla(obj.ax);
      hold(obj.ax, 'off');
      grid(obj.ax, 'off');

      x = xin.getitem('x');
     
      plot(obj.ax, real(x), 'b');
      hold(obj.ax, 'on');
      plot(obj.ax, imag(x), 'r');
      plot(obj.ax, abs(x), 'k');

      ylim(obj.ax, [-1.5, 1.5]);
      grid(obj.ax, 'on');


      % post update
      obj.counter = obj.counter + 1;
      obj.last_call = tic;
    end
  end
end
