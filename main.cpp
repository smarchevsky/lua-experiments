#include <cstring>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

std::string filePath(PROJECT_DIR "/test.lua");

#if 0
#define LOG_LUA_WRAPPER(str) printf("%s\n", str);
#else
#define LOG_LUA_WRAPPER(str)
#endif

namespace {
class LuaGlobal {
    lua_State* const L;
    bool success;

public:
    operator bool() { return success; }
    LuaGlobal(lua_State* ls, const char* str)
        : L(ls)
    {
        LOG_LUA_WRAPPER("LuaGlobal()");
        success = lua_getglobal(L, str);
    }

    ~LuaGlobal()
    {
        lua_pop(L, 1);
        LOG_LUA_WRAPPER("~LuaGlobal()");
    }
};

class LuaField {
    lua_State* const L;
    bool success;

public:
    operator bool() { return success; }
    LuaField(lua_State* ls, const char* str)
        : L(ls)
    {
        LOG_LUA_WRAPPER("LuaField()");
        success = lua_getfield(L, -1, str);
    }

    ~LuaField()
    {
        lua_pop(L, 1);
        LOG_LUA_WRAPPER("~LuaField()");
    }
};

class LuaString {
    lua_State* const L;
    bool success = false;

public:
    operator bool() { return success; }
    LuaString(lua_State* ls, std::string& buf)
        : L(ls)
    {
        LOG_LUA_WRAPPER("LuaString()");
        size_t len;
        if (lua_isstring(L, -1)) {
            const char* ptr = lua_tolstring(L, -1, &len);
            buf.assign(ptr, len);
            success = true;
        }
    }

    ~LuaString()
    {
        LOG_LUA_WRAPPER("~LuaString()");
    }
};

class LuaNumber {
    lua_State* const L;
    bool success = false;

public:
    operator bool() { return success; }
    LuaNumber(lua_State* ls, double& f)
        : L(ls)
    {
        LOG_LUA_WRAPPER("LuaNumber()");
        size_t len;
        if (lua_isnumber(L, -1)) {
            f = lua_tonumber(L, -1);
            success = true;
        }
    }

