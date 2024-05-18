function out = byteorder()
x = uint16(1);
x = typecast(x, 'uint8');
if x(1) == 1
  out = 'l';
else
  out = 'b';
end
end
