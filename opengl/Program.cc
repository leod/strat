#include "opengl/Program.hh"

#include "opengl/Shader.hh"

#include <stdexcept>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cassert>

using namespace glm;

namespace opengl {

Program::Program(Shader const* vertex,
                 Shader const* fragment,
                 Shader const* geometry) {
    assert(vertex != nullptr && "Vertex shader is required.");
    assert(fragment != nullptr && "Fragment shader is required.");

    name = glCreateProgram();
    glAttachShader(name, vertex->getName());
    glAttachShader(name, fragment->getName());
    if (geometry) glAttachShader(name, geometry->getName());

    glLinkProgram(name);

    {
        GLint programOk;
        glGetProgramiv(name, GL_LINK_STATUS, &programOk);
        if (!programOk) {
            GLint logLength;
            char* log;

            glGetProgramiv(name, GL_INFO_LOG_LENGTH, &logLength);
            log = static_cast<char*>(malloc(logLength));
            glGetProgramInfoLog(name, logLength, NULL, log);
            
            std::string str(log); 
            free(log);

            throw std::runtime_error(str);
        }
    }
}

Program::~Program() {
    glDeleteProgram(name);
}

GLint Program::getName() const {
    return name;
}

void Program::bind() const {
    glUseProgram(name);
}

void Program::unbind() const {
    glUseProgram(0);
}

GLuint Program::getUniformLocation(char const* identifier) const {
    return glGetUniformLocation(name, identifier);
}

GLuint Program::getAttribLocation(char const* identifier) const {
    GLuint location = glGetAttribLocation(name, identifier);

    if (location >= GL_MAX_VERTEX_ATTRIBS)
        std::runtime_error(
                std::string("Location ") + identifier +
                " not found in program");
                           
    return location;
}

void Program::setUniform(GLuint location, GLint value) const {
    glUniform1i(location, value);
}

void Program::setUniform(GLuint location, GLfloat value) const {
    glUniform1f(location, value);
}

void Program::setUniform(GLuint location, vec2 const& value) const {
    glUniform2f(location, value.x, value.y);
}

void Program::setUniform(GLuint location, vec3 const& value) const {
    glUniform3f(location, value.x, value.y, value.z);
}

void Program::setUniform(GLuint location, vec4 const& value) const {
    glUniform4f(location, value.x, value.y, value.z, value.w);
}

void Program::setUniform(GLuint location, mat2 const& value) const {
    glUniformMatrix2fv(location, 1, 0, glm::value_ptr(value));
}

void Program::setUniform(GLuint location, mat3 const& value) const {
    glUniformMatrix3fv(location, 1, 0, glm::value_ptr(value));
}

void Program::setUniform(GLuint location, mat4 const& value) const {
    glUniformMatrix4fv(location, 1, 0, glm::value_ptr(value));
}

} // namespace game
