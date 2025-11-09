--a = vec3(0, 0, 3)
--b = vec3(4, 5, 6)
--c = a + b
--q1 = quat(3, vec3(3,3,3))
--print(q1)




function processVecs(a, b)
    -- Example: return dot and distance
    local dot = a.x * b.x + a.y * b.y + a.z * b.z
    local dx = a.x - b.x
    local dy = a.y - b.y
    local dz = a.z - b.z
    local dist = math.sqrt(dx*dx + dy*dy + dz*dz)
    return dot, dist
end

