#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "shader.h"

GLFWwindow* initGLFW(int width, int height)
{
    /* Initialize the library */
    if (!glfwInit())
        throw;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window;
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(width, height, "wwave", NULL, NULL);
    if (!window) {
        glfwTerminate();
        throw;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    return window;
}

void compileShaders(Shader* shaders)
{
    shaders[0] = Shader("src/shaders/wave.vert", "src/shaders/basic.frag"); // wave shader
    shaders[1] = Shader("src/shaders/dft.vert", "src/shaders/basic.frag"); // dft shader
}