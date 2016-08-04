-- transitivity of upvalues:
-- supposing that function func1 encloses function func2 
-- and function func2 encloses function func3 either directly or indirectly
-- if symbol 'a' is a local symbol of func1 then an upvalue of func3,
-- then symbol 'a' is also an upvalue of func2.

local function func1()
	local a, b, c = 1, 2, 3

	local function func2()
		b = 10

		local function func3()
			c = a + b
			return c
		end

		return func3()
	end

	return func2(), a, b, c
end

local r1, r2, r3, r4 = func1()
print(r1, r2, r3, r4)
