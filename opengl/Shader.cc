#include "opengl/Shader.hh"

#include <stdexcept>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <iostream>

namespace opengl {

static std::string readFile(std::string const& filename) {
    std::ifstream file(filename, std::ios::binary);

    if (!file)
        throw std::runtime_error("File " + filename + " not found.");

    std::string contents;

    while (file.good()) {
        char c;
        file.read(&c, 1);

        contents += c;
    }

    return contents;
}

Shader::Shader(GLenum type, std::string const& filename) {
    // Read shader file, create and compile shader object
    {
        std::string contents = readFile(filename);
        char const* source = contents.c_str();
        GLint sourceLength = contents.size();

        name = glCreateShader(type);
        glShaderSource(name, 1, &source, &sourceLength);
        glCompileShader(name);
    }

    // Check for errors
    {
        GLint shaderOk;
        glGetShaderiv(name, GL_COMPILE_STATUS, &shaderOk);
        if (!shaderOk) {
            GLint logLength;
            char* log;

            glGetShaderiv(name, GL_INFO_LOG_LENGTH, &logLength);
            log = static_cast<char*>(malloc(logLength + 1));
            glGetShaderInfoLog(name, logLength, NULL, log);
            
            std::string str(log); 
            free(log);

            throw std::runtime_error("Error in " + filename + ": " + str);
        }
    }
}

Shader::~Shader() {
    glDeleteShader(name);
}

GLuint Shader::getName() const {
    return name;
}

GLint Shader::getType() const {
    GLint type;
    glGetShaderiv(name, GL_SHADER_TYPE, &type);
    return type;
}

} // namespace game
