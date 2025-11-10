
#include "lua_functions.h"
#include <filesystem>
#include <fstream>

std::string filePath(PROJECT_DIR "/test.lua");

static std::string textFromFile(const std::filesystem::path& path)
{
    std::string sourceCode;
    std::ifstream codeStream(path, std::ios::in);
    if (codeStream.is_open()) {
        std::stringstream sstr;
        sstr << codeStream.rdbuf();
        sourceCode = sstr.str();
        codeStream.close();
    } else {
        printf("Can't open file: %s\n", path.c_str());
    }
    return sourceCode;
}

/*int main()
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

        // if (auto f = LuaGlobal(L, "player")) {
        //     if (auto f = LuaField(L, "table")) {
        //         if (auto f = LuaField(L, "a")) {
        //             if (auto f = LuaString(L, stringBuf)) {
        //                 printf("%s\n", stringBuf.c_str());
        //             }
        //         }
        //     }
        // }

        // lua_getglobal(L, "executeFromCpp"); // stack: [function]
        // lua_pushstring(L, "World"); // stack: [function, "World"]

        // if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
        //     std::cerr << "Error: " << lua_tostring(L, -1) << "\n";
        //     lua_pop(L, 1);
        // } else {
        //     const char* result = lua_tostring(L, -1);
        //     std::cout << result << "\n";
        //     lua_pop(L, 1);
        // }

        Vec3 a { 1, 2, 3 };
        Vec3 b { 4, 5, 6 };

        float dot = 0, dist = 0;
        if (callLuaProcessVecs(L, a, b, dot, dist)) {
            printf("dot=%.2f dist=%.2f\n", dot, dist);
        }


    } else
        std::cerr << "Lua error: " << lua_tostring(L, -1) << std::endl;

    lua_close(L);

    return 0;
}*/

#include "glad.h"
#include <GLFW/glfw3.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 pos;

out vec2 inUV;
void main()
{
    gl_Position = vec4(pos.x, pos.y, 0, 1);
    inUV = pos * vec2(1,-1);
}
)";

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    //
    //
    auto fragment = textFromFile(PROJECT_DIR "/shaders/shader.glsl");
    const char* fragmentChar = fragment.c_str();
    //
    //

    glShaderSource(fragmentShader, 1, &fragmentChar, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    float vertices[] = { -1, -1, 1, -1, 1, 1, -1, -1, -1, 1, 1, 1 };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw our first triangle
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
