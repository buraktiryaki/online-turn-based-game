#include "mainClient.hpp"

MainClient::MainClient(GLFWwindow *w, int sWidth, int sHeight)
    : m_shipMain("../resources/models/mainShip/mainShip.gltf"),
      m_shipGen1("../resources/models/shipL1/shipL1.gltf"),
      m_shipGen2("../resources/models/shipL2/shipL2.gltf"),
      m_shipGen3("../resources/models/shipL3/shipL3.gltf"),
      m_groundModel("../resources/models/hexa.glb"),
      m_shipShader("../shaders/ship2.vs", "../shaders/ship2.fs"),
      m_groundShader("../shaders/ground.vs", "../shaders/ground.fs"),
      m_picker("../shaders/id.vs", "../shaders/id.fs", sWidth, sHeight),
      m_client(this),
      m_game(&m_client)
{
    m_selectedHexXZ[0] = 0;
    m_selectedHexXZ[1] = 0;

    m_scrWidth = sWidth;
    m_scrHeight = sHeight;
    m_Window = w;

    m_DeltaTime = 0.0f;
    m_LastFrame = 0.0f;

    m_lastX = m_scrWidth / 2.0f;
    m_lastY = m_scrHeight / 2.0f;
    m_firstMouse = true;

    m_control = true;

    m_Camera.Position = glm::vec3(5.0f, 8.5f, 9.0f);

    m_groundModel.meshes[0].color.r = 0.0f;
    m_groundModel.meshes[0].color.g = 0.0f;
    m_groundModel.meshes[0].color.b = 0.8f;

    m_groundModel.meshes[1].color.r = 0.0f;
    m_groundModel.meshes[1].color.g = 0.0f;
    m_groundModel.meshes[1].color.b = 0.6f;

    //set a model for mouse picking
    m_picker.setGroundModel(&m_groundModel);
}

void MainClient::connect(const std::string &host, const uint16_t port,
                         int id, const std::string uname)
{
    m_gameId = id;
    m_gameUname = uname;
    m_client.connect(host, port);
}

void MainClient::initWnd()
{
    // bu class ile pencereyi ilişkilendirir. Daha sonra main'deki callbacklerden
    // bu classin objesine erişebileceğiz
    glfwSetWindowUserPointer(m_Window, this);
    glfwSwapInterval(1); //v-sync

    //stbi settings
    //stbi_set_flip_vertically_on_load(true);

    //opengl settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    //set ini file path
    string *s = new string("../resources/imgui.ini");
    io.IniFilename = s->c_str();
}

void MainClient::mainLoop()
{
    while (!glfwWindowShouldClose(m_Window))
    {
        double currentFrame = glfwGetTime();
        m_DeltaTime = currentFrame - m_LastFrame;
        m_LastFrame = currentFrame;

        processInput();
        m_client.runNoLimit();

        glm::mat4 projection =
            glm::perspective(glm::radians(m_Camera.Zoom),
                             (float)m_scrWidth / (float)m_scrHeight, 0.1f, 100.0f);
        //glm::mat4 projection = glm::ortho( 0.0f, 800.0f, 600.0f, 0.0f, 0.01f, 30.0f);

        glm::mat4 view = m_Camera.GetViewMatrix();
        glm::mat4 model;

        if (m_game.m_state == Game::GameState::Started)
        {
            m_picker.drawIdBuffer(projection, view, &m_game);
            int selectedId = m_picker.getObjectID();
        }

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        drawGUI();
        if (m_game.m_state == Game::GameState::Started)
            drawGame(model, view, projection);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_Window);
        glfwPollEvents();
    }
}

