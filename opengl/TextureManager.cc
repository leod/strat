#include "opengl/TextureManager.hh"

#include "util/Log.hh"

namespace opengl {

Texture* TextureManager::load(std::string const& filename) {
    auto textureIt = textures.find(filename);
    if (textureIt != textures.end())
        return textureIt->second.get();

    INFO(opengl) << "Loading texture \"" << filename << "\"";

    Texture* texture = new Texture(filename);
    textures[filename] = std::unique_ptr<Texture>(texture);

    return texture;
}

} // namespace game
