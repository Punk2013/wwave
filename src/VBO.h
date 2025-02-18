#pragma once

#include <glad/glad.h>

class VBO {
public:
    unsigned int ID;

    VBO();

    void bind() const ;
    static void unbind();
    void data(int size, const void* data, GLenum usage);
};