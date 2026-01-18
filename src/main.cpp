
// #include "lua_functions.h"
#include "TextEditor.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "opengl_utils.h"

// #define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

std::string filePath(PROJECT_DIR "/test.lua");

void stylizeImGui()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    ImGui::StyleColorsDark();
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.ChildRounding = 6.0f;
    style.PopupRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.ScrollbarRounding = 6.0f;
    style.WindowBorderSize = 0.0f;
    style.ItemSpacing = ImVec2(8, 6);
    style.FramePadding = ImVec2(10, 6);
    style.ItemInnerSpacing = ImVec2(6, 4);
    style.IndentSpacing = 20.0f;
    style.FrameBorderSize = 0.0f;
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f); // midle title
    style.WindowMenuButtonPosition = ImGuiDir_None;
    style.ScrollbarSize = 12.0f;
    style.GrabMinSize = 10.0f;
    style.GrabRounding = 4.0f;

    colors[ImGuiCol_TitleBgActive] = ImVec4(0.05f, 0.05f, 0.1f, 0.70f);
    colors[ImGuiCol_Text] = ImVec4(1, 1, 1, 0.70f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.10f);
    colors[ImGuiCol_Button] = ImVec4(0.10f, 0.125f, 0.15f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.35f, 0.40f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.40f, 0.45f, 0.50f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.17f, 1.00f);
}

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glv(width, height); }

bool windowOpen = true;
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    } else if (key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_PRESS) {
        windowOpen = !windowOpen;
    }
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->width, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);

    if (window == nullptr)
        return 1;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    gladLoad((GLADloadproc)glfwGetProcAddress);

    glfwSetKeyCallback(window, key_callback);
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
#if 0
        auto t0 = std::chrono::high_resolution_clock::now();
        ImFont* myFont = io.Fonts->AddFontFromFileTTF("../../proggyfonts/ProggyDotted/ProggyDotted Regular.ttf", 18.0f);
        auto t1 = std::chrono::high_resolution_clock::now() - t0;
        printf("font parsing %llu", std::chrono::duration_cast<std::chrono::microseconds>(t1).count());
        assert(myFont && "no font");
#endif

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

        stylizeImGui();

        ImGuiStyle& style = ImGui::GetStyle();
        float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(primaryMonitor);
        main_scale = 2;
        style.ScaleAllSizes(main_scale);
        style.FontScaleDpi = main_scale;

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    initGL();

    TextEditor te;
    te.SetLanguageDefinition(TextEditor::LanguageDefinition::C());

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        clear();
        draw();

        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            static float f = 0.0f;
            static int counter = 0;

            ImGuiInputTextFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

            if (windowOpen) {
                ImGui::SetNextWindowPos(ImVec2(40, 40));
                ImGui::SetNextWindowSize(ImVec2(display_w - 80, display_h - 80));
                ImGui::Begin("Multicolor text editor", nullptr, flags);

                te.Render("code", ImVec2(-1, -1), true);

                ImGui::End();
            }

            ImGui::Render();

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        glfwSwapBuffers(window);
        fflush(stdout);
    }

    uninitGL();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
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


    } else {
        std::cerr << "Lua error: " << lua_tostring(L, -1) << std::endl;
    }

    lua_close(L);

    return 0;
}*/
