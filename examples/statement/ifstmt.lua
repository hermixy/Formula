local a, b, c = -4, 5, 0

if a > 0 then
	c = a + b
else 
	if b < 0 then
		c = a - b
	else 
		c = a * b
	end
end
