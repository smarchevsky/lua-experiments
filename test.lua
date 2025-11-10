--a = vec3(0, 0, 3)
--b = vec3(4, 5, 6)
--c = a + b
--q1 = quat(3, vec3(3,3,3))
--print(q1)

function printTable(t)
    for i, v in ipairs(t) do io.write(v, " ") end
    print()
end

a = Arr2d({
    {1, 2, 3},
    {6, 5, 6},
    {7, 8, 9}
})

row = a:binarySearchByCol(1,7)


printTable(row)
--print(a:getRow(1))


--rr2d:set(1, 2, 42)

--print("Value:", Arr2d:get(1, 2))
--print(Arr2d:get(1, 2))

--print(roadSplineLength())

--function processVecs(a, b)
--    -- Example: return dot and distance
--    local dot = a.x * b.x + a.y * b.y + a.z * b.z
--    local dx = a.x - b.x
--    local dy = a.y - b.y
--    local dz = a.z - b.z
--    local dist = math.sqrt(dx*dx + dy*dy + dz*dz)
--    return dot, dist
--end

