#include "library.h"

#define _USE_MATH_DEFINES

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <cstdlib>
#include <cmath>

#include <glad/glad.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <GLFW/glfw3.h>

#define glInfo(a) std::cout << #a << ": " << glGetString(a) << std::endl

#define FOR(i,a) for(int i=0; i<a; i++)

#define PI 3.1415926535897

std::set<std::string> messages;

// This function is called on any openGL API error
void debug(GLenum, // source
		   GLenum, // type
		   GLuint, // id
		   GLenum, // severity
		   GLsizei, // length
		   const GLchar *message,
		   const void *) // userParam
{
	if(messages.count(std::string(message)) == 0)
	{
		messages.insert(std::string(message));
		std::cout << "DEBUG: " << message << std::endl;
	}
}
void error_callback(int error, const char* description)
{
	std::cout << description << std::endl;
}
GLFWwindow* openContext()
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
	{
		std::cerr << "Could not init glfw" << std::endl;
		return nullptr;
	}
	glfwSetErrorCallback(error_callback);


	// This is a debug context, this is slow, but debugs, which is interesting
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 640, "Shader", NULL, NULL);
	if (!window)
	{
		std::cerr << "Could not init window" << std::endl;
		glfwTerminate();
		return nullptr;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	if(!gladLoadGL()) {
		printf("Something went wrong!\n");
		exit(-1);
	}

	// Now that the context is initialised, print some informations
	glInfo(GL_VENDOR);
	glInfo(GL_RENDERER);
	glInfo(GL_VERSION);
	glInfo(GL_SHADING_LANGUAGE_VERSION);

	// And enable debug
//	glEnable(GL_DEBUG_OUTPUT);
//	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

//	glDebugMessageCallback(debug, nullptr);

	return window;
}

void runGL(void (*init)(),void (*render)(const int width, const int height))
{
	auto window = openContext();

	init();

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		render(width, height);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
}

// Build a shader from a string
GLuint buildShader(GLenum const &shaderType, std::string const &src)
{
	GLuint shader = glCreateShader(shaderType);

	const char* ptr = src.c_str();
	GLint length = src.length();

	glShaderSource(shader, 1, &ptr, &length);

	glCompileShader(shader);

	GLint res;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &res);
	if(!res)
	{
		std::cerr << "shader compilation error" << std::endl;

		char message[1000];

		GLsizei readSize;
		glGetShaderInfoLog(shader, 1000, &readSize, message);
		message[999] = '\0';

		std::cerr << message << std::endl;

		glfwTerminate();
		exit(-1);
	}

	return shader;
}

// read a file content into a string
std::string fileGetContents(const std::string &path)
{
	std::ifstream t(path);

	assert(t.is_open());

	std::stringstream buffer;
	buffer << t.rdbuf();

	return buffer.str();
}

// build a program with a vertex shader and a fragment shader
GLuint buildProgram(const std::string &vertexFile, const std::string &fragmentFile)
{
	auto vshader = buildShader(GL_VERTEX_SHADER, fileGetContents(vertexFile));
	auto fshader = buildShader(GL_FRAGMENT_SHADER, fileGetContents(fragmentFile));

	GLuint program = glCreateProgram();

	glAttachShader(program, vshader);
	glAttachShader(program, fshader);

	glLinkProgram(program);

	GLint res;
	glGetProgramiv(program, GL_LINK_STATUS, &res);
	if(!res)
	{
		std::cerr << "program link error" << std::endl;

		char message[1000];

		GLsizei readSize;
		glGetProgramInfoLog(program, 1000, &readSize, message);
		message[999] = '\0';

		std::cerr << message << std::endl;

		glfwTerminate();
		exit(-1);
	}

	return program;
}

Triangle triangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec2 uv1, glm::vec2 uv2, glm::vec2 uv3)
{
    Triangle t;
	t.pn[0].p = p1;
	t.pn[1].p = p2;
	t.pn[2].p = p3;
	glm::vec3 normal = glm::normalize(glm::cross(t.pn[2].p - t.pn[0].p, t.pn[1].p - t.pn[0].p));
	t.pn[0].n = normal;
	t.pn[1].n = normal;
	t.pn[2].n = normal;
	t.pn[0].uv = uv1;
	t.pn[1].uv = uv2;
	t.pn[2].uv = uv3;
	return t;
}
Triangle triangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3){
    return triangle(p1,p2,p3,glm::vec2(),glm::vec2(),glm::vec2());
}
std::vector<Triangle> square(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4){
    //2-3
    //1-4
    std::vector<Triangle> tris;
	tris.push_back(triangle(
                         p1, p2, p3,
                         glm::vec2(0,0),glm::vec2(0,1),glm::vec2(1,1)
                         ));
	tris.push_back(triangle(
                         p1, p3, p4,
                         glm::vec2(0,0),glm::vec2(1,1),glm::vec2(1,0)
                         ));
	return tris;
}
std::vector<Triangle> square(float width, float height)
{
    glm::vec3 p1( -width / 2.,  -height / 2.,   0 );
	glm::vec3 p2( -width / 2.,  height / 2.,    0 );
	glm::vec3 p3( width / 2.,   height / 2.,    0 );
	glm::vec3 p4( width / 2.,   -height / 2.,   0 );

	return square( p1, p2, p3, p4 );
}
std::vector<Triangle> cube(float width, float height, float depth)
{
	std::vector<Triangle> tris;
	// Points
        // Front
        //2-3
        //1-4
	glm::vec3 p1( -width / 2.,  -height / 2.,   depth / 2. );
	glm::vec3 p2( -width / 2.,  height / 2.,    depth / 2. );
	glm::vec3 p3( width / 2.,   height / 2.,    depth / 2. );
	glm::vec3 p4( width / 2.,   -height / 2.,   depth / 2. );
        // Back
        //6-7
        //5-8
    glm::vec3 p5( -width / 2.,  -height / 2.,   -depth / 2. );
	glm::vec3 p6( -width / 2.,  height / 2.,    -depth / 2. );
	glm::vec3 p7( width / 2.,   height / 2.,    -depth / 2. );
	glm::vec3 p8( width / 2.,   -height / 2.,   -depth / 2. );

    // Faces
        // Front
    std::vector<Triangle> square1 = square( p1, p2, p3, p4 );
    tris.insert( tris.end(), square1.begin(), square1.end() );
        // Back
    std::vector<Triangle> square2 = square( p8, p7, p6, p5 );
    tris.insert( tris.end(), square2.begin(), square2.end() );
        // Left
    std::vector<Triangle> square3 = square( p5, p6, p2, p1 );
    tris.insert( tris.end(), square3.begin(), square3.end() );
        // Right
    std::vector<Triangle> square4 = square( p4, p3, p7, p8 );
    tris.insert( tris.end(), square4.begin(), square4.end() );
        // Top
    std::vector<Triangle> square5 = square( p2, p6, p7, p3 );
    tris.insert( tris.end(), square5.begin(), square5.end() );
        // Bottom
    std::vector<Triangle> square6 = square( p4, p8, p5, p1 );
    tris.insert( tris.end(), square6.begin(), square6.end() );

    /*FOR(i,tris.size()){
        std::cout << tris[i].pn[0].n.x << " + "<< tris[i].pn[0].n.y << " + " << tris[i].pn[0].n.z << std::endl;
    }*/

	return tris;
}

