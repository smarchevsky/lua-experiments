
// #include "lua_functions.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "glad.h"

// #define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#include <vector>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

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

struct TextEditData {
    // std::vector<char> text;
    std::string text;
};
// The custom callback function
// It MUST be a C-style function (static or global)
static int InputTextCallback(ImGuiInputTextCallbackData* data)
{
    if (!data)
        return 1;
    auto& text = ((TextEditData*)data->UserData)->text;

    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        if (data->BufTextLen > text.capacity()) {
            text.resize(data->BufTextLen + 4);
            data->Buf = text.data();
            return 0;
        }
    }

    return 0;
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
    GLFWwindow* window = glfwCreateWindow((int)(1280 * main_scale), (int)(800 * main_scale), "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);

    if (window == nullptr)
        return 1;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        main_scale = 2;
        style.ScaleAllSizes(main_scale);
        style.FontScaleDpi = main_scale;

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    unsigned int VBO, VAO;
    unsigned int shaderProgram;
    {
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

        shaderProgram = glCreateProgram();
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

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    bool first_time = true;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw our first triangle
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        /////////////////////////////////////////////////////////////////////////////////////////

        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            static float f = 0.0f;
            static int counter = 0;
            ImGui::Begin("Hello, world!");

            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            int start = draw_list->VtxBuffer.Size;
            int end = draw_list->VtxBuffer.Size;

            ImGui::Text("This is some useful text.");
            ImGuiInputTextFlags flags = 0
                | ImGuiInputTextFlags_WordWrap
                | ImGuiInputTextFlags_CallbackResize
                | ImGuiInputTextFlags_NoHorizontalScroll
                | ImGuiInputTextFlags_AllowTabInput
                | ImGuiInputTextFlags_CallbackCharFilter;

            if (first_time) {
                // 1. Set the flag to true for the next item
                ImGui::SetKeyboardFocusHere();
                first_time = false;
            }

            static TextEditData editor_state = { "Initial text..." };
            ImGui::InputTextMultiline("###TextEditWindow", (char*)editor_state.text.data(),
                editor_state.text.size() + 1, ImVec2(-1, -1), flags, InputTextCallback, (void*)&editor_state);

            for (int i = start; i < end; ++i) {
                //    if (i < start + 8)
                //        draw_list->VtxBuffer[i].col = ImColor(255, 0, 0, 255);
            }

            // printf("%d %d\n", start, end);

            // ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
            // ImGui::ColorEdit3("clear color", (float*)&clear_color);
            // if (ImGui::Button("Button"))
            //     counter++;
            // ImGui::SameLine();
            // ImGui::Text("counter = %d", counter);
            // ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();

            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        /////////////////////////////////////////////////////////////////////////////////////////
        glfwSwapBuffers(window);
        fflush(stdout);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
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