void MainClient::drawGame(glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection)
{
    //render ground
    m_groundShader.use();
    m_groundShader.setMat4("projection", projection);
    m_groundShader.setMat4("view", view);

    float constX = 1.5f;
    float constZ = 0.5f * sqrt(3);
    float posX = 0.0f;
    float posZ = 0.0f;
    for (int z = 0; z < m_game.m_map.sizeZ; z++)
    {
        for (int x = 0; x < m_game.m_map.sizeX; x++)
        {
            posX = constX * x;
            //j%2 -> tek cift old ver.: if else
            posZ = (z * (2 * constZ)) + (x % 2) * constZ;
            //m_game.m_map.cells[z][x].pos = glm::vec3(posX, 0.0f, posZ);
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(posX, 0.0f, posZ));
            m_groundShader.setMat4("model", model);

            float selectedCoef = 1.0f;
            if (m_selectedHexXZ[0] == x && m_selectedHexXZ[1] == z)
                selectedCoef = 2.0f;

            //team color
            if (m_game.m_map.cells[z][x].ownerId == 0)
            {
                m_groundModel.meshes[0].color = colorRed * selectedCoef;
                m_groundModel.meshes[1].color = colorRed * 0.4f;
            }
            else if (m_game.m_map.cells[z][x].ownerId == 1)
            {
                m_groundModel.meshes[0].color = colorBlue * selectedCoef;
                m_groundModel.meshes[1].color = colorBlue * 0.4f;
            }
            else
            {
                m_groundModel.meshes[0].color = colorNeutral * selectedCoef;
                m_groundModel.meshes[1].color = colorNeutral * 0.4f;
            }
            m_groundModel.Draw(m_groundShader);
        }
    }

    //render ship
    m_shipShader.use();
    m_shipShader.setMat4("projection", projection);
    m_shipShader.setMat4("view", view);
    //set shader's uniform variables
    m_shipShader.setVec3("viewPos", m_Camera.Position);
    m_shipShader.setVec3("camLight.position", m_Camera.Position);
    m_shipShader.setVec3("camLight.ambient", 0.3f, 0.3f, 0.3f);
    m_shipShader.setVec3("camLight.diffuse", 0.8f, 0.8f, 0.8f);
    m_shipShader.setVec3("camLight.specular", 0.4f, 0.4f, 0.4f);

    posX = 0.0f;
    posZ = 0.0f;
    for (int z = 0; z < m_game.m_map.sizeZ; z++)
    {
        for (int x = 0; x < m_game.m_map.sizeX; x++)
        {
            if (m_game.m_map.cells[z][x].ship)
            {
                posX = constX * x;
                //j%2 -> tek cift old ver.: if else
                posZ = (z * (2 * constZ)) + (x % 2) * constZ;
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(posX, 0.0f, posZ));
                model = glm::rotate(model,
                                    glm::radians(60.0f * (int)m_game.m_map.cells[z][x].ship->rotation),
                                    glm::vec3(0.0f, 1.0f, 0.0f));

                glm::vec4 teamColor;
                if (m_game.m_map.cells[z][x].ownerId == 0)
                {
                    teamColor = colorShipRed;
                }
                else
                {
                    teamColor = colorShipBlue;
                }

                m_shipShader.setMat4("model", model);

                switch (m_game.m_map.cells[z][x].ship->type)
                {
                case Game::SpaceShipType::Main:
                    m_shipMain.meshes[1].color = teamColor;
                    m_shipMain.Draw(m_shipShader);
                    break;
                case Game::SpaceShipType::Gen1:
                    m_shipGen1.meshes[1].color = teamColor;
                    m_shipGen1.Draw(m_shipShader);
                    break;
                case Game::SpaceShipType::Gen2:
                    m_shipGen2.meshes[1].color = teamColor;
                    m_shipGen2.Draw(m_shipShader);
                    break;
                case Game::SpaceShipType::Gen3:
                    m_shipGen3.meshes[1].color = teamColor;
                    m_shipGen3.Draw(m_shipShader);
                    break;
                }
            }
        }
    }
}

