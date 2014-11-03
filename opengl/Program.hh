#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "opengl/Error.hh"
#include "opengl/Buffer.hh"

namespace opengl {

struct Shader;

namespace detail {

template<typename T> struct AttribInfos {
};

template<> struct AttribInfos<glm::vec2> {
    enum {
        Size = 2,
        Type = GL_FLOAT,
        Stride = sizeof(glm::vec2)
    };
};

template<> struct AttribInfos<glm::vec3> {
    enum {
        Size = 3,
        Type = GL_FLOAT,
        Stride = sizeof(glm::vec3)
    };
};

} // namespace detail

struct Program {
    Program(Shader const* vertex,
            Shader const* fragment,
            Shader const* geometry = nullptr);
    ~Program();

    GLint getName() const;
    void bind() const;
    void unbind() const;

    GLuint getUniformLocation(char const*) const;
    GLuint getAttribLocation(char const*) const;

    void setUniform(GLuint location, GLint) const;
    void setUniform(GLuint location, GLfloat) const;
    void setUniform(GLuint location, glm::vec2 const&) const;
    void setUniform(GLuint location, glm::vec3 const&) const;
    void setUniform(GLuint location, glm::vec4 const&) const;
    void setUniform(GLuint location, glm::mat2 const&) const;
    void setUniform(GLuint location, glm::mat3 const&) const;
    void setUniform(GLuint location, glm::mat4 const&) const;

    template<typename T>
    void setAttrib(GLuint location, Buffer<T> const& buffer) const {
        assert(location < GL_MAX_VERTEX_ATTRIBS);
        assert(detail::AttribInfos<T>::Size >= 1 &&
               detail::AttribInfos<T>::Size <= 4);
        assert(detail::AttribInfos<T>::Stride >= 0);

        buffer.bind();
        glVertexAttribPointer(
            location, 
            detail::AttribInfos<T>::Size,
            detail::AttribInfos<T>::Type,
            GL_FALSE,
            detail::AttribInfos<T>::Stride,
            nullptr);
        glEnableVertexAttribArray(location);
    }

    void unsetAttrib(GLuint location) const {
        glDisableVertexAttribArray(location);
    }

private:
    GLint name;
};

} // namespace game
