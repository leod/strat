#pragma once

#include "opengl/Buffer.hh"

#include <glm/glm.hpp>

#include <map>
#include <memory>
#include <vector>
#include <list>

namespace opengl {

struct Texture;
struct TextureManager;

// Loads a convenient subset of OBJ.
struct OBJ {
    struct Material {
        Texture* texture;

        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;

        Material();

        Material(Material const& a)
            : texture(a.texture), diffuse(a.diffuse) {

        }
    };

    struct Part {
        Material material;

        std::unique_ptr<Buffer<glm::vec3>> vertices;
        std::unique_ptr<Buffer<glm::vec2>> texCoords;
        std::unique_ptr<Buffer<glm::vec3>> normals;

        Part(Material, Buffer<glm::vec3>*, Buffer<glm::vec2>*, Buffer<glm::vec3>*); 
    };

    std::list<Part> parts;

    OBJ(std::string const& filename, TextureManager&);

private:
    typedef std::map<std::string, Material> MaterialLib;

    MaterialLib loadMaterialLib(std::string const& filename, TextureManager&);

    void load(std::string const& filename, TextureManager&);
};

} // namespace game
