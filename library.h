#include <vector>
#include <string>

//#include "glad/include/glad/glad.h"
//#include "..\..\..\glfw-3.2.1\include\GLFW\glfw3.h"
//#include "..\..\..\glm-0.9.8.5\glm\gtc\matrix_transform.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#define PRINT(a) std::cout << a << std::endl;

// read a file content into a string
std::string fileGetContents(const std::string &path);

// build a program with a vertex shader and a fragment shader
GLuint buildProgram(const std::string &vertexFile, const std::string &fragmentFile);

struct PNUV
{
	glm::vec3 p; // position
	glm::vec3 n; // normal
	glm::vec2 uv;
};

struct Triangle
{
	PNUV pn[3]; // 3 points make a triangl
};

Triangle triangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
Triangle triangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec2 uv1, glm::vec2 uv2, glm::vec2 uv3);
std::vector<Triangle> square(float width, float height);
std::vector<Triangle> cube(float width, float height, float depth);

// Read an stl file and return a list of triangles
std::vector<Triangle> readStl(const std::string &filename);

struct texImage {
	int w, h;					// width, height
	std::vector<float> colors;	// colors => size = w*h*3
};
// Read an ppm image and return a list of colors
texImage readPPM(const std::string &filename);
texImage singleColor(int width, int height, float r, float g, float b);

void runGL(void (*init)(), void (*render)(const int width, const int height));

std::string vecToString( glm::vec3 v );
float degreesToRadians( float degrees );