    ~LuaNumber()
    {
        LOG_LUA_WRAPPER("~LuaNumber()");
    }
};

#define LUA_GET_INPUT(type, name, index) type* name = (type*)luaL_checkudata(L, index, #type "Meta")
#define LUA_GET_FLOAT(name, index) Float name = (Float)luaL_checknumber(L, index)
#define LUA_GET_OUTPUT(type) type* outptr = (type*)lua_newuserdata(L, sizeof(type))

typedef float Float;

struct vec3 : public glm::vec<3, Float, glm::defaultp> {
    vec3() { }
    vec3(Float a) { x = y = z = a; }
    vec3(Float _x, Float _y, Float _z) { x = _x, y = _y, z = _z; }
    vec3(const glm::vec<3, Float, glm::defaultp>& v) { x = v.x, y = v.y, z = v.z; }
    vec3(const vec3& rhs) = default;
};

struct quat : public glm::qua<Float, glm::defaultp> {
    quat() { }
    quat(Float _x, Float _y, Float _z, Float _w) { x = _x, y = _y, z = _z, w = _w; }
    quat(const glm::qua<Float, glm::defaultp>& q) { x = q.x, y = q.y, z = q.z, w = q.w; }
};

//
// VEC3
//

int lua_newVec3(lua_State* L)
{
    LUA_GET_FLOAT(x, 1);
    LUA_GET_FLOAT(y, 2);
    LUA_GET_FLOAT(z, 3);
    LUA_GET_OUTPUT(vec3);

    *outptr = vec3(x, y, z);

    luaL_getmetatable(L, "vec3Meta");
    lua_setmetatable(L, -2);
    return 1;
}

int lua_vec3_index_getter(lua_State* L)
{
    LUA_GET_INPUT(vec3, v, 1);
    const char* key = luaL_checkstring(L, 2);
    if (strcmp(key, "x") == 0)
        lua_pushnumber(L, v->x);
    else if (strcmp(key, "y") == 0)
        lua_pushnumber(L, v->y);
    else if (strcmp(key, "z") == 0)
        lua_pushnumber(L, v->z);
    else {
        lua_pushnil(L);
    }
    return 1;
}

int lua_vec3_index_setter(lua_State* L)
{
    LUA_GET_INPUT(vec3, v, 1);
    const char* key = luaL_checkstring(L, 2);
    LUA_GET_FLOAT(value, 3);
    if (strcmp(key, "x") == 0)
        v->x = value;
    else if (strcmp(key, "y") == 0)
        v->y = value;
    else if (strcmp(key, "z") == 0)
        v->z = value;
    return 0;
}

int lua_vec3_cross(lua_State* L)
{
    LUA_GET_INPUT(vec3, a, 1);
    LUA_GET_INPUT(vec3, b, 2);
    LUA_GET_OUTPUT(vec3);

    *outptr = glm::cross(*a, *b);

    luaL_getmetatable(L, "vec3Meta");
    lua_setmetatable(L, -2);
    return 1;
}

int lua_vec3_dot(lua_State* L)
{
    LUA_GET_INPUT(vec3, a, 1);
    LUA_GET_INPUT(vec3, b, 2);

    Float dot = a->x * b->x + a->y * b->y + a->z * b->z;

    lua_pushnumber(L, dot);
    return 1;
}

typedef vec3 (*BinaryFuncVec3Vec3)(const vec3&, const vec3&);
int lua_BinaryFuncVec3Vec3(lua_State* L, BinaryFuncVec3Vec3 func)
{
    vec3 result;

    if (luaL_testudata(L, 1, "vec3Meta") && luaL_testudata(L, 2, "vec3Meta")) {
        LUA_GET_INPUT(vec3, v1, 1);
        LUA_GET_INPUT(vec3, v2, 2);
        result = func(*v1, *v2);
    }

    else if (luaL_testudata(L, 1, "vec3Meta") && lua_isnumber(L, 2)) {
        LUA_GET_INPUT(vec3, v, 1);
        Float f = (Float)lua_tonumber(L, 2);
        result = func(*v, f);
    }

    else if (lua_isnumber(L, 1) && luaL_testudata(L, 2, "vec3Meta")) {
        Float f = (Float)lua_tonumber(L, 1);
        LUA_GET_INPUT(vec3, v, 2);
        result = func(f, *v);

    } else {
        return luaL_error(L, "Invalid operands for vec3 operations");
    }

    LUA_GET_OUTPUT(vec3);
    *outptr = result;

    luaL_getmetatable(L, "vec3Meta");
    lua_setmetatable(L, -2);
    return 1;
}

// clang-format off
int lua_vec3_add(lua_State* L) { return lua_BinaryFuncVec3Vec3(L, [](const vec3& a, const vec3& b) -> vec3 { return a + b; }); }
int lua_vec3_sub(lua_State* L) { return lua_BinaryFuncVec3Vec3(L, [](const vec3& a, const vec3& b) -> vec3 { return a - b; }); }
int lua_vec3_mul(lua_State* L) { return lua_BinaryFuncVec3Vec3(L, [](const vec3& a, const vec3& b) -> vec3 { return a * b; }); }
int lua_vec3_div(lua_State* L) { return lua_BinaryFuncVec3Vec3(L, [](const vec3& a, const vec3& b) -> vec3 { return a / b; }); }
// clang-format on

int lua_vec3_tostring(lua_State* L)
{
    LUA_GET_INPUT(vec3, v, 1);
    lua_pushfstring(L, "vec3(%f, %f, %f)", (double)v->x, (double)v->y, (double)v->z);
    return 1;
}

//
// QUAT
//

int lua_new_quat(lua_State* L)
{
    quat result;

    if (lua_isnumber(L, 1) && luaL_testudata(L, 2, "vec3Meta")) {
        LUA_GET_FLOAT(y, 1);
        LUA_GET_INPUT(vec3, v, 2);
        result = glm::angleAxis(y, glm::normalize(*v));

    } else {
        LUA_GET_FLOAT(x, 1);
        LUA_GET_FLOAT(y, 2);
        LUA_GET_FLOAT(z, 3);
        LUA_GET_FLOAT(w, 4);
        result = glm::normalize(quat(x, y, z, w));
    }

    LUA_GET_OUTPUT(quat);
    *outptr = result;

    luaL_getmetatable(L, "quatMeta");
    lua_setmetatable(L, -2);
    return 1;
}

int lua_normalize(lua_State* L)
{
    if (luaL_testudata(L, 1, "vec3Meta")) {
        LUA_GET_INPUT(vec3, v, 1);
        LUA_GET_OUTPUT(vec3);
        *outptr = glm::normalize(*v);
        luaL_getmetatable(L, "vec3Meta");

    } else if (luaL_testudata(L, 1, "quatMeta")) {
        LUA_GET_INPUT(quat, q, 1);
        LUA_GET_OUTPUT(quat);
        *outptr = glm::normalize(*q);
        luaL_getmetatable(L, "quatMeta");

    } else {
        return luaL_error(L, "Correct operands for normalize: quat, vec3");
    }

    lua_setmetatable(L, -2);
    return 1;
}

int lua_inverse(lua_State* L)
{
    LUA_GET_INPUT(quat, v, 1);
    LUA_GET_OUTPUT(quat);
    *outptr = glm::inverse(*v);

    luaL_getmetatable(L, "quatMeta");
    lua_setmetatable(L, -2);
    return 1;
}

int lua_quat_mul(lua_State* L)
{
    const char* errorMsg = "Correct operands: (quat = quat * quat) or (vec = quat * vec)";

    if (luaL_testudata(L, 1, "quatMeta")) {
        LUA_GET_INPUT(quat, q, 1);

        if (luaL_testudata(L, 2, "vec3Meta")) { // vec = quat * vec
            LUA_GET_INPUT(vec3, v, 2);
            LUA_GET_OUTPUT(vec3);
            *outptr = *q * *v;
            luaL_getmetatable(L, "vec3Meta");
            errorMsg = nullptr;

        } else if (luaL_testudata(L, 2, "quatMeta")) { // quat = quat * quat
            LUA_GET_INPUT(quat, q2, 2);
            LUA_GET_OUTPUT(quat);
            *outptr = glm::normalize(*q * *q2);
            luaL_getmetatable(L, "quatMeta");
            errorMsg = nullptr;
        }
    }

    if (errorMsg)
        return luaL_error(L, errorMsg);

    lua_setmetatable(L, -2);
    return 1;
}

int lua_quat_tostring(lua_State* L)
{
    LUA_GET_INPUT(quat, q, 1);
    lua_pushfstring(L, "quat(%f, %f, %f, %f)", (double)q->x, (double)q->y, (double)q->z, (double)q->w);
    return 1;
}

void registerMathFunctions(lua_State* L)
{
    { // VEC3
        luaL_newmetatable(L, "vec3Meta");

        lua_pushcfunction(L, lua_vec3_add);
        lua_setfield(L, -2, "__add");
        lua_pushcfunction(L, lua_vec3_sub);
        lua_setfield(L, -2, "__sub");
        lua_pushcfunction(L, lua_vec3_mul);
        lua_setfield(L, -2, "__mul");
        lua_pushcfunction(L, lua_vec3_div);
        lua_setfield(L, -2, "__div");
        lua_pushcfunction(L, lua_vec3_tostring);
        lua_setfield(L, -2, "__tostring");

        // SUBSCRIPT
        lua_pushcfunction(L, lua_vec3_index_getter);
        lua_setfield(L, -2, "__index");
        lua_pushcfunction(L, lua_vec3_index_setter);
        lua_setfield(L, -2, "__newindex");

        lua_pop(L, 1);
    }

    { // QUAT
        luaL_newmetatable(L, "quatMeta");
        lua_pushcfunction(L, lua_quat_mul);
        lua_setfield(L, -2, "__mul");
        lua_pushcfunction(L, lua_quat_tostring);
        lua_setfield(L, -2, "__tostring");
        lua_pop(L, 1);
    }

    // GENERAL FUNCTIONS VEC3
    lua_register(L, "vec3", lua_newVec3);
    lua_register(L, "dot", lua_vec3_dot);
    lua_register(L, "cross", lua_vec3_cross);

    // GENERAL FUNCTIONS QUAT
    lua_register(L, "quat", lua_new_quat);
    // lua_register(L, "quatPitchYawRoll", lua_quatPitchYawRoll);
    lua_register(L, "normalize", lua_normalize);
    lua_register(L, "inverse", lua_inverse);
}
} // namespace

int main()
{
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    registerMathFunctions(L);

    if (L == NULL) {
        std::cerr << "Failed to create Lua state." << std::endl;
        return 1;
    }

    std::string stringBuf;
    if (luaL_dofile(L, filePath.c_str()) == LUA_OK) {

        if (auto f = LuaGlobal(L, "player")) {
            if (auto f = LuaField(L, "table")) {
                if (auto f = LuaField(L, "a")) {
                    if (auto f = LuaString(L, stringBuf)) {
                        printf("%s\n", stringBuf.c_str());
                    }
                }
            }
        }
        if (auto f = LuaGlobal(L, "a")) {
            if (auto f = LuaString(L, stringBuf)) {
                printf("%s\n", stringBuf.c_str());
            }
        }

    } else
        std::cerr << "Lua error: " << lua_tostring(L, -1) << std::endl;

    lua_close(L);

    return 0;
}
