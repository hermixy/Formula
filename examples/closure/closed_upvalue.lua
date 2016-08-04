-- the following example is from 'The Implementation of Lua 5.0'
-- the upvalue 'a' is not exisited in stack slot when calling add2 or add3

local function add (x)
	return function(y) return x+y end
end

local add2 = add(2)
local a = add2(6)

local add3 = add(3)
local b = add3(7)
