#pragma once

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "render/shader.h"
#include "render/camera.h"
#include "render/model.h"
#include "render/picking.h"

#include "netClient.hpp"

#include "game/gameClient.hpp"

//colors
const glm::vec4 colorRed = glm::vec4(0.8f, 0.0f, 0.0f, 0.8f);
const glm::vec4 colorBlue = glm::vec4(0.0f, 0.0f, 0.8f, 0.8f);
const glm::vec4 colorNeutral = glm::vec4(0.8f, 0.8f, 0.8f, 0.8f);
const glm::vec4 colorShipRed = glm::vec4(1.0f, 0.45f, 0.0f, 1.0f);
const glm::vec4 colorShipBlue = glm::vec4(0.0f, 0.6f, 1.0f, 1.0f);

class MainClient
{
public:
    unsigned int m_scrWidth;
    unsigned int m_scrHeight;

    double m_DeltaTime;
    double m_LastFrame;

    int m_gameId;
    std::string m_gameUname;
    bool m_control;

    Game::GameClient m_game;
    Net::NetClient m_client;

    GLFWwindow *m_Window;

    MousePicking m_picker;
    int m_selectedHexXZ[2];

    Camera m_Camera;
    float m_lastX;
    float m_lastY;
    bool m_firstMouse;

    Model m_groundModel;
    Model m_shipMain;
    Model m_shipGen1;
    Model m_shipGen2;
    Model m_shipGen3;

    Shader m_groundShader;
    Shader m_shipShader;

public:
    MainClient(GLFWwindow *w, int sWidth, int sHeight);

    void connect(const std::string &host, const uint16_t port,
                 int id, const std::string uname);
    void initWnd();

    void mainLoop();
    void drawGame(glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection);
    void drawGUI();

    //callbacks
    void processInput();
    void cbFrameBufferSize(int width, int height);
    void cbMouse(double xpos, double ypos);
    void cbScroll(double xoffset, double yoffset);
    void cbKey(int key, int scancode, int action, int mods);
    void cbMouseButton(int button, int action, int mods);

    void deallocate();
    ~MainClient();
};
