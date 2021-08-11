#include "render/framebuffer.h"

Framebuffer::Framebuffer(Fbuffer_Type t, unsigned int width, unsigned int height)
    : m_fbID(0), m_width(width), m_height(height), m_type(t)
{
    create();
}

void Framebuffer::create()
{
    //fb exist
    if (m_fbID)
    {
        std::cout << "fb exist, deleting fb..." << std::endl;

        glDeleteFramebuffers(1, &m_fbID);
        glDeleteTextures(1, &m_textColorBuffer);
        glDeleteRenderbuffers(1, &m_rbo);
    }

    glGenFramebuffers(1, &m_fbID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbID);

    //create texture
    switch (m_type)
    {
    case COLOR:
        createTexture(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
        break;
    case ID:
        createTexture(GL_R32I, GL_RED_INTEGER, GL_INT);
        break;
    }
    //create RBO for stencil and depth attachment
    createRBO();

    //check error
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::createTexture(GLenum internalFormat, GLenum format, GLenum type)
{
    glGenTextures(1, &m_textColorBuffer);
    glBindTexture(GL_TEXTURE_2D, m_textColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_width, m_height, 0, format, type, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textColorBuffer, 0);
}

void Framebuffer::createRBO()
{
    glGenRenderbuffers(1, &m_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
}

void Framebuffer::bindTexture()
{
    glBindTexture(GL_TEXTURE_2D, m_textColorBuffer);
}

void Framebuffer::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbID);
}

void Framebuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::resize(int w, int h)
{
    m_width = w;
    m_height = h;

    create();
}

int Framebuffer::getWidth()
{
    return m_width;
}

int Framebuffer::getHeight()
{
    return m_height;
}

Framebuffer::~Framebuffer()
{
}
