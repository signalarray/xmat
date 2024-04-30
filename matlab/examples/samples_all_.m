function samples_all_()
clc
clear
close all
fclose('all');


[folder_script, filename_script, ~] = fileparts(mfilename('fullpath'));
fprintf('%s\n', folder_script);
fprintf('%s\n', filename_script);

folder_data = fullfile(folder_script, '..', '..', 'data');

sample_3();


% group samples
% -------
  function sample_0()
    fprintf('sample_0: xmat.XHead: dump/load ');
    filename_out = fullfile(folder_data, 'sample_0.xmat');

    % BufByte
    h00 = xmat.XHead();
    obbuf = xmat.BufByte.out();
    h00.dump(obbuf);
    obbuf.close();

    ibbuf = xmat.BufByte.in();
    ibbuf.set_buffer(obbuf.buff);
    h01 = xmat.XHead();
    h01.load(ibbuf)

    % BufFile
    h10 = xmat.XHead();
    ofbuf = xmat.BufFile.out(filename_out);
    h10.dump(ofbuf);
    ofbuf.close();

    ifbuf = xmat.BufFile.in(filename_out);
    h11 = xmat.XHead();
    h11.load(ifbuf)
  end


  function sample_1()
    fprintf('sample_1: xmat.XBlock: make');
    filename_out = fullfile(folder_data, 'sample_1.xmat');

    % XBlock.make()
    bd0 = xmat.XBlock.make(1, 'a0')
    bd1 = xmat.XBlock.make(ones(3, 4), 'a1')
    bd2 = xmat.XBlock.make(ones(3, 4, 1, 1), 'a2')
    bd3 = xmat.XBlock.make(ones(3, 4, 1, 1), 'a3', 4)

    bd4 = xmat.XBlock.make('char', 's0')

  end

  function sample_2()
    fprintf('sample_3: xmat.XBlock: dump/load ');
    filename_out = fullfile(folder_data, 'sample_2.xmat');

    bd0 = xmat.XBlock.make(1, 'a0')

    % dump / load
    % -----------
    % BufByte
    bd_ = bd0;
    obbuf = xmat.BufByte.out();
    bd_.dump(obbuf);
    obbuf.close();

    ibbuf = xmat.BufByte.in();
    ibbuf.set_buffer(obbuf.buff);
    bd0 = xmat.XBlock();
    bd0.load(ibbuf)

    % BufFile
    ofbuf = xmat.BufFile.out(filename_out);
    bd_.dump(ofbuf);
    ofbuf.close();

    ifbuf = xmat.BufFile.in(filename_out);
    b1 = xmat.XBlock();
    b1.load(ifbuf)
  end

  function sample_3()
    fprintf('sample_3: xmat.BugOut, xmat.BugIn\n');
    filename_out = fullfile(folder_data, 'sample_3.xmat');

    % BufByte
    % ------
    fprintf("\nBufByte\n===================================\n");
    % BugOut
    xout = xmat.BugOut.byte();
    xout.setitem('a0', 1i);
    xout.setitem('ab0', [1 2]);
    xout.setitem('abc0', 1);
    xout.setitem('long-name-field', 1);
    xout.setitem('long-name-field------------', 1);
    xout.close();

    % BugIn
    xin = xmat.BugIn.byte(xout.obuf.buf);
    xin.print()

    % BufFile
    % ------
    fprintf("\nBufFile\n===================================\n");
    % BugOut
    xout = xmat.BugOut.file(filename_out);
    xout.setitem('a0', 1);
    xout.setitem('a1', 1i);
    xout.close();

    % BugIn
    xin = xmat.BugIn.file(filename_out);
    xin.print()
  end

% end samples
% -----------
end