std::vector<Triangle> readStl(const std::string &filename)
{
	std::vector<Triangle> tris;

	// 3 points * (3 float * 2 pn)+(2 float) * 4 size)
	static_assert(sizeof(Triangle) == (3 * (3*2+2) * 4), "Triangle is not tighly packed");

	std::ifstream file(filename);
	assert(file.is_open());

	std::string dummy;

	// skip header
	std::getline(file, dummy);

	do
	{
		Triangle t;

		// facet
		file >> dummy;
		// normal
		file >> dummy;

		// skip normal
		file >> dummy;
		file >> dummy;
		file >> dummy;

		// skip outer loop
		file >> dummy;
		file >> dummy;

		// vertex
		for(int i = 0; i < 3; i++)
		{
			file >> dummy;
			for(int j = 0; j < 3; j++)
			{
				file >> t.pn[i].p[j];
			}
		}

		// skip endloop
		file >> dummy;
		// skip endfacet
		file >> dummy;

		if(file.good())
		{
			glm::vec3 normal = glm::normalize(glm::cross( t.pn[1].p - t.pn[0].p, t.pn[2].p - t.pn[0].p ));
			for(int i = 0; i < 3; i++)
				t.pn[i].n = normal;

			tris.push_back(t);
		}
	} while(file.good());

	return tris;
}

std::vector<Triangle> readOff( const std::string &filename )
{
    std::ifstream reader;
	reader.open( std::string(filename+".off").c_str() );
	assert(reader.is_open());

	std::string type;
	reader >> type;
	assert( type == "OFF" );

	int nPoints, nFaces, nEdges;
	reader >> nPoints >> nFaces >> nEdges;
	std::vector<glm::vec3> points;
	std::vector<Triangle> faces;

	double x,y,z;
	int pPerFace, pIndex;
	FOR( i, nPoints )
	{
        reader >> x >> y >> z;
	    points.push_back( glm::vec3(x,y,z) );
	}
	FOR( i, nFaces )
	{
	    reader >> pPerFace;
	    assert(pPerFace == 3);

	    Triangle t;
	    FOR(j,pPerFace)
	    {
            reader >> pIndex;
            t.pn[j].p = points[ pIndex ];
	    }
        // Normal from the fist points
        glm::vec3 normal = glm::normalize(glm::cross( t.pn[1].p - t.pn[0].p, t.pn[2].p - t.pn[0].p ));
        FOR(j,pPerFace)
	    {
            t.pn[j].n = normal;
	    }

        faces.push_back(t);
	}

	reader.close();

	return faces;
}

texImage readPPM(const std::string &filename)
{
	texImage pixels;

	std::ifstream file(filename+".ppm");
	assert(file.is_open());

	std::string type;
	int colorMax;
	float r, g, b;

	file >> type >> pixels.w >> pixels.h >> colorMax;
	assert( type.compare( "P3" ) );

	FOR(i, pixels.w)
		FOR(j, pixels.h)
		{
			file >> r >> g >> b;
			assert(r >= 0 && r <= colorMax &&  g >= 0 && g <= colorMax && b >= 0 && b <= colorMax);
			pixels.colors.push_back(r/colorMax);
			pixels.colors.push_back(g/colorMax);
			pixels.colors.push_back(b/colorMax);
		}

	return pixels;
}
texImage singleColor(int width, int height, float r, float g, float b)
{
    texImage result;
    result.w = width;
    result.h = height;

    FOR(i, width)
		FOR(j, height)
		{
            result.colors.push_back( r );
            result.colors.push_back( g );
            result.colors.push_back( b );
		}

    return result;
}

std::string floatToStr( float value ){
    std::ostringstream o;
    o << value;
    return o.str();
}
std::string vecToString( glm::vec3 v )
{
    return floatToStr(v.x)+" "+floatToStr(v.y)+" "+floatToStr(v.z);
}
float degreesToRadians( float degrees ){
    return (degrees * 2 * PI / 360.f);
}
