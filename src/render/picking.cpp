#include "render/picking.h"

MousePicking::MousePicking(const char *vshader, const char *fshader, int w, int h)
    : m_idShader(vshader, fshader), m_idfb(ID, w, h)
{
    m_objectID = 0;
    m_idCounter = 0;

    m_cursorX = 0;
    m_cursorY = 0;
}

void MousePicking::drawIdBuffer(glm::mat4 &proj, glm::mat4 &view, Game::GameClient *game)
{
    m_idfb.bind();

    GLint indexcol[] = {-1, 0, 0, 0};
    glClearBufferiv(GL_COLOR, 0, indexcol);
    glClear(GL_DEPTH_BUFFER_BIT);

    m_idShader.use();
    m_idShader.setMat4("projection", proj);
    m_idShader.setMat4("view", view);

    //draw to idbuffer

    float constX = 1.5f;
    float constZ = 0.5f * sqrt(3);
    float posX = 0.0f;
    float posZ = 0.0f;

    for (int z = 0; z < game->m_map.sizeZ; z++)
    {
        for (int x = 0; x < game->m_map.sizeX; x++)
        {
            posX = constX * x;
            //j%2 -> tek cift old ver.: if else
            posZ = (z * (2 * constZ)) + (x % 2) * constZ;
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(posX, 0.0f, posZ));
            int index = (game->m_map.sizeX * z) + x;
            m_idShader.setInt("id", index);
            m_idShader.setMat4("model", model);
            m_ground->Draw(m_idShader);
        }
    }

    //reading objectid from idbuffer
    int px = m_cursorX;
    int py = m_idfb.getHeight() - m_cursorY - 1;
    GLint pixelID;
    glReadPixels(px, py, 1, 1, GL_RED_INTEGER, GL_INT, &pixelID);
    m_objectID = pixelID;

    m_idfb.unbind();
}

int MousePicking::getObjectID()
{
    return m_objectID;
}

void MousePicking::getXZCoord(int arrXZ[2])
{
    const int mapSizeX = 8;
    const int mapSizeZ = 3;

    if(m_objectID == -1)
        return;
    
    arrXZ[0] = m_objectID % mapSizeX;//x
    arrXZ[1] = m_objectID / mapSizeX;//z
}

void MousePicking::setGroundModel(Model *m)
{
    m_ground = m;
}

void MousePicking::setCursor(int x, int y)
{
    m_cursorX = x;
    m_cursorY = y;
}

void MousePicking::resize(int w, int h)
{
    m_idfb.resize(w, h);
}

MousePicking::~MousePicking()
{
}