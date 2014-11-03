#pragma once

#include <string>

#include <GL/glew.h>

namespace opengl {

struct Texture {
    Texture();
    Texture(std::string const& filename);

    ~Texture();

    GLuint getName() const;
    void bind(GLint layer = 0) const;

    void saveToFile(std::string const& filename) const;

private:
    GLuint name;
};

} // namespace game
