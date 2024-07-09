file_out = fullfile(temp_data_folder(), 'example_matlab.xmat');

xin = xmat.MapStreamIn.file(file_out);
xin.print()

a = xin.getitem('a')
b = xin.getitem('b')
c = xin.getitem('c')

xin.close()
