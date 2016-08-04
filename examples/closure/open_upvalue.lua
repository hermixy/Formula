-- calling foo, and then bar, the upvalue b is 
-- in stack slot untill foo is returned
local function foo()
	local a, b, c = 1, 2, 3
	local function bar()
		b = 10
		return b
	end

	return bar()
end

local a = foo()
