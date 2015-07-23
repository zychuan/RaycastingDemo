#include "common.h"
#include "Grid.h"
#include <time.h>
#include <fstream>
#include <sstream>

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

//////////////////////////window
int WindowWidth = 1024;
int WindowHeight = 1024;

//////////////////////////window
//camera transform variables
bool isPressed = false;
int oldX = 0, oldY = 0;
float rX = 10.0f, rY = 10.0f, dist = 2.2f;

//grid object
CGrid* grid;

//modelview projection matrices
glm::mat4 MV,M, V, P;

float last_time = 0, current_time = 0;

//cube vertex array and vertex buffer object IDs
GLuint cubeVBOID;
GLuint cubeVAOID;
GLuint cubeIndicesID;

//pseudo iso-surface ray casting shader
GLSLShader shader;

//background colour
glm::vec4 bg = glm::vec4(1.0, 1.0, 1.0, 1);

//volume dataset filename 
//const std::string volume_file = "media/fluid.raw";
const int frameNums = 150;

//volume dimensions
const int XDIM = 128;
const int YDIM = 128;
const int ZDIM = 128;

//volume texture ID
GLuint textureID;
GLuint *textureIDs = new GLuint[frameNums];

bool isAnimate = true;

bool LoadVolume(int frame) {
	std::stringstream volume_file;
	volume_file << "media/fluid" << frame << ".raw";

	std::ifstream infile(volume_file.str().c_str(), std::ios_base::binary);

	if (infile.good()) {
		//read the volume data file
		GLubyte* pData = new GLubyte[XDIM*YDIM*ZDIM];
		infile.read(reinterpret_cast<char*>(pData), XDIM*YDIM*ZDIM*sizeof(GLubyte));
		infile.close();

		//generate OpenGL texture
		glGenTextures(1, &textureIDs[frame]);
		glBindTexture(GL_TEXTURE_3D, textureIDs[frame]);

		// set the texture parameters
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		//set the mipmap levels (base and max)
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 4);

		//allocate data with internal format and foramt as (GL_RED)			
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, XDIM, YDIM, ZDIM, 0, GL_RED, GL_UNSIGNED_BYTE, pData);
		GL_CHECK_ERRORS

		//generate mipmaps
		glGenerateMipmap(GL_TEXTURE_3D);

		//delete the volume data allocated on heap
		delete[] pData;
		return true;
	}
	else {
		return false;
	}
}

bool LoadVolumes() {
	for (int i = 0; i < frameNums; i++) {
		if (!LoadVolume(i))
			return false;
	}
	return true;
}

void initGL() {

	GL_CHECK_ERRORS

	// create a uniform grid of size 20x20 in XZ plane
	//grid = new CGrid(20, 20);

	GL_CHECK_ERRORS
	
	//Load the raycasting shader
	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/raycaster.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/raycaster.frag");

	//compile and link the shader
	shader.CreateAndLinkProgram();
	shader.Use();

	//add attributes and uniforms
	shader.AddAttribute("vVertex");
	shader.AddUniform("MVP");
	shader.AddUniform("volume");
	shader.AddUniform("camPos");
	shader.AddUniform("step_size");

	//pass constant uniforms at initialization
	glUniform3f(shader("step_size"), 1.0f / XDIM, 1.0f / YDIM, 1.0f / ZDIM);
	glUniform1i(shader("volume"), 0);
	shader.UnUse();

	GL_CHECK_ERRORS

		//load volume data
		if (LoadVolumes()) {
			std::cout << "Volume data loaded successfully." << std::endl;
		}
		else {
			std::cout << "Cannot load volume data." << std::endl;
			exit(EXIT_FAILURE);
		}

		//set background colour
		glClearColor(bg.r, bg.g, bg.b, bg.a);

		//setup unit cube vertex array and vertex buffer objects
		glGenVertexArrays(1, &cubeVAOID);
		glGenBuffers(1, &cubeVBOID);
		glGenBuffers(1, &cubeIndicesID);

		//unit cube vertices 
		glm::vec3 vertices[8] = { glm::vec3(-0.5f, -0.5f, -0.5f),
			glm::vec3(0.5f, -0.5f, -0.5f),
			glm::vec3(0.5f, 0.5f, -0.5f),
			glm::vec3(-0.5f, 0.5f, -0.5f),
			glm::vec3(-0.5f, -0.5f, 0.5f),
			glm::vec3(0.5f, -0.5f, 0.5f),
			glm::vec3(0.5f, 0.5f, 0.5f),
			glm::vec3(-0.5f, 0.5f, 0.5f) };

		//unit cube indices
		GLushort cubeIndices[36] = { 0, 5, 4,
			5, 0, 1,
			3, 7, 6,
			3, 6, 2,
			7, 4, 6,
			6, 4, 5,
			2, 1, 3,
			3, 1, 0,
			3, 0, 7,
			7, 0, 4,
			6, 5, 2,
			2, 5, 1 };
		glBindVertexArray(cubeVAOID);
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBOID);
		//pass cube vertices to buffer object memory
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &(vertices[0].x), GL_STATIC_DRAW);

		GL_CHECK_ERRORS

		//enable vertex attributre array for position
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		//pass indices to element array  buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), &cubeIndices[0], GL_STATIC_DRAW);

		glBindVertexArray(0);

	//enable depth test
	glEnable(GL_DEPTH_TEST);

	//set the over blending function
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	cout << "Initialization successfull" << endl;
}

