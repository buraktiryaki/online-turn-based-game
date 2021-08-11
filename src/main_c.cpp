#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "mainClient.hpp"

#include "netClient.hpp"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

using namespace std;

int main(int argc, char *argv[])
{
	cout << "hi-client" << endl;

	string host = "localhost";
	uint16_t port = 30000;

	int id;
	string uname;

	// ./client 0 burak localhost 30000
	if (argc == 3)
	{
		id = stoi(argv[1]);
		uname = argv[2];
	}
	else if (argc == 5)
	{
		id = stoi(argv[1]);
		uname = argv[2];
		host = argv[3];
		port = stoi(argv[4]);
	}
	else
	{
		cout << "usage: ./client 0 burak localhost 30000" << endl;
		return -1;
	}

	if (!glfwInit())
		exit(-1);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_MAXIMIZED, true);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// delete window decoration
	//glfwWindowHint(GLFW_DECORATED, false);

	GLFWwindow *window = glfwCreateWindow(800, 600, "Client (v0.1)", NULL, NULL);
	if (!window)
	{
		cout << "glfwCreateWindow err..." << endl;
		glfwTerminate();
		exit(-1);
	}

	//glfwMaximizeWindow(window);
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "GLADLoader err..." << endl;
		exit(-1);
	}

	//init clientmain class
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	MainClient game(window, width, height);
	game.connect(host, port, id, uname);
	game.initWnd();
	game.mainLoop();
	game.deallocate();

	cout << "end-client" << endl;
	return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	MainClient *mg = (MainClient *)glfwGetWindowUserPointer(window);
	mg->cbFrameBufferSize(width, height);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	MainClient *mg = (MainClient *)glfwGetWindowUserPointer(window);
	mg->cbKey(key, scancode, action, mods);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	MainClient *mg = (MainClient *)glfwGetWindowUserPointer(window);
	mg->cbMouseButton(button, action, mods);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
	MainClient *mg = (MainClient *)glfwGetWindowUserPointer(window);
	mg->cbMouse(xpos, ypos);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
	MainClient *mg = (MainClient *)glfwGetWindowUserPointer(window);
	mg->cbScroll(xoffset, yoffset);
}
