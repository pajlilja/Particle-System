#pragma once

#include <memory>

#include "ogl.h"

class texture {
private:
    GLenum texture_type;
    GLuint texture_id;
    GLuint m_width;
    GLuint m_height;
    GLuint m_format;
protected:
    texture(GLenum texture_type) : texture_type(texture_type) {
        glGenTextures(1, &texture_id);
    }
public:
    static std::shared_ptr<texture> genTexture(GLenum texture_type) {
        return std::shared_ptr<texture>(new texture(texture_type),
                                    [=](texture* t) {
                                        glDeleteTextures(1, &t->texture_id);
                                        delete t;
                                    });
    }

    void bind() {
        glBindTexture(texture_type, texture_id);
    }

    void texImage2D(GLint level, GLint internalformat, GLsizei width, GLsizei height,
                    GLint border, GLenum format, GLenum type, const GLvoid * data)
    {
	m_format = format;
	m_height = height;
	m_width = width;
        bind();
        glTexImage2D(texture_type, level, internalformat, width, height, border, format,
                     type, data);
    }

    void texStorage2D(GLint levels, GLenum internalformat, GLsizei width,
		      GLsizei height) {

	m_width = width;
	m_height = height;
	m_format = internalformat;
	bind();
	glTexStorage2D(texture_type, levels, internalformat, width, height);
    }

    GLenum get_format() {
	return m_format;
    }

    GLuint get_width() {
	return m_width;
    }

    GLuint get_height() {
	return m_height;
    }

    void texParameteri(GLenum pname, GLint param) {
        bind();
        glTexParameteri(texture_type, pname, param);
    }

    void generateMipmap() {
        bind();
        glGenerateMipmap(texture_type);
    }

    GLuint get_id() {
        return texture_id;
    }
};
