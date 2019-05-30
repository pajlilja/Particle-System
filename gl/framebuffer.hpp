#pragma once

#include <memory>

#include "ogl.h"
#include "texture.hpp"

class framebuffer {
private:
    GLuint framebuffer_id;
protected:
    framebuffer() {
        glCreateFramebuffers(1, &framebuffer_id);
    }
public:
    static std::shared_ptr<framebuffer>
    genFramebuffer() {
        return std::shared_ptr<framebuffer>(
            new framebuffer,
            [=](framebuffer* fb) {
                glDeleteFramebuffers(1, &fb->framebuffer_id);
                delete fb;
            });
    }

    GLuint get_id() {
        return framebuffer_id;
    }

    void bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);
    }

    void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void framebufferTexture(std::shared_ptr<texture> t, GLenum attachment) {
        glNamedFramebufferTexture(framebuffer_id, attachment, t->get_id(), 0);
    }

    void drawBuffers(GLsizei n, const GLenum* bufs) {
        glNamedFramebufferDrawBuffers(framebuffer_id, n, bufs);
    }

    GLenum checkFramebufferStatus() {
        return glCheckNamedFramebufferStatus(framebuffer_id, GL_FRAMEBUFFER);
    }
};
