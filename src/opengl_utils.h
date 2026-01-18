#pragma once

#include <string>

namespace std {
namespace filesystem {
    class path;
}
}

static std::string textFromFile(const std::filesystem::path& path);

static const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 pos;

out vec2 inUV;
void main()
{
    gl_Position = vec4(pos.x, pos.y, 0, 1);
    inUV = pos * vec2(1,-1);
}
)";

typedef void* (*GLADloadproc)(const char* name);
void gladLoad(GLADloadproc load);
void initGL();
void uninitGL();
void glv(int width, int height);

void clear();
void draw();
