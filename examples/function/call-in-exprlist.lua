-- Define functions
local foo = function () return 3, 5, 7 end
local bar = function (a, b) return a+b end

-- Call functions
--a = foo()
--a = bar(foo())
--a, b, c = 9, foo()
--a, b, c = foo()
--local a, b = foo(), 9
local a = bar(foo(), foo())
