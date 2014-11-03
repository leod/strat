#include "opengl/Texture.hh"

#include <iostream>
#include <stdexcept>
#include <cassert>

namespace opengl {

Texture::Texture() {
    glGenTextures(1, &name);
}

Texture::Texture(std::string const& filename) {
    /*sf::Image image;
    if (!image.loadFromFile(filename))
        throw std::runtime_error("Failed to load texture " + filename);*/

    glGenTextures(1, &name);
    bind();

    /*glTexImage2D(GL_TEXTURE_2D,
                 0, // LOD
                 GL_RGBA,
                 image.getSize().x,
                 image.getSize().y,
                 0, // border
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 image.getPixelsPtr());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);*/

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
            GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
            GL_LINEAR_MIPMAP_LINEAR);
    /*gluBuild2DMipmaps(GL_TEXTURE_2D, 4, image.getSize().x, image.getSize().y,
                      GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr()); */
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

Texture::~Texture() {
    glDeleteTextures(1, &name);
}

GLuint Texture::getName() const {
    return name;
}

void Texture::bind(GLint layer) const {
    glActiveTexture(GL_TEXTURE0 + layer);
    glBindTexture(GL_TEXTURE_2D, name);
}

void Texture::saveToFile(std::string const& filename) const {
    glBindTexture(GL_TEXTURE_2D, name);
    
    GLint width, height;
    GLint format;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT,
            &format);
    assert(format == GL_RGBA);

    //boost::scoped_array<GLubyte> buffer(new GLubyte[width * height * 4]);
    //glGetTexImage(GL_TEXTURE_2D, 0, format, GL_UNSIGNED_BYTE, buffer.get());

    /*sf::Image image;
    //image.create(width, height, buffer.get());
    image.saveToFile(filename);*/

    glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace game
