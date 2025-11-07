#include <functional>
#include <iostream>
#include <variant>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

std::string filePath(PROJECT_DIR "/test.lua");

bool tableRetrieveString(lua_State* L, const char* key, std::string& buf)
{
    lua_getfield(L, -1, key);

    bool success = false;
    size_t len;

    if (lua_isstring(L, -1)) {
        const char* ptr = lua_tolstring(L, -1, &len);
        buf.assign(ptr, len);
        success = true;
    }

    lua_pop(L, 1);
    return success;
}

bool tableRetrieveNumber(lua_State* L, const char* key, double& val)
{
    lua_getfield(L, -1, key);

    bool success = false;
    if (lua_isnumber(L, -1)) {
        val = lua_tonumber(L, -1);
        success = true;
    }

    lua_pop(L, 1);

    return success;
}

bool tableRetrieveBool(lua_State* L, const char* key, bool& val)
{
    lua_getfield(L, -1, key);

    bool success = false;
    if (lua_isboolean(L, -1)) {
        val = lua_toboolean(L, -1);
        success = true;
    }

    lua_pop(L, 1);

    return success;
}

bool tableRetrieveTable(lua_State* L, const char* key)
{
    lua_getfield(L, -1, key);

    bool success = false;

    if (lua_istable(L, -1)) {
        // val = lua_tonumber(L, -1);
        printf("it is table\n");
        success = true;
    }

    lua_pop(L, 1);

    return success;
}
/*
class LuaObject;
typedef std::variant<double, bool, char*, LuaObject*> VariantType;

class LuaObject {
    lua_State* L;
    VariantType var;
    bool pushed = false;

public:
    LuaObject(lua_State* _L, const char* str)
        : L(_L)
    {
        lua_getglobal(L, str);
        pushed = true;
    }

    ~LuaObject()
    {
        if (pushed)
            lua_pop(L, 1);
    }

    template <typename T>
    T* getAs()
    {
        return std::get_if<T>(&var);
    }
};
*/

int main()
{

    lua_State* L = luaL_newstate();

    if (L == NULL) {
        std::cerr << "Failed to create Lua state." << std::endl;
        return 1;
    }

    luaL_openlibs(L);

    if (luaL_dofile(L, filePath.c_str()) == LUA_OK) {

        std::string stringBuf;
        auto getGlobal = [&](const char* str, std::function<void()> f) {
            bool success = lua_getglobal(L, str);
            f();
            lua_pop(L, 1);
            return success;
        };

        auto getTableField = [&](const char* key, std::function<void()> f) {
            bool success = lua_getfield(L, -1, key);
            f();
            lua_pop(L, 1);
            return success;
        };

        auto getString = [&]() {
            bool success = false;
            size_t len;
            if (lua_isstring(L, -1)) {
                const char* ptr = lua_tolstring(L, -1, &len);
                stringBuf.assign(ptr, len);
                success = true;
            }

            return success;
        };

        getGlobal("player", [&]() {
            getTableField("table", [&]() {
                getTableField("a", [&]() {
                    if (getString()) {
                        printf("%s\n", stringBuf.c_str());
                    }
                });
            });
        });

        getGlobal("a", [&]() {
            if (getString()) {
                printf("%s\n", stringBuf.c_str());
            }
        });

    } else
        std::cerr << "Lua error: " << lua_tostring(L, -1) << std::endl;

    lua_close(L);

    return 0;
}
