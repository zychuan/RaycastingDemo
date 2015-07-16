#include "common.h"
#include <time.h>

//////////////////////////window
int WindowWidth = 1024;
int WindowHeight = 1024;

static bool pressed[2];
static float lastMouse[2];
//////////////////////////window
/////////////////////////opengl
float rX = 0, rY = 0;
float dist = 0.0f;
////////////////////////opengl
void initGL()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_NORMALIZE);

	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0);
}

void cameraInit(int w, int h)
{
	WindowWidth = w;
	WindowHeight = h;
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
	gluPerspective(63, (GLfloat)w / (GLfloat)h, 0.1f, 1000.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

static void DrawAxis()
{
	/* draw axis x,y,z	*/
	glLineWidth(1.0f);
	glBegin(GL_LINES);

	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.5f, 0.f, 0.5f);
	glVertex3f(2.0f, 0.f, 0.5f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.5f, 0.f, 0.5f);
	glVertex3f(0.5f, 2.f, 0.5f);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.5f, 0.f, 0.5f);
	glVertex3f(0.5f, 0.f, 2.f);

	glEnd();
}
void mainLoop(GLFWwindow* window)
{
	initGL();
	cameraInit(WindowWidth, WindowHeight);

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
		gluLookAt(0, 0, 1.5, 0, 0, 0, 0.0, 1.0, 0.0);

		glTranslatef(0, 0, dist);
		glRotatef(rX, 1.5, 0.5, 0.5);
		glRotatef(rY, 0.5, 1.5, 0.5);
		glTranslatef(-0.5, -0.5, -0.5);
		DrawAxis();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void WindowSizeCallback(GLFWwindow* window, int w, int h)
{
	cameraInit(w, h);
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	double xd, yd;
	glfwGetCursorPos(window, &xd, &yd);

	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			pressed[0] = true;
			lastMouse[0] = (float)xd;
			lastMouse[1] = (float)yd;
		}
		else if (action == GLFW_RELEASE) {
			pressed[0] = false;
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		if (action == GLFW_PRESS) {
			pressed[1] = true;
			lastMouse[0] = (float)xd;
			lastMouse[1] = (float)yd;
		}
		else if (action == GLFW_RELEASE) {
			pressed[1] = false;
		}
	}
}

void MousePosCallback(GLFWwindow* window, double xd, double yd)
{
	if (pressed[0]) {
		float x = (float)xd / (float)WindowWidth;
		float y = 1.0f - (float)yd / (float)WindowHeight;
		float dx = lastMouse[0] - (float)xd;
		float dy = lastMouse[1] - (float)yd;
	}
	if (pressed[1]) {
		rY += ((float)xd - lastMouse[0]) / 5.0f;
		rX += ((float)yd - lastMouse[1]) / 5.0f;
	}
	lastMouse[0] = (float)xd;
	lastMouse[1] = (float)yd;
}

void MouseScrollCallback(GLFWwindow* window, double xd, double yd)
{
	float x = 0.1f * (float)yd;
	dist += x;
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
	}
}

int main(int argc, char** argv)
{

	try {
		if (!glfwInit())
			throw "Failed to initialize GLFW";

		GLFWwindow* window = glfwCreateWindow(WindowWidth, WindowHeight, "FLIP DEMO", NULL, NULL);
		if (!window) {
			glfwTerminate();
			throw "Failed to open window";
		}

		glfwMakeContextCurrent(window);
		glfwSetMouseButtonCallback(window, MouseButtonCallback);
		glfwSetCursorPosCallback(window, MousePosCallback);
		glfwSetWindowSizeCallback(window, WindowSizeCallback);
		glfwSetScrollCallback(window, MouseScrollCallback);
		glfwSetKeyCallback(window, KeyCallback);
		srand(static_cast <unsigned> (time(0)));
		mainLoop(window);

		glfwDestroyWindow(window);
	}
	catch (const char * error) {
		std::cerr << "ERROR: " << error << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwTerminate();
	return EXIT_SUCCESS;
}