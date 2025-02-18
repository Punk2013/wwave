#include "VAO.h"

VAO::VAO() {
    glGenVertexArrays(1, &ID);
}

void VAO::bind() const {
    glBindVertexArray(ID);
}

void VAO::unbind() {
    glBindVertexArray(0);
}

void VAO::attribute(const VBO& vbo, int location, GLenum type, int count, int stride, bool normalized, void* offset) {
    bind();
    vbo.bind();
    glVertexAttribPointer(location, count, type, normalized, stride, offset);
    glEnableVertexAttribArray(location);
}