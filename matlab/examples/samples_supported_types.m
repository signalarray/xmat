function S = samples_supported_types()

S_ = struct();

% double
% ------
S_.x0 = 10 + 1./3;

S_.x1 = 10 + 1./(1:4);

shape = [3, 4];
S_.x2 = reshape(1:prod(shape), shape);

shape = [3, 4, 5];
S_.x3 = reshape(1:prod(shape), shape);

shape = [3, 4, 5, 6];
S_.x4 = reshape(1:prod(shape), shape);


% ----------
M = {
  {"f64", @double}, ...
  {"f32", @single}, ...
  {"i8", @int8}, ...
  {"i16", @int16}, ...
  {"i32", @int32}, ...
  {"i64", @int64}, ...
  {"u8", @int8}, ...
  {"u16", @int16}, ...
  {"u32", @int32}, ...
  {"u64", @int64}, ...
};


S = struct();

fields_ = fieldnames(S_);
for m = 1:length(M)
  for n = 1:length(fields_)
    name = M{m}{1} + fields_{n};
    S.(name) = M{m}{2}(S_.(fields_{n}));
  end

  if ~any(strcmp(M{m}{1}, ["f64" "f32"]))
    continue
  end

  for n = 1:length(fields_)
    name = M{m}{1} + fields_{n};
    S.("cx" + name) = S.(name) - 1j * S.(name);
  end
end

S.string_single_line = "some content in oneline string format: ~!@#$%^&*(){}[]<>+_`";
S.string_multy_line = "line1\nline2\nline3\n";

fprintf('end\n');
end
