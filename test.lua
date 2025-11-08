--[[
player2 = {}
player2["Name"] = "Sonya"
player2["age"] = "222"

player = { Name = 34,
           table = { a = "stra", b = "strb"},
           Level = 20 }

table = { t1 = "34", t2 = "hello world", t3 = true }
a = 3 + 4
]]


a = vec3(0, 0, 3)
b = vec3(4, 5, 6)


c = a + b

q1 = quat(1, 2, 3, 4)
q2 = quat(0.04, b)

print(normalize(q1))
print(inverse(normalize(q1)))
