function example_3_file(mode, folder)
% Parameters:
% -----------
% mode: str
%   {"read", "write"}
% folder: str
%   { "" OR "matlab": default, "cpp", "python" }
%
% Examples:
% example_3_file("write")
% example_3_file("read")
% example_3_file("read", "")
% example_3_file("read", "matlab")
% example_3_file("read", "cpp")
% example_3_file("read", "python")

clc
% clear
close all
fclose('all');

if nargin < 1
  mode = "write";
end

if nargin < 2 || isempty(folder)
  folder = "matlab";
end

if ~any(strcmp(folder, ["matlab", "cpp", "python"]))
  error("wrong parameter `folder`: %s", folder);
end

[folder_script, filename_script, ~] = fileparts(mfilename("fullpath"));
fprintf('%s\n', folder_script);
fprintf('%s\n', filename_script);

folder_data = fullfile(temp_data_folder(), folder);
fprintf('output folder: %s\n', folder_data);
if ~isfolder(folder_data)
  fprintf('temporary dir for data was created\n');
  mkdir(folder_data)
end


% main
% ---------------
if mode == "read" 
  read();

elseif mode == "write"
  write();

else
  error("wrong mode")
end


% subfunctions
% ============
function write()
file_data = fullfile(folder_data, "xout.xmat");

S = samples_supported_types();
fields = fieldnames(S);

xout = xmat.Output.from_file(file_data);
for n = 1:length(fields)
  xout.setitem(fields{n}, S.(fields{n}))
end

xout.close()
end


function read()
file_data = fullfile(folder_data, "xout.xmat");
xin = xmat.Input.from_file(file_data);

fields = fieldnames(xin.map);
for n = 1:length(fields)
  s_ = xin.getitem(fields{n});
  fprintf('%s<%s>::\n---------\n', fields{n}, class(s_))
  if ndims(s_) < 4
    disp(s_)
  else
    fprintf('shape: %s\n', mat2str(size(s_)));
  end
  fprintf("\n");
end
end

end
