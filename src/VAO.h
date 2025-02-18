#pragma once

#include <glad/glad.h>

#include "VBO.h"

class VAO {
public:
    unsigned int ID;

    VAO();

    void bind() const ;
    static void unbind();

    void attribute(const VBO& vbo, int location, GLenum type, int count, int stride, bool normalized = false, void* offset = (void*)0);
};