void MainClient::drawGUI()
{
    //if mouse on any window set control to false
    ImGuiIO &io = ImGui::GetIO();
    m_control = !io.WantCaptureMouse;

    //ImGui::ShowDemoWindow();

    string temp;
    if (m_game.m_state == Game::GameState::Finished)
    {
        ImGui::Begin("Game Finished");
        if (m_game.m_winningPlayerId == m_gameId)
            ImGui::Text("YOU WON!");
        else
            ImGui::Text("YOU LOST!");
        ImGui::Separator();

        temp = "Number of rounds played: " + to_string(m_game.m_turnCount);
        ImGui::Text(temp.c_str());
        ImGui::End();
        return;
    }

    if (!m_client.isConnected())
    {
        ImGui::Begin("Connection");
        ImGui::Text("Connection Error!");
        ImGui::End();
        m_game.m_state = Game::GameState::Disconnected;
        return;
    }

    static bool GameStatusOpen = true;
    if (GameStatusOpen)
    {
        ImGui::Begin("Game Status");
        if (m_game.m_state == Game::GameState::Waiting)
            ImGui::Text("Waiting for other players.");
        else if (m_game.m_state == Game::GameState::Ready)
            ImGui::Text("Get Ready. Game will start in 5 seconds...");
        else if (m_game.m_state == Game::GameState::Started)
            GameStatusOpen = false;
        ImGui::End();
        return;
    }

    ImGui::Begin("Turn Info");
    ImGui::Columns(2);
    if (m_game.m_turn == m_gameId)
        ImGui::Text("It's Your turn.");
    else
        ImGui::Text("It's the opponent's turn.");

    auto now = chrono::system_clock::now();
    time_t t = chrono::system_clock::to_time_t(now);
    int64_t remainingTime = m_game.m_nextTurnTimeEpoch - t;
    if (remainingTime < 0)
        remainingTime = 0;
    temp = "Remaining Time: " + to_string(remainingTime);
    ImGui::Text(temp.c_str());
    if (ImGui::Button("End Turn"))
    {
        Net::Message msg;
        msg.m_header.id = Net::MsgTypes::EndTurn;
        m_client.send(msg);
    }

    ImGui::NextColumn();
    temp = "Turn count: ";
    temp += to_string(m_game.m_turnCount);
    ImGui::Text(temp.c_str());
    temp = "Turn: ";
    temp += to_string(m_game.m_turn);
    ImGui::Text(temp.c_str());
    ImGui::End();

    ImGui::Begin("Player Info");
    ImGui::Columns(m_game.m_numPlayers, "pi_col");
    for (int i = 0; i < m_game.m_numPlayers; i++)
    {
        if (m_gameId == m_game.m_players[i].id)
            temp = "[YOU]";
        else
            temp = "[OPPONENT]";
        ImGui::Text(temp.c_str());

        temp = "Id: " + to_string(m_game.m_players[i].id);
        ImGui::Text(temp.c_str());
        temp = "Name: " + m_game.m_players[i].name;
        ImGui::Text(temp.c_str());
        temp = "Money: " + to_string(m_game.m_players[i].money);
        ImGui::Text(temp.c_str());
        temp = "Income: " + to_string(m_game.m_players[i].income);
        ImGui::Text(temp.c_str());
        temp = "Number of Cells: " + to_string(m_game.m_players[i].numCells);
        ImGui::Text(temp.c_str());
        temp = "Team: " + to_string(m_game.m_players[i].team);
        ImGui::Text(temp.c_str());
        ImGui::NextColumn();
    }
    ImGui::End();

    //debug screen
    /*
    ImGui::Begin("Debug Info");
    ImGui::Text("...");
    ImGui::End();
    */

    //order screen
    ImGui::Begin("Order Screen");

    /* for debug
    if (ImGui::Button("Increase Money"))
    {
        Net::Message msg;
        msg.m_header.id = Net::MsgTypes::BasicOrder;
        m_client.send(msg);
    }
    */

    //rotate
    ImGui::Separator();
    ImGui::Text("Rotation");
    static int rotation = 0;
    const char *items[] = {"Down", "RightDown", "RightUp", "Up", "LeftUp", "LeftDown"};
    ImGui::Combo("Select Rotation", &rotation, items, IM_ARRAYSIZE(items));
    if (ImGui::Button("Rotate Ship"))
    {
        Net::Message msg;
        msg.m_header.id = Net::MsgTypes::Rotate;
        msg << m_selectedHexXZ[0] << m_selectedHexXZ[1] << rotation;
        m_client.send(msg);
    }

    //move
    ImGui::Separator();
    ImGui::Text("Move");
    static int directionMove = 0;
    ImGui::Combo("Select Direction###sd01", &directionMove, items, IM_ARRAYSIZE(items));
    if (ImGui::Button("Move Ship"))
    {
        Net::Message msg;
        msg.m_header.id = Net::MsgTypes::Move;
        msg << m_selectedHexXZ[0] << m_selectedHexXZ[1] << directionMove;
        m_client.send(msg);
    }

    //attack
    ImGui::Separator();
    ImGui::Text("Attack");
    static int directionAtck = 0;
    ImGui::Combo("Select Direction###sd02", &directionAtck, items, IM_ARRAYSIZE(items));
    if (ImGui::Button("Attack"))
    {
        Net::Message msg;
        msg.m_header.id = Net::MsgTypes::Attack;
        msg << m_selectedHexXZ[0] << m_selectedHexXZ[1] << directionAtck;
        m_client.send(msg);
    }

    //call ship
    ImGui::Separator();
    ImGui::Text("Call Ship");
    static int directionCall = 0;
    static int type = 0; //Game::SpaceShipType
    const char *items2[] = {"Level-1", "Level-2", "Level-3"};
    ImGui::Combo("Select Direction###sd03", &directionCall, items, IM_ARRAYSIZE(items));
    ImGui::Combo("Select Type", &type, items2, IM_ARRAYSIZE(items2));
    if (ImGui::Button("Call a Ship"))
    {
        Net::Message msg;
        msg.m_header.id = Net::MsgTypes::CallShip;
        msg << (type + 1) << directionCall;
        cout << "new ship type: " << type + 1 << endl;
        m_client.send(msg);
    }
    ImGui::End();

    //ship info
    ImGui::Begin("Ship Info");

    int xCoord = m_selectedHexXZ[0];
    int zCoord = m_selectedHexXZ[1];
    temp = "X:" + to_string(xCoord) + " Y:" + to_string(zCoord);
    ImGui::Text(temp.c_str());

    Game::ShipCommon *s = NULL;
    if (xCoord < m_game.m_map.sizeX && zCoord < m_game.m_map.sizeZ &&
        xCoord >= 0 && zCoord >= 0)
    {
        s = m_game.m_map.cells[zCoord][xCoord].ship;
    }

    if (s)
    {
        temp = "HP: " + to_string(s->hitpoints);
        ImGui::Text(temp.c_str());
        temp = "Damage: " + to_string(s->damage);
        ImGui::Text(temp.c_str());
        temp = "Type: " + to_string((int)s->type);
        ImGui::Text(temp.c_str());
        temp = "Rotation: " + to_string((int)s->rotation);
        ImGui::Text(temp.c_str());
        //orders
        temp = "Right to Attack: " + to_string((int)(s->turnAttack));
        ImGui::Text(temp.c_str());
        temp = "Right to Move: " + to_string((int)(s->turnMove));
        ImGui::Text(temp.c_str());
        temp = "Right to Rotate: " + to_string((int)(s->turnRotate));
        ImGui::Text(temp.c_str());
    }
    else
    {
        ImGui::Text("There is no ship.");
    }

    ImGui::End();
}

