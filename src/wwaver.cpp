#include "wwaver.h"
#include <iostream>

GLFWwindow* initGLFW(int width, int height);
void compileShaders(Shader* shaders);

Wwaver::Wwaver(int width, int height)
{
    m_window = initGLFW(width, height);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        throw;
    }

    glViewport(0, 0, width, height);

    compileShaders(m_shaders);
}