function matlab_samples()
clc
clear
close all
fclose('all');


[folder_script, filename_script, ~] = fileparts(mfilename('fullpath'));
fprintf('%s\n', folder_script);
fprintf('%s\n', filename_script);

folder_data = fullfile(folder_script, '..', 'data');

sample_2();


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
    fprintf('sample_2: test xmat.StreamFile\n');

    os = xmat.StreamBytes('w');
    
    h = xmat.Header();
    bd = xmat.Block('blockname', 'typename', 4, 2, [2, 2])
    bd.write(os, h);

    is = xmat.StreamBytes('r', os.buff)
    h2 = xmat.Block.read(is, h)
  end

  
  function sample_3()
    fprintf('sample_3: test xmat.Save');

    xout = xmat.Save.bytes();
    xout.add('field_0', 1:10);
    % xout.add('field_1', randi(8, [3, 5]));
    % xout.add('field_2', randi(8, [2, 3]), 'uint8');
    xout.close();
  end
end






