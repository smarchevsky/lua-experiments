
// #include "lua_functions.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

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

#include <iostream>
#include <unordered_map>

#define ARR_SIZE(x) sizeof(x) / sizeof(x[0])
const char* colors[] = { "\033[0m", "\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m", "\033[37m" };

struct TrieNode {
    bool isWord = false;
    uint8_t colorIndex = 0;
    std::unordered_map<char, TrieNode*> children;
};

class Trie {
    TrieNode root;
    void clear(TrieNode& n)
    {
        for (auto& c : n.children) {
            clear(*c.second);
            delete c.second;
        }
    }

public:
    Trie() { insert("int"), insert("hello"), insert("float"); };
    ~Trie() { clear(root); };

    void insert(const std::string& word)
    {
        TrieNode* node = &root;
        for (auto c : word)
            node = node->children[c] ? node->children[c] : (node->children[c] = new TrieNode());
        // node->colorIndex = index;
        node->isWord = true;
    }

    int match(const char* text, int start, int& colorIndex) const
    {
        const TrieNode* node = &root;
        int len = 0;

        for (int i = start; text[i]; ++i) {
            auto it = node->children.find(text[i]);
            if (it == node->children.end())
                break;

            node = it->second;
            ++len;

            if (node->isWord) {
                colorIndex = node->colorIndex;
                return len;
            }
        }

        return 0;
    }
};

struct ColorMark {
    int pos;
    int colorIndex;
};

bool isIdent(char c) { return isalnum(c) || c == '_'; }

void highlight(ImFontBaked* font, const char* str, int strLen,
    const Trie& trie, std::vector<ColorMark>& marks)
{
    marks.clear();
    int numInvisible = 0;
    for (int i = 0; i < strLen;) {
        int colorIndex = 0;
        int len = trie.match(str, i, colorIndex);

        if (len > 0) {
            int end = i + len;
            char before = (i > 0) ? str[i - 1] : '\0';
            char after = (end < strLen) ? str[end] : '\0';

            if (!isIdent(before) && !isIdent(after)) {
                // output += colors[colorIndex] + line.substr(i, len) + "\033[0m";
                marks.push_back({ i - numInvisible, 1 });
                marks.push_back({ i + len - numInvisible, 0 });
                i += len;
                continue;
            }
        }
        //  output += line[i];
        if (!font->FindGlyph(str[i])->Visible) {
            numInvisible++;
        }
        i++;
    }

    std::vector<ColorMark> compacted;
    for (int i = 0; i < strLen; ++i) {
        if (!font->FindGlyph(str[i])->Visible) {
            numInvisible++;
        }
    }

    // for (auto& m : marks) {
    //     if (m.colorIndex) {
    //         printf("start: %d, ", m.pos);
    //     } else {
    //         printf("end: %d, ", m.pos);
    //     }
    // }

    // if (marks.size())
    //     printf("\n");
    // printf("numInvisible %d  ", numInvisible);
}

bool windowOpen = true;
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    else if (key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_PRESS) {
        windowOpen = !windowOpen;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }

struct TextEditData {
    std::string text;
    std::vector<ColorMark> colorMarks;
    int prevSize = 0;
};

static const Trie trie;