void MainClient::processInput()
{
    if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS)
    {
        if (m_Camera.Position.z > 3.0f)
            m_Camera.ProcessKeyboard(FORWARD, m_DeltaTime * 1.5);
    }
    if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS)
    {
        if (m_Camera.Position.z < 11.0f)
            m_Camera.ProcessKeyboard(BACKWARD, m_DeltaTime * 1.5);
    }
    if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS)
    {
        if (m_Camera.Position.x > -1.0f)
            m_Camera.ProcessKeyboard(LEFT, m_DeltaTime * 1.5);
    }
    if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS)
    {
        if (m_Camera.Position.x < 10.0f)
            m_Camera.ProcessKeyboard(RIGHT, m_DeltaTime * 1.5);
    }
}

void MainClient::cbFrameBufferSize(int width, int height)
{
    //width, height = 0 when minimize wnd on windows
    if (width != 0 && height != 0)
    {
        m_scrWidth = width;
        m_scrHeight = height;
        glViewport(0, 0, m_scrWidth, m_scrHeight);
        m_picker.resize(m_scrWidth, m_scrHeight);
    }
}

void MainClient::cbMouse(double xpos, double ypos)
{
    m_picker.setCursor((int)xpos, (int)ypos);
}

void MainClient::cbScroll(double xoffset, double yoffset)
{
    m_Camera.ProcessMouseScroll(yoffset);
}

void MainClient::cbKey(int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(m_Window, true);
    else if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
    {
        cout << "dtime: " << 1 / m_DeltaTime << endl;
    }
    else if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        m_client.sendPing();
    }
    else if (key == GLFW_KEY_UP && action == GLFW_PRESS)
    {
        if (m_selectedHexXZ[1] > 0)
            m_selectedHexXZ[1]--;
    }
    else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
    {
        if (m_selectedHexXZ[1] < m_game.m_map.sizeZ - 1)
            m_selectedHexXZ[1]++;
    }
    else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
    {
        if (m_selectedHexXZ[0] > 0)
            m_selectedHexXZ[0]--;
    }
    else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
    {
        if (m_selectedHexXZ[0] < m_game.m_map.sizeX - 1)
            m_selectedHexXZ[0]++;
    }
}

void MainClient::cbMouseButton(int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && m_control)
    {
        m_picker.getXZCoord(m_selectedHexXZ);
    }
}

void MainClient::deallocate()
{
    glDeleteFramebuffers(1, &m_picker.m_idfb.m_fbID);
    glDeleteTextures(1, &m_picker.m_idfb.m_textColorBuffer);
    glDeleteRenderbuffers(1, &m_picker.m_idfb.m_rbo);
}

MainClient::~MainClient()
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::cout << "glErr: " << std::hex << err << std::dec << std::endl;
    }

    m_client.disconnect();
    glfwTerminate();
}