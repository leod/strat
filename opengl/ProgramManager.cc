#include "opengl/ProgramManager.hh"

#include "util/Log.hh"

#include <cassert>

namespace opengl {

Program* ProgramManager::load(std::string const& vertexFilename,
                              std::string const& fragmentFilename,
                              std::string const& geometryFilename) {
    ProgramRequest request(vertexFilename,
                           fragmentFilename,
                           geometryFilename);
    auto programIt = programs.find(request);
    if (programIt != programs.end())
        return programIt->second.get();

    Shader* vertexShader = loadShader(GL_VERTEX_SHADER, vertexFilename);
    Shader* fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentFilename);
    Shader* geometryShader = geometryFilename != "" ?
        loadShader(GL_GEOMETRY_SHADER, geometryFilename) : nullptr;

    Program* program = new Program(vertexShader,
                                   fragmentShader,
                                   geometryShader);
    programs[request] = std::unique_ptr<Program>(program);

    return program;
}

Shader* ProgramManager::loadShader(GLint type, std::string const& filename) {
    INFO(opengl) << "Loading shader \"" << filename << "\"";

    auto shaderIt = shaders.find(filename);
    if (shaderIt != shaders.end()) {
        assert(shaderIt->second->getType() == type &&
               "file was loaded twice with different types.");
        return shaderIt->second.get();
    } 

    Shader* shader = new Shader(type, filename);
    shaders[filename] = std::unique_ptr<Shader>(shader);

    return shader;
}

} // namespace game
