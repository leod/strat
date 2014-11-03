#include "opengl/Framebuffer.hh"

#include <iostream>
#include <stdexcept>

#include "opengl/Framebuffer.hh"

using namespace glm;

namespace opengl {

Framebuffer::Framebuffer(Config config, ivec2 size)
    : config(config),
      size(size),
      name(0),
      depthName(0),
      colorTexture(config & COLOR ? new Texture() : nullptr) {
    assert(config & COLOR || config & DEPTH);

    // There's some big names in here, so be careful!

    glGenFramebuffersEXT(1, &name); 
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, name);

    if (colorTexture) {
        glBindTexture(GL_TEXTURE_2D, colorTexture->getName());

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 

        glBindTexture(GL_TEXTURE_2D, 0);

        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                  GL_COLOR_ATTACHMENT0_EXT,
                                  GL_TEXTURE_2D,
                                  colorTexture->getName(),
                                  0);
    }

    if (config & DEPTH) {
        glGenRenderbuffersEXT(1, &depthName);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthName);
        glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,
                                 GL_DEPTH_COMPONENT,
                                 size.x,
                                 size.y);
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
                                     GL_DEPTH_ATTACHMENT_EXT,
                                     GL_RENDERBUFFER_EXT,
                                     depthName);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
                                     GL_DEPTH_ATTACHMENT_EXT,
                                     GL_RENDERBUFFER_EXT,
                                     depthName);
    }

    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
        throw std::runtime_error("Failed to create framebuffer");

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

Framebuffer::~Framebuffer() {
    glDeleteFramebuffersEXT(1, &name);

    if (depthName)
        glDeleteRenderbuffersEXT(1, &name);
}

Texture const& Framebuffer::getColorTexture() const {
    assert(colorTexture);
    return *colorTexture.get();
}

void Framebuffer::renderIntoImpl(std::function<void()> const& f,
                                 Framebuffer::Clear clear) const {
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, name);
    glPushAttrib(GL_VIEWPORT_BIT);
    glViewport(0, 0, size.x, size.y);

    if (clear) {
        GLuint clearFlags = 0;
        if (config & COLOR) clearFlags |= GL_COLOR_BUFFER_BIT;
        if (config & DEPTH) clearFlags |= GL_DEPTH_BUFFER_BIT;

        glClear(clearFlags);
    }

    f();

    glPopAttrib();
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

} // namespace game
