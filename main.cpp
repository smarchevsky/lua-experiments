#include <iostream>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

std::string filePath(PROJECT_DIR "/test.lua");

/*
bool getString(LuaWrapper& w)
{
    size_t len;
    if (lua_isstring(w.L, -1)) {
        const char* ptr = lua_tolstring(w.L, -1, &len);
        w.stringBuf.assign(ptr, len);
        return true;
    }

    return false;
};

bool getNumber(LuaWrapper& w)
{
    bool success = false;
    size_t len;
    if (lua_isstring(w.L, -1)) {
        const char* ptr = lua_tolstring(w.L, -1, &len);
        w.stringBuf.assign(ptr, len);
        success = true;
    }

    return success;
};
*/

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

int main()
{
    lua_State* L = luaL_newstate();

    if (L == NULL) {
        std::cerr << "Failed to create Lua state." << std::endl;
        return 1;
    }

    luaL_openlibs(L);
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

    } else
        std::cerr << "Lua error: " << lua_tostring(L, -1) << std::endl;

    lua_close(L);

    return 0;
}