static int InputTextCallback(ImGuiInputTextCallbackData* data)
{
    if (!data)
        return 1;

    auto textEditData = (TextEditData*)data->UserData;
    std::string& text = textEditData->text;
    int& prevSize = textEditData->prevSize;
    auto& marks = textEditData->colorMarks;

    if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit) {
        int start = data->CursorPos - 1;
        auto& buf = data->Buf;

        while (start >= 0) {
            if (!isIdent(buf[start]))
                break;
            start--;
        }

        highlight(data->Ctx->Font->LastBaked, data->Buf, data->BufTextLen, trie, marks);

        return 0;
    }

    if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter) {
        if (data->EventChar == '`') {
            windowOpen = !windowOpen;
            return 1;
        }
    }

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

    glfwSetKeyCallback(window, key_callback);
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
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw our first triangle
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        /////////////////////////////////////////////////////////////////////////////////////////

        // static bool wind
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            static float f = 0.0f;
            static int counter = 0;

            ImGuiInputTextFlags flags = 0
                | ImGuiWindowFlags_NoMove
                | ImGuiWindowFlags_NoResize
                // | ImGuiWindowFlags_NoCollapse
                // | ImGuiWindowFlags_NoSavedSettings
                ;

            ImGui::SetNextWindowPos(ImVec2(40, 40));
            ImGui::SetNextWindowSize(ImVec2(display_w - 80, display_h - 80));
            ImGui::Begin("Lua editor, press ` to toggle window", &windowOpen, flags);

            ImGui::Text("This is some useful text.");
            flags = 0
                | ImGuiInputTextFlags_WordWrap
                | ImGuiInputTextFlags_CallbackResize
                | ImGuiInputTextFlags_CallbackEdit
                | ImGuiInputTextFlags_NoHorizontalScroll
                | ImGuiInputTextFlags_AllowTabInput
                | ImGuiInputTextFlags_CallbackCharFilter;

            if (first_time) {
                // 1. Set the flag to true for the next item
                ImGui::SetKeyboardFocusHere();
                first_time = false;
            }

            static TextEditData editData = { "Initial text..." };

            ImGui::InputTextMultiline("##editor",
                (char*)editData.text.data(), editData.text.size() + 1,
                ImVec2(-1, -1), flags, InputTextCallback, (void*)&editData);

            //            ImGuiID id = ImGui::GetID("##editor");

            // for (int cmd_i = 0; cmd_i < dl->CmdBuffer.Size; cmd_i++) {
            //     const ImDrawCmd& cmd = dl->CmdBuffer[cmd_i];
            //     int vtxCount = cmd.ElemCount; // index count, not vertex count
            //      if (cmd.TexRef == font->OwnerAtlas->TexID) {
            //          printf("offset: %d ", cmd.VtxOffset);
            //          printf("count: %d", cmd.ElemCount);
            //          printf("\n");
            //          // vtxOffset = cmd.VtxOffset;
            //      }
            // }

            if (auto font = ImGui::GetFont()) {
                if (auto fb = font->LastBaked) {
                    // printf("%d\n", fb->FindGlyph('c')->Visible);
                }
            }

            ImGuiWindow* child = ImGui::GetCurrentWindow()->DC.ChildWindows.back();

            auto& vb = child->DrawList->VtxBuffer;

            int numItems = editData.colorMarks.size();
            for (int colorMarkIndex = 0; colorMarkIndex < numItems - 1; ++colorMarkIndex) {
                auto& curr = editData.colorMarks[colorMarkIndex];
                auto& next = editData.colorMarks[colorMarkIndex + 1];
                if (curr.colorIndex == 0)
                    continue;

                auto vertexBufSizeAsChar = vb.Size / 4;
                if (curr.pos >= vertexBufSizeAsChar)
                    break;
                if (next.pos >= vertexBufSizeAsChar)
                    break;
                // assert(curr.pos < editData.text.size() && "curr pos out of range");
                // assert(next.pos < editData.text.size() && "next pos out of range");

                for (int ci = curr.pos; ci < next.pos; ++ci) {
                    for (int vertIndex = 0; vertIndex < 4; ++vertIndex) {
                        vb[ci * 4 + vertIndex].col = ImColor(255, 0, 0, 255);
                    }
                }
            }

            // printf("%d", window->IDStack.size());

            // IDStack

            // printf("%p ", child);
            //  ImDrawList* dl = child->DrawList;

            // for (int i = start; i < end; ++i) {
            //     if (i >= 0 && i < draw_list->VtxBuffer.size())
            //         draw_list->VtxBuffer[i].col = ImColor(255, 0, 0, 255);
            // }

            // for (int i = 0; i < draw_list->VtxBuffer.size(); ++i) {
            //     draw_list->VtxBuffer[i].col = ImColor(255, 0, 0, 255);
            // }

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
