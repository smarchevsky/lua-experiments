#include <cstring>
#include <iostream>

#include <glm/glm.hpp>

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

typedef float LuaFloat;

struct vec3 : public glm::vec<3, LuaFloat, glm::defaultp> {
    vec3() { }
    vec3(LuaFloat a) { x = y = z = a; }
    vec3(const glm::vec<3, LuaFloat, glm::defaultp>& v) { x = v.x, y = v.y, z = v.z; }
    vec3(LuaFloat _x, LuaFloat _y, LuaFloat _z) { x = _x, y = _y, z = _z; }
    vec3(const vec3& rhs) = default;
};

int lua_newVec3(lua_State* L)
{
    LuaFloat x = (LuaFloat)luaL_checknumber(L, 1);
    LuaFloat y = (LuaFloat)luaL_checknumber(L, 2);
    LuaFloat z = (LuaFloat)luaL_checknumber(L, 3);
    vec3* v = (vec3*)lua_newuserdata(L, sizeof(vec3));
    *v = vec3(x, y, z);
    luaL_getmetatable(L, "vec3Meta");
    lua_setmetatable(L, -2);
    return 1;
}

int lua_vec3_index_getter(lua_State* L)
{
    vec3* v;
    v = (vec3*)luaL_checkudata(L, 1, "vec3Meta");

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
    vec3* v;
    v = (vec3*)luaL_checkudata(L, 1, "vec3Meta");

    const char* key = luaL_checkstring(L, 2);
    LuaFloat value = (LuaFloat)luaL_checknumber(L, 3);

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
    vec3* a = (vec3*)luaL_checkudata(L, 1, "vec3Meta");
    vec3* b = (vec3*)luaL_checkudata(L, 2, "vec3Meta");

    vec3* result = (vec3*)lua_newuserdata(L, sizeof(vec3));

    result->x = a->y * b->z - b->y * a->z;
    result->y = a->z * b->x - b->z * a->x,
    result->z = a->x * b->y - b->x * a->y;

    luaL_getmetatable(L, "vec3Meta");
    lua_setmetatable(L, -2);
    return 1;
}

int lua_vec3_dot(lua_State* L)
{
    vec3* a = (vec3*)luaL_checkudata(L, 1, "vec3Meta");
    vec3* b = (vec3*)luaL_checkudata(L, 2, "vec3Meta");

    LuaFloat dot = a->x * b->x + a->y * b->y + a->z * b->z;
    lua_pushnumber(L, dot);
    return 1;
}

typedef vec3 (*BinaryFuncVec3Vec3)(const vec3&, const vec3&);
int lua_BinaryFuncVec3Vec3(lua_State* L, BinaryFuncVec3Vec3 func)
{
    vec3 result;

    if (luaL_testudata(L, 1, "vec3Meta") && luaL_testudata(L, 2, "vec3Meta")) {
        vec3* a = (vec3*)luaL_checkudata(L, 1, "vec3Meta");
        vec3* b = (vec3*)luaL_checkudata(L, 2, "vec3Meta");
        result = func(*a, *b);
    }

    else if (luaL_testudata(L, 1, "vec3Meta") && lua_isnumber(L, 2)) {
        vec3* a = (vec3*)luaL_checkudata(L, 1, "vec3Meta");
        LuaFloat b = (LuaFloat)lua_tonumber(L, 2);
        result = func(*a, b);
    }

    else if (lua_isnumber(L, 1) && luaL_testudata(L, 2, "vec3Meta")) {
        LuaFloat a = (LuaFloat)lua_tonumber(L, 1);
        vec3* b = (vec3*)luaL_checkudata(L, 2, "vec3Meta");
        result = func(a, *b);
    } else {
        return luaL_error(L, "Invalid operands for vec3 addition");
    }

    vec3* out = (vec3*)lua_newuserdata(L, sizeof(vec3));
    *out = result;

    luaL_getmetatable(L, "vec3Meta");
    lua_setmetatable(L, -2);
    return 1;
}
int lua_vec3_add(lua_State* L)
{
    return lua_BinaryFuncVec3Vec3(L, [](const vec3& a, const vec3& b) -> vec3 { return a + b; });
}

int lua_vec3_sub(lua_State* L)
{
    return lua_BinaryFuncVec3Vec3(L, [](const vec3& a, const vec3& b) -> vec3 { return a - b; });
}

int lua_vec3_mul(lua_State* L)
{
    return lua_BinaryFuncVec3Vec3(L, [](const vec3& a, const vec3& b) -> vec3 { return a * b; });
}

int lua_vec3_div(lua_State* L)
{
    return lua_BinaryFuncVec3Vec3(L, [](const vec3& a, const vec3& b) -> vec3 { return a / b; });
}

int lua_vec3_tostring(lua_State* L)
{
    vec3* v = (vec3*)luaL_checkudata(L, 1, "vec3Meta");
    lua_pushfstring(L, "vec3(%f, %f, %f)", v->x, v->y, v->z);
    return 1;
}

void registervec3(lua_State* L)
{
    luaL_newmetatable(L, "vec3Meta");

    // MATH

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

    lua_pop(L, 1); // pop metatable

    // GENERAL FUNCTIONS

    lua_register(L, "vec3", lua_newVec3);
    lua_register(L, "dot", lua_vec3_dot);
    lua_register(L, "cross", lua_vec3_cross);
}
} // namespace

int main()
{
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    registervec3(L);

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
