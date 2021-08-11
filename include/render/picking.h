#ifndef PICKING_H
#define PICKING_H

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "render/framebuffer.h"
#include "render/model.h"

#include "game/gameClient.hpp"

class MousePicking
{
private:
    unsigned int m_objectID;
    unsigned int m_idCounter;

    int m_cursorX;
    int m_cursorY;

    Model *m_ground;

    Shader m_idShader;

public:
    Framebuffer m_idfb;

public:
    MousePicking(const char *vshader, const char *fshader, int w, int h);

    void drawIdBuffer(glm::mat4 &proj, glm::mat4 &view, Game::GameClient *game);

    int getObjectID();
    void getXZCoord(int arrXZ[2]);

    void setGroundModel(Model *m);
    void setCursor(int x, int y);
    void resize(int w, int h);

    ~MousePicking();
};

#endif //PICKING_H