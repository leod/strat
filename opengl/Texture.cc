#include "opengl/Texture.hh"

#include <iostream>
#include <stdexcept>
#include <cassert>

#include <IL/il.h>

namespace opengl {

Texture::Texture() {
    glGenTextures(1, &name);
}

Texture::Texture(std::string const& filename) {
    ILuint image;
    ilGenImages(1, &image);
    ilBindImage(image);
    if (!ilLoadImage(filename.c_str()))
        throw std::runtime_error("Couldn't load image " + filename);

    glGenTextures(1, &name);
    bind();

    //ILint format = ilGetInteger(IL_IMAGE_FORMAT);
    //assert(format == IL_RGB || format == IL_RGBA);

    glTexImage2D(GL_TEXTURE_2D,
                 0, // LOD
                 ilGetInteger(IL_IMAGE_BPP),
                 ilGetInteger(IL_IMAGE_WIDTH),
                 ilGetInteger(IL_IMAGE_HEIGHT),
                 0, // border
                 ilGetInteger(IL_IMAGE_FORMAT),
                 GL_UNSIGNED_BYTE,
                 ilGetData());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    /*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
            GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
            GL_LINEAR_MIPMAP_LINEAR);*/
    /*gluBuild2DMipmaps(GL_TEXTURE_2D, 4, width, height,
                      GL_RGBA, GL_UNSIGNED_BYTE, ilGetData()); */
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    ilDeleteImages(1, &image);

    glBindTexture(GL_TEXTURE_2D, 0);
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
