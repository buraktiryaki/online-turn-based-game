#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <iostream>
#include <glad/glad.h>

enum Fbuffer_Type
{
    COLOR,
    ID
};

class Framebuffer
{
public:
    unsigned int m_fbID;
    unsigned int m_textColorBuffer;
    unsigned int m_rbo;

    unsigned int m_width;
    unsigned int m_height;

    Fbuffer_Type m_type;

public:
    Framebuffer(Fbuffer_Type t, unsigned int width, unsigned int height);

    void create();
    void createTexture(GLenum internalFormat, GLenum format, GLenum type);
    void createRBO();

    void bindTexture();

    void bind();
    void unbind();

    void resize(int w, int h);
    int getWidth();
    int getHeight();

    ~Framebuffer();
};

#endif //FRAMEBUFFER_H
