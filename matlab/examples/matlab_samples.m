function matlab_samples()
clc
clear
close all
fclose('all');


[folder_script, filename_script, ~] = fileparts(mfilename('fullpath'));
fprintf('%s\n', folder_script);
fprintf('%s\n', filename_script);

folder_data = fullfile(folder_script, '..', 'temp');

sample_3();


  function sample_0()
    fprintf('sample_0\n');
    filename_out = fullfile(folder_data, 'sample_0.xmat');

    os = xmat.StreamFile(filename_out, 'w');
    
    h = xmat.Header()
    h.write(os);
    os.close();

    is = xmat.StreamFile(filename_out, 'r')
    h2 = xmat.Header.read(is)
    is.close();
  end


  function sample_1()
    fprintf('sample_1: test xmat.StreamFile\n');
    filename_out = fullfile(folder_data, 'sample_1.xmat');

    os = xmat.StreamFile(filename_out, 'w');
    
    h = xmat.Header();
    bd = xmat.Block('blockname', 'typename', 4, 2, [2, 2])
    bd.write(os, h);
    os.close();

    is = xmat.StreamFile(filename_out, 'r')
    h2 = xmat.Block.read(is, h)
    is.close();
  end


  function sample_2()
    fprintf('sample_2: test xmat.StreamBytes\n');

    os = xmat.StreamBytes('w');
    
    h = xmat.Header();
    bd = xmat.Block('blockname', 'typename', 4, 2, [2, 2])
    bd.write(os, h);

    is = xmat.StreamBytes('r', os.buff)
    h2 = xmat.Block.read(is, h)
  end

  
  function sample_3()
    fprintf('sample_3: test xmat.Save\n');
    filename_out = fullfile(folder_data, 'sample_2.xmat');
    
    mode = 'f';  % {'b', 'f'}
    if mode == 'b'
      xout = xmat.Save.bytes();
    else
      xout = xmat.Save.file(filename_out);
    end

    xout.save('field_0', uint8(1:4));
    xout.save('field_1', randi(8, [3, 5]));
    xout.save('field_2', randi(8, [2, 3], 'uint8'));
    xout.save('field_3', complex(1:8, 8:-1:1));
    xout.save('field_4', 'asdads');
    xout.close();

    if mode == 'b'
      xin = xmat.Load.bytes(xout.ostream.buff);
    else
      xin = xmat.Load.file(filename_out);
    end
    disp(xin.map)
    
    a0 = xin.load('field_0')
    a1 = xin.load('field_1')
    a2 = xin.load('field_2')
    a3 = xin.load('field_3')
    a4 = xin.load('field_4')
    
    if mode == 'b'
      % load Header and Blocks in separate way
      hbuff = xout.ostream.buff(1:xmat.Header.k_SIZEB);
      lbuff = xout.ostream.buff(xmat.Header.k_SIZEB+1:end);
      
      h = xmat.Header.read(xmat.StreamBytes('r', hbuff));
      xin2 = xmat.Load(xmat.StreamBytes('r', lbuff), h);
      disp(xin2.map)
      a0 = xin2.load('field_0')
    end
  end
end


