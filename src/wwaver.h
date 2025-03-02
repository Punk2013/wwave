#pragma once

#define MAX_SHADERS 20

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader.h"
#include "VAO.h"
#include "VBO.h"
#include "loader.h"

class Wwaver
{
public:
    Wwaver(int width, int height);
    void start();
    void end();

// private:
    GLFWwindow *m_window;
    Shader m_shaders[MAX_SHADERS];

    VAO m_vaoWave, m_vaoDft;
    VBO m_vboWave, m_vboDft;

    Loader* m_loader;
};