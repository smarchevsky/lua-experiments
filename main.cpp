#include <iostream>

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

typedef void (*Func)(lua_State*);

static std::string stringBuf;

bool getGlobal_impl(lua_State* L, const char* str, Func f)
{
    bool success = lua_getglobal(L, str);
    f(L);
    lua_pop(L, 1);
    return success;
};

bool getTableField_impl(lua_State* L, const char* key, Func f)
{
    bool success = lua_getfield(L, -1, key);
    f(L);
    lua_pop(L, 1);
    return success;
};

bool getString_impl(lua_State* L)
{
    bool success = false;
    size_t len;
    if (lua_isstring(L, -1)) {
        const char* ptr = lua_tolstring(L, -1, &len);
        stringBuf.assign(ptr, len);
        success = true;
    }

    return success;
};

#define GET_GLOBAL(str) getGlobal_impl(L, str, [](lua_State* L)
#define GET_FIELD(str) getTableField_impl(L, str, [](lua_State* L)

int main()
{
    lua_State* L = luaL_newstate();

    if (L == NULL) {
        std::cerr << "Failed to create Lua state." << std::endl;
        return 1;
    }

    luaL_openlibs(L);

    Func a = [](lua_State* state) {};
    if (luaL_dofile(L, filePath.c_str()) == LUA_OK) {

        GET_GLOBAL("player")
        {
            GET_FIELD("table")
            {
                GET_FIELD("a")
                {
                    if (getString_impl(L)) {
                        printf("%s\n", stringBuf.c_str());
                    }
                });
            });
        });

        getGlobal_impl(L, "a", [](lua_State* L) {
            if (getString_impl(L)) {
                printf("%s\n", stringBuf.c_str());
            }
        });

    } else
        std::cerr << "Lua error: " << lua_tostring(L, -1) << std::endl;

    lua_close(L);

    return 0;
}
