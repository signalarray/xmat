function samples_all()
clc
clear
close all
fclose('all');


[folder_script, filename_script, ~] = fileparts(mfilename('fullpath'));
fprintf('%s\n', folder_script);
fprintf('%s\n', filename_script);

folder_data = fullfile(folder_script, '..', '..', 'data');

sample_4();


% group samples
% -------
  function sample_0()
    fprintf('sample_0: xmat.DataTypeReg \n');

  end


  function sample_1()
    fprintf('sample_0: xmat.DStreamByte\n');
    
    ods = xmat.DStreamByte.out();
    ods.endian = xmat.Endian.little;
    ods.write([1, 1i])
    reshape(ods.buf(), 8, []).'

    ids = xmat.DStreamByte.in();
    ids.set_buffer(ods.buf());
    ids.read(2, 'cx_double')
  end


  function sample_2()
    fprintf('sample_1: xmat.XHead: dump/load \n');
    filename_out = fullfile(folder_data, 'sample_2.xmat');

    ods = xmat.DStreamByte.out();
    ods.endian = xmat.Endian.native;

    h = xmat.XHead();
    h.print();
    h.dump(ods);

    ods.print(8)

    fprintf('XHead.load\n');
    ids = xmat.DStreamByte.in();
    ids.set_buffer(ods.buf());
    h2 = xmat.XHead();
    h2.load(ids);
    h2.print();
  end


  function sample_3()
    fprintf('sample_1: xmat.XBlock: dump/load \n');
    filename_out = fullfile(folder_data, 'sample_3.xmat');

    ods = xmat.DStreamByte.out();
    ods.endian = xmat.Endian.native;

    fprintf('xmat.XBlock: dump\n');
    b0 = xmat.XBlock().make(1j, 'nameforblock');
    b0.print()

    b0.dump(ods);
    ods.print(8)
    
    fprintf('xmat.XBlock: load\n');
    ids = xmat.DStreamByte.in();
    ids.set_buffer(ods.buf());

    b1 = xmat.XBlock().load(ids);
    b1.print()
  end

  
  function sample_4()
    fprintf('sample_4: xmat.MapStreamOut/In: \n');
    filename_out = fullfile(folder_data, 'sample_4.xmat');

    X = reshape(1:6, 3, 2);
    Xi = X + 1i;
    disp(X)
    disp(Xi)

    xout = xmat.MapStreamOut.byte(xmat.Endian.big);
    xout.setitem('a00', X);
    xout.setitem('a01', Xi);
    xout.morder = 'C';
    xout.setitem('a10', X);
    xout.flipshape = true;
    xout.setitem('a11', X);
    xout.close();
    xout.ods.print(8)

    xin = xmat.MapStreamIn.byte(xout.ods.buf);
    xin.print()
    return

    y00 = xin.getitem('a00')
    xmat.XBlock().make(y00).print()

    y01 = xin.getitem('a01')
    xmat.XBlock().make(y01).print()

    fprintf('non-flipshape\n');
    y10 = xin.getitem('a10')
    xmat.XBlock().make(y10).print()

    y11 = xin.getitem('a11')
    xmat.XBlock().make(y11).print()
    
    fprintf('allow flipshape\n');
    xin.flipshape = true;
    y10 = xin.getitem('a10')
    xmat.XBlock().make(y10).print()

    y11 = xin.getitem('a11')
    xmat.XBlock().make(y11).print()
  end


  function sample_5()
    fprintf('sample_5: xmat.MapStreamOut/In.file(): \n');
    filename_out = fullfile(folder_data, 'sample_5.xmat');

    X = reshape(1:6, 3, 2);
    Xi = X + 1i;
    disp(X)
    disp(Xi)

    xout = xmat.MapStreamOut.file(filename_out, xmat.Endian.big);
    xout.setitem('a00', X);
    xout.setitem('a01', Xi);
    xout.morder = 'C';
    xout.setitem('a10', X);
    xout.flipshape = true;
    xout.setitem('a11', X);
    xout.close();
    %! xout.ods.print(8)

    xin = xmat.MapStreamIn.file(filename_out);
    fprintf('xin.endian: %s\n', xin.endian);
    xin.print()
    return
    y00 = xin.getitem('a00')
    xmat.XBlock().make(y00).print()

    y01 = xin.getitem('a01')
    xmat.XBlock().make(y01).print()

    fprintf('non-flipshape\n');
    y10 = xin.getitem('a10')
    xmat.XBlock().make(y10).print()

    y11 = xin.getitem('a11')
    xmat.XBlock().make(y11).print()
    
    fprintf('allow flipshape\n');
    xin.flipshape = true;
    y10 = xin.getitem('a10')
    xmat.XBlock().make(y10).print()

    y11 = xin.getitem('a11')
    xmat.XBlock().make(y11).print()
  end

% end samples
% -----------
end