void cameraInit(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	//setup the projection matrix
	P = glm::perspective(45.0f, (float)w / (float)h, 0.1f, 1000.0f);
}

static int currentFrame = 0;
void mainLoop(GLFWwindow* window)
{
	initGL();
	cameraInit(WindowWidth, WindowHeight);

	while (!glfwWindowShouldClose(window)) {
		GL_CHECK_ERRORS

		glm::mat4 V = glm::lookAt(
			glm::vec3(0.0f, 0.0f, dist),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0, 1, 0)
		);
		
		//set the camera transform
		//glm::mat4 Rx = glm::rotate(Tr, rX, glm::vec3(1.0f, 0.0f, 0.0f));
		//glm::mat4 MV = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));
		M = glm::rotate(glm::mat4(1.0f), rX, glm::vec3(1.0f, 0.0f, 0.0f));
		M = glm::rotate(M, rY, glm::vec3(0.0f, 1.0f, 0.0f));
		MV = V * M;
		//get the camera position
		glm::vec3 camPos = glm::vec3(glm::inverse(MV)*glm::vec4(0, 0, 0, 1));

		//clear colour and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//get the combined modelview projection matrix
		glm::mat4 MVP = P * V * M;

		//render grid
		//grid->Render(glm::value_ptr(MVP));

		//enable blending and bind the cube vertex array object
		glEnable(GL_BLEND);
		glBindVertexArray(cubeVAOID);
		//bind the raycasting shader
		shader.Use();
		//pass shader uniforms
		glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
		glUniform3fv(shader("camPos"), 1, &(camPos.x));

		if (currentFrame >= frameNums)
			currentFrame = 0;

		glBindTexture(GL_TEXTURE_3D, textureIDs[currentFrame]);
		if (isAnimate) currentFrame++;

		//render the cube
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
		//unbind the raycasting shader
		shader.UnUse();
		//disable blending
		glDisable(GL_BLEND);

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

	if (action == GLFW_PRESS) {
		isPressed = true;
		oldX = (int)xd;
		oldY = (int)yd;
	}
	else {
		isPressed = false;
	}
}

void MousePosCallback(GLFWwindow* window, double xd, double yd)
{
	if (isPressed) {
		rX += ((float)yd - oldY) / 5.0f;
		rY += ((float)xd - oldX) / 5.0f;
		oldX = (int)xd;
		oldY = (int)yd;
		//cout << "rX: " << rX << endl;
		//cout << "rY: " << rY << endl;
		//cout << "##########" << endl;
	}
}

void MouseScrollCallback(GLFWwindow* window, double xd, double yd)
{
	dist += 0.1f * (float)yd;
	//cout << "dist: " << dist << endl;
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_S)
			isAnimate = !isAnimate;
	}
}

int main(int argc, char** argv)
{

	try {
		if (!glfwInit())
			throw "Failed to initialize GLFW";

		glfwWindowHint(GLFW_SAMPLES, 4);

		GLFWwindow* window = glfwCreateWindow(WindowWidth, WindowHeight, "Ray Casting DEMO", NULL, NULL);
		if (!window) {
			glfwTerminate();
			throw "Failed to open window";
		}

		glfwMakeContextCurrent(window);
		// Initialize GLEW
		glewExperimental = true; // Needed for core profile
		if (glewInit() != GLEW_OK) {
			fprintf(stderr, "Failed to initialize GLEW\n");
			return -1;
		}

		glfwSetMouseButtonCallback(window, MouseButtonCallback);
		glfwSetCursorPosCallback(window, MousePosCallback);
		glfwSetWindowSizeCallback(window, WindowSizeCallback);
		glfwSetScrollCallback(window, MouseScrollCallback);
		glfwSetKeyCallback(window, KeyCallback);

		srand(static_cast <unsigned> (time(0)));

		//output hardware information
		cout << "\tUsing GLEW " << glewGetString(GLEW_VERSION) << endl;
		cout << "\tVendor: " << glGetString(GL_VENDOR) << endl;
		cout << "\tRenderer: " << glGetString(GL_RENDERER) << endl;
		cout << "\tVersion: " << glGetString(GL_VERSION) << endl;
		cout << "\tGLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

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