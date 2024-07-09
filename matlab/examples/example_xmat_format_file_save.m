file_out = fullfile(temp_data_folder(), 'example_matlab.xmat');

xout = xmat.MapStreamOut.file(file_out);

% floating-point numbers
xout.setitem('a', 3.14);
xout.setitem('b', linspace(-1, 1, 7));
xout.setitem('c', reshape(1:2*3, [2 3]));
xout.setitem('d', reshape(1:2*3*4, [2 3 4]));
xout.setitem('e', reshape(1:3^5, [3 3 3 3 3]));

% integer numbers
xout.setitem('f', randi([0 255], 2, 3, 'uint8'));
xout.setitem('g', randi([-128 127], 2, 3, 'int8'));

% complex-value numbers
xout.setitem('h', exp(1i*2*pi*3*linspace(0, 1, 32)));
xout.setitem('i', exp(1i*2*pi*[2, 3, 4].*linspace(0, 1, 32)'));

% ascii-string
xout.setitem('j', 'string value from matlab');

% name of a data-block could be any ascii-string with less than 32 length
xout.setitem('longer-name', 1);
xout.setitem('maximun_allowed_block_name_is_32', 1);
xout.setitem('name_can_contain_any_ascii:@<>[]', 2);

xout.close();
