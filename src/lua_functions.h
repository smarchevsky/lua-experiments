#ifndef LUA_FUNCTIONS_H
#define LUA_FUNCTIONS_H

#include "cpp_math.h"

#include <cstring>
#include <iostream>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

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
} // namespace

#define LUA_GET_INPUT(type, name, index) type* name = (type*)luaL_checkudata(L, index, #type "Meta")
#define LUA_GET_FLOAT(name, index) Float name = (Float)luaL_checknumber(L, index)

#define LUA_GET_OUTPUT(type) type* outptr = (type*)lua_newuserdata(L, sizeof(type))
#define LUA_SET_FLOAT(name) lua_pushnumber(L, name)
//
// VEC3
//
namespace {
int vec3_new(lua_State* L)
{
    LUA_GET_FLOAT(x, 1);
    LUA_GET_FLOAT(y, 2);
    LUA_GET_FLOAT(z, 3);
    LUA_GET_OUTPUT(Vec3);

    *outptr = Vec3(x, y, z);

    luaL_getmetatable(L, "Vec3Meta");
    lua_setmetatable(L, -2);
    return 1;
}

int vec3_index_getter(lua_State* L)
{
    LUA_GET_INPUT(Vec3, v, 1);
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

int vec3_index_setter(lua_State* L)
{
    LUA_GET_INPUT(Vec3, v, 1);
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

int vec3_cross(lua_State* L)
{
    LUA_GET_INPUT(Vec3, a, 1);
    LUA_GET_INPUT(Vec3, b, 2);
    LUA_GET_OUTPUT(Vec3);

    *outptr = glm::cross(*a, *b);

    luaL_getmetatable(L, "Vec3Meta");
    lua_setmetatable(L, -2);
    return 1;
}

int vec3_dot(lua_State* L)
{
    LUA_GET_INPUT(Vec3, a, 1);
    LUA_GET_INPUT(Vec3, b, 2);

    Float dot = a->x * b->x + a->y * b->y + a->z * b->z;

    lua_pushnumber(L, dot);
    return 1;
}

typedef Vec3 (*BinaryFuncVec3Vec3)(const Vec3&, const Vec3&);
int binaryFuncVec3Vec3(lua_State* L, BinaryFuncVec3Vec3 func)
{
    Vec3 result;

    if (luaL_testudata(L, 1, "Vec3Meta") && luaL_testudata(L, 2, "Vec3Meta")) {
        LUA_GET_INPUT(Vec3, v1, 1);
        LUA_GET_INPUT(Vec3, v2, 2);
        result = func(*v1, *v2);
    }

    else if (luaL_testudata(L, 1, "Vec3Meta") && lua_isnumber(L, 2)) {
        LUA_GET_INPUT(Vec3, v, 1);
        Float f = (Float)lua_tonumber(L, 2);
        result = func(*v, Vec3(f, f, f));
    }

    else if (lua_isnumber(L, 1) && luaL_testudata(L, 2, "Vec3Meta")) {
        Float f = (Float)lua_tonumber(L, 1);
        LUA_GET_INPUT(Vec3, v, 2);
        result = func(Vec3(f, f, f), *v);

    } else {
        return luaL_error(L, "Invalid operands for vec3 operations");
    }

    LUA_GET_OUTPUT(Vec3);
    *outptr = result;

    luaL_getmetatable(L, "Vec3Meta");
    lua_setmetatable(L, -2);
    return 1;
}

// clang-format off
int vec3_add(lua_State* L) { return binaryFuncVec3Vec3(L, [](const Vec3& a, const Vec3& b) -> Vec3 { return Vec3(a + b); }); }
int vec3_sub(lua_State* L) { return binaryFuncVec3Vec3(L, [](const Vec3& a, const Vec3& b) -> Vec3 { return a - b; }); }
int vec3_mul(lua_State* L) { return binaryFuncVec3Vec3(L, [](const Vec3& a, const Vec3& b) -> Vec3 { return a * b; }); }
int vec3_div(lua_State* L) { return binaryFuncVec3Vec3(L, [](const Vec3& a, const Vec3& b) -> Vec3 { return a / b; }); }
// clang-format on

int vec3_tostring(lua_State* L)
{
    LUA_GET_INPUT(Vec3, v, 1);
    lua_pushfstring(L, "vec3(%f, %f, %f)", (double)v->x, (double)v->y, (double)v->z);
    return 1;
}

//
// QUAT
//

int quat_new(lua_State* L)
{
    Quat result;

    if (lua_isnumber(L, 1) && luaL_testudata(L, 2, "Vec3Meta")) {
        LUA_GET_FLOAT(y, 1);
        LUA_GET_INPUT(Vec3, v, 2);
        result = glm::angleAxis(y, glm::normalize(*v));

    } else {
        LUA_GET_FLOAT(x, 1);
        LUA_GET_FLOAT(y, 2);
        LUA_GET_FLOAT(z, 3);
        LUA_GET_FLOAT(w, 4);
        result = glm::normalize(Quat(x, y, z, w));
    }

    LUA_GET_OUTPUT(Quat);
    *outptr = result;

    luaL_getmetatable(L, "QuatMeta");
    lua_setmetatable(L, -2);
    return 1;
}

int normalize(lua_State* L)
{
    if (luaL_testudata(L, 1, "Vec3Meta")) {
        LUA_GET_INPUT(Vec3, v, 1);
        LUA_GET_OUTPUT(Vec3);
        *outptr = glm::normalize(*v);
        luaL_getmetatable(L, "Vec3Meta");

    } else if (luaL_testudata(L, 1, "QuatMeta")) {
        LUA_GET_INPUT(Quat, q, 1);
        LUA_GET_OUTPUT(Quat);
        *outptr = glm::normalize(*q);
        luaL_getmetatable(L, "QuatMeta");

    } else {
        return luaL_error(L, "Correct operands for normalize: quat, vec3");
    }

    lua_setmetatable(L, -2);
    return 1;
}

int inverse(lua_State* L)
{
    LUA_GET_INPUT(Quat, v, 1);
    LUA_GET_OUTPUT(Quat);
    *outptr = glm::inverse(*v);

    luaL_getmetatable(L, "QuatMeta");
    lua_setmetatable(L, -2);
    return 1;
}

int quat_mul(lua_State* L)
{
    const char* errorMsg = "Correct operands: (quat = quat * quat) or (vec = quat * vec)";

    if (luaL_testudata(L, 1, "QuatMeta")) {
        LUA_GET_INPUT(Quat, q, 1);

        if (luaL_testudata(L, 2, "Vec3Meta")) { // vec = quat * vec
            LUA_GET_INPUT(Vec3, v, 2);
            LUA_GET_OUTPUT(Vec3);
            *outptr = *q * *v;
            luaL_getmetatable(L, "Vec3Meta");
            errorMsg = nullptr;

        } else if (luaL_testudata(L, 2, "QuatMeta")) { // quat = quat * quat
            LUA_GET_INPUT(Quat, q2, 2);
            LUA_GET_OUTPUT(Quat);
            *outptr = glm::normalize(*q * *q2);
            luaL_getmetatable(L, "QuatMeta");
            errorMsg = nullptr;
        }
    }

    if (errorMsg)
        return luaL_error(L, errorMsg);

    lua_setmetatable(L, -2);
    return 1;
}

int quat_tostring(lua_State* L)
{
    LUA_GET_INPUT(Quat, q, 1);
    lua_pushfstring(L, "quat(%f, %f, %f, %f)",
        (double)q->x, (double)q->y, (double)q->z, (double)q->w);
    return 1;
}

//
// SPLINE
//

Spline spline;

int roadSplineLength(lua_State* L)
{
    lua_pushnumber(L, spline.GetLength());
    return 1;
}

int roadSplineDistanceToKey(lua_State* L)
{
    LUA_GET_FLOAT(distance, 1);

    lua_pushnumber(L, spline.DistanceToKey(distance));
    return 1;
}

int roadSplineKeyToDistance(lua_State* L)
{
    LUA_GET_FLOAT(key, 1);
    lua_pushnumber(L, spline.KeyToDistance(key));
    return 1;
}

int roadSplinePositionAtKey(lua_State* L)
{
    LUA_GET_FLOAT(key, 1);
    LUA_GET_OUTPUT(Vec3);

    *outptr = spline.GetInterpAtKey(key).getPos();

    luaL_getmetatable(L, "Vec3Meta");
    lua_setmetatable(L, -2);
    return 1;
}

int roadSplineKeyClosestToPosition(lua_State* L)
{
    LUA_GET_INPUT(Vec3, v, 1);
    lua_pushnumber(L, spline.GetKeyClosestToPosition(*v));
    return 1;
}

int roadSplinePositionAndRotationAtKey(lua_State* L)
{
    float f;
    lua_pushnumber(L, f);
    // in float, out (vec3, quat)
    LUA_GET_OUTPUT(Vec3);

    luaL_getmetatable(L, "Vec3Meta");
    lua_setmetatable(L, -2);
    return 1;
}

/*
lua_register(L, "roadSplineDistanceToKey"         lua_roadSplineDistanceToKey);
lua_register(L, "roadSplinePositionAtKey",         lua_roadSplinePositionAtKey);
lua_register(L, "roadSplinePositionRotationAtKey", lua_roadSplinePositionRotationAtKey);
lua_register(L, "roadSplineKeyClosestToPosition", lua_roadSplineKeyClosestToPosition);

*/

//
// BOT
//

void pushVec3(lua_State* L, const Vec3& v)
{
    Vec3* u = (Vec3*)lua_newuserdata(L, sizeof(Vec3));
    *u = v;
    luaL_getmetatable(L, "Vec3Meta");
    lua_setmetatable(L, -2);
}

bool callLuaProcessVecs(lua_State* L, const Vec3& a, const Vec3& b, float& out1, float& out2)
{
    lua_getglobal(L, "processVecs"); // push function

    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        return false;
    }

    // Push 2 Vec3 args
    pushVec3(L, a);
    pushVec3(L, b);

    // Call: 2 args, expecting 2 returns
    if (lua_pcall(L, 2, 2, 0) != LUA_OK) {
        const char* err = lua_tostring(L, -1);
        printf("Lua error: %s\n", err);
        lua_pop(L, 1);
        return false;
    }

    // Read return values (in reverse order)
    out2 = (float)lua_tonumber(L, -1);
    out1 = (float)lua_tonumber(L, -2);
    lua_pop(L, 2);

    return true;
}

void registerMathFunctions(lua_State* L)
{
    { // VEC3
        lua_register(L, "vec3", vec3_new);
        lua_register(L, "dot", vec3_dot);
        lua_register(L, "cross", vec3_cross);

        luaL_newmetatable(L, "Vec3Meta");
        lua_pushcfunction(L, vec3_add), lua_setfield(L, -2, "__add");
        lua_pushcfunction(L, vec3_sub), lua_setfield(L, -2, "__sub");
        lua_pushcfunction(L, vec3_mul), lua_setfield(L, -2, "__mul");
        lua_pushcfunction(L, vec3_div), lua_setfield(L, -2, "__div");
        lua_pushcfunction(L, vec3_tostring), lua_setfield(L, -2, "__tostring");
        lua_pushcfunction(L, vec3_index_getter), lua_setfield(L, -2, "__index");
        lua_pushcfunction(L, vec3_index_setter), lua_setfield(L, -2, "__newindex");
        lua_pop(L, 1);
    }

    { // QUAT
        lua_register(L, "quat", quat_new);
        lua_register(L, "inverse", inverse);

        luaL_newmetatable(L, "QuatMeta");
        lua_pushcfunction(L, quat_mul), lua_setfield(L, -2, "__mul");
        lua_pushcfunction(L, quat_tostring), lua_setfield(L, -2, "__tostring");
        lua_pop(L, 1);
    }

    // GENERAL FUNCTIONS VEC3
    lua_register(L, "normalize", normalize);

    { // road spline
        lua_register(L, "roadSplineLength", roadSplineLength);
        lua_register(L, "roadSplineDistanceToKey", roadSplineDistanceToKey);
        lua_register(L, "roadSplineKeyToDistance", roadSplineKeyToDistance);
        lua_register(L, "roadSplinePositionAtKey", roadSplinePositionAtKey);
        lua_register(L, "roadSplinePositionRotationAtKey", roadSplinePositionAndRotationAtKey);
        lua_register(L, "roadSplineKeyClosestToPosition", roadSplineKeyClosestToPosition);
    }
}
} // namespace
#endif // LUA_FUNCTIONS_H
