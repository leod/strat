#pragma once

#include <vector>
#include <cassert>
#include <GL/glew.h>

namespace opengl {

template<typename T>
struct Buffer {
    Buffer(GLenum target,
           std::vector<T> const&,
           GLenum usage = GL_STATIC_DRAW);
    ~Buffer();

    GLuint getName() const;
    void bind() const;

    size_t getNumElements() const;

private:
    GLenum target;
    size_t numElements;

    GLuint name;
};

void drawElements(Buffer<GLushort> const&,
                  GLenum mode,
                  GLint count = 0);

template<typename T>
Buffer<T>::Buffer(GLenum target, std::vector<T> const& data, GLenum usage)
    : target(target), numElements(data.size()) {
    assert(glGenBuffers != nullptr);

    glGenBuffers(1, &name);
    glBindBuffer(target, name);
    glBufferData(target, sizeof(T) * numElements, &data[0], usage);
}

template<typename T>
Buffer<T>::~Buffer() {
    glDeleteBuffers(1, &name);
}

template<typename T>
GLuint Buffer<T>::getName() const {
    return name;
}

template<typename T>
void Buffer<T>::bind() const {
    glBindBuffer(target, name);
}

template<typename T>
size_t Buffer<T>::getNumElements() const {
    return numElements;
}

} // namespace game
