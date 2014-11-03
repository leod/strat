#pragma once

#include <string>
#include <map>
#include <memory>
#include <tuple>

#include <GL/glew.h>

#include "opengl/Shader.hh"
#include "opengl/Program.hh"

namespace opengl {

struct ProgramManager {
    Program* load(std::string const& vertexFilename,
                  std::string const& fragmentFilename,
                  std::string const& geometryFilename = "");

private:
    typedef std::tuple<std::string, std::string, std::string>
            ProgramRequest;

    std::map<std::string, std::unique_ptr<Shader>> shaders;
    std::map<ProgramRequest, std::unique_ptr<Program>> programs;
    
    Shader* loadShader(GLint type, std::string const& filename);
};

} // namespace game
