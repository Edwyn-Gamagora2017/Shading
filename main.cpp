//#include "stdafx.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <time.h>
#include <math.h>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

//#include "library.h"
#include "primitives/Figure.h"

#ifndef PI
#define PI 3.14159265
#endif // !PI

#define FOR(i,a) for(int i=0; i<a; i++)

GLuint program;
GLuint frameBuffer;
Texture frameTexture, wallTexture, floorTexture;
bool monkey = false;
bool captureFrame = true;
std::vector<Figure *> figures;

void initTexture(texImage textureValues, Texture * texture)
{
    texture->width = textureValues.w;
    texture->height = textureValues.h;

    glGenTextures( 1, &(texture->tex) );
	glBindTexture(GL_TEXTURE_2D, texture->tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texture->width, texture->height, 0, GL_RGB, GL_FLOAT, &textureValues.colors[0]);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}
void initEmptyTexture(int width, int height, Texture * texture)
{
    texture->width = width;
    texture->height = height;

    glGenTextures( 1, &(texture->tex) );
	glBindTexture(GL_TEXTURE_2D, texture->tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texture->width, texture->height, 0, GL_RGB, GL_FLOAT, nullptr);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void init()
{
	// PROGRAM
    program = buildProgram( "cube.vsl", "cube.fsl" );

	// TEXTURE
    initTexture( readPPM("texture-arrow"), &wallTexture );
    initTexture( singleColor(10,10,1,1,0), &floorTexture );
    initTexture( singleColor(200,200,1,0,0), &frameTexture );
	// END TEXTURE

	// FRAMEBUFFER
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameTexture.tex, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// END FRAMEBUFFER

    // load the stl mesh
    if(monkey){
        std::vector<Triangle> trisMonkey = readStl("monkey.stl");
        // UV
        FOR(i,trisMonkey.size())
            FOR(j,3)
                trisMonkey[i].pn[j].uv = glm::vec2(trisMonkey[i].pn[j].p.x,trisMonkey[i].pn[j].p.z);
        figures.push_back( new Figure( trisMonkey, false, glm::tvec3<float>(0,0,0), glm::tvec3<float>(0.,0.,0.), glm::tvec3<float>(1.), program, wallTexture, GL_TEXTURE2 ) );
    }
    else{
        // Cube
        figures.push_back( new Figure( cube(1, 1, 1), false, glm::tvec3<float>(0.,0.,0.), glm::tvec3<float>(0.,0.,0.), glm::tvec3<float>(1.), program, wallTexture, GL_TEXTURE2 ) );
        // Floor
        figures.push_back( new Figure( square(1, 1), false, glm::tvec3<float>(0.,-0.5,0.), glm::tvec3<float>(-90.,0.,0.), glm::tvec3<float>(3.), program, floorTexture, GL_TEXTURE2 ) );
    }
}

double itMov = 0;
bool writingBuffer = true;
void render(const int width, const int height)
{
    // reseting buffers
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(1, 1, 1, 0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // standard values
        // Color
    float color = 0.3f;
        // Window resize
    float cameraRatio = 1.;
        // Perspective and Camera and Light
    float distCamera;
    glm::vec3 cameraPosition, cameraUp;
    glm::vec4 lightPosition;
    itMov = itMov + 1;
    if (itMov > 360) itMov -= 360;
    if(monkey)
    {
        // monkey
        distCamera = 3;
        cameraPosition = glm::vec3(cos(itMov*PI / 180)*distCamera, sin(itMov*PI / 180)*distCamera, 0);
        //cameraP = glm::vec3(0, -distCamera, 0);
        cameraUp = glm::vec3(0, 0, -1);
        lightPosition = glm::vec4(0, 2, 2, 10);
    }
    else
    {
        // square
        distCamera = 2;
        cameraPosition = glm::vec3(0, 0, distCamera);
        //cameraPosition = glm::vec3(cos(itMov*PI / 180)*distCamera, 1, sin(itMov*PI / 180)*distCamera);
        cameraUp = glm::vec3(0, -1, 0);
        lightPosition = glm::vec4(1, 1, 1, 2);
    }
    float cameraFovV = 30.;
    float cameraNearV = 1.;
    float cameraFarV = distCamera*3;
    glm::vec3 objectPosition(0, 0, 0);
    glm::tmat4x4<GLfloat> cameraProjectionMatrix = glm::perspective<GLfloat>(cameraFovV, cameraRatio, cameraNearV, cameraFarV);
    glm::tmat4x4<GLfloat, glm::precision::packed_highp> cameraLookAtMatrix = glm::lookAt<GLfloat, glm::precision::packed_highp>(cameraPosition, objectPosition, cameraUp);
    glm::tmat4x4<GLfloat> cameraPVMatrix = cameraProjectionMatrix*cameraLookAtMatrix;
    // objects
    FOR(j,figures.size()){
        for (int i = 0; i < (captureFrame?2:1); i++)
        {
            glUseProgram(figures[j]->Getprogram());
            //if(captureFrame){
                if (writingBuffer)
                {
                    figures[j]->SetTexture( wallTexture );
                    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
                    glViewport(0, 0, frameTexture.width, frameTexture.height);

                    // ViewPerspective
                    glm::tmat4x4<GLfloat> objectProjectionMatrix = glm::perspective<GLfloat>(cameraFovV, cameraRatio, cameraNearV, cameraFarV);
                    glm::tmat4x4<GLfloat, glm::precision::packed_highp> objectLookAtMatrix = glm::lookAt<GLfloat, glm::precision::packed_highp>(lightPosition, objectPosition, cameraUp);
                    glm::tmat4x4<GLfloat> objectPVMatrix = objectProjectionMatrix*objectLookAtMatrix;
                    int itPersp = glGetUniformLocation(figures[j]->Getprogram(), "pvTransf");
                    glUniformMatrix4fv(itPersp, 1, false, &(objectPVMatrix[0][0]));
                }
                else {
                    figures[j]->SetTexture( frameTexture );
                    glBindFramebuffer(GL_FRAMEBUFFER, 0);
                    glViewport(0, 0, width, height);

                    // ViewPerspective
                    int itPersp = glGetUniformLocation(figures[j]->Getprogram(), "pvTransf");
                    glUniformMatrix4fv(itPersp, 1, false, &(cameraPVMatrix[0][0]));
                }
                writingBuffer = !writingBuffer;
            //}

            // Uniforms
            // TEXTURE
            int itTex = glGetUniformLocation(figures[j]->Getprogram(), "tex");
            glUniform1i(itTex, 2);

            // Light
            int itLight = glGetUniformLocation(figures[j]->Getprogram(), "light");
            glUniform4f(itLight, lightPosition.x, lightPosition.y, lightPosition.z, lightPosition.w);

            figures[j]->draw();

            glUseProgram(0);
        }
	}
	glDisable(GL_DEPTH_TEST);
}

int main(void)
{
	runGL(init, render);
}
