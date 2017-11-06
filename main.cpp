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
Texture depthTexture, wallTexture, floorTexture;
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
void initDepthTexture(int width, int height, Texture * texture)
{
    texture->width = width;
    texture->height = height;

    glGenTextures( 1, &(texture->tex) );
	glBindTexture(GL_TEXTURE_2D, texture->tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture->width, texture->height, 0, GL_RGBA, GL_FLOAT, nullptr);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, texture->width, texture->height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
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
    //initTexture( singleColor(1024,1024,1,0,0), &depthTexture );
    initDepthTexture( 1024, 1024, &depthTexture );
	// END TEXTURE

	// FRAMEBUFFER
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depthTexture.tex, 0);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture.tex, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// END FRAMEBUFFER

    // load the stl mesh
    if(monkey){
        std::vector<Triangle> trisMonkey = readStl("monkey.stl");
        // UV
        FOR(i,trisMonkey.size())
            FOR(j,3)
                trisMonkey[i].pn[j].uv = glm::vec2(trisMonkey[i].pn[j].p.x,trisMonkey[i].pn[j].p.z);
        figures.push_back( new Figure( trisMonkey, glm::tvec3<float>(0,0,0), glm::tvec3<float>(0.,0.,0.), glm::tvec3<float>(1.), program, wallTexture, GL_TEXTURE2 ) );
    }
    else{
        figures.push_back( new Figure( square(1, 1), glm::tvec3<float>(0.,0.,0.), glm::tvec3<float>(0.,0.,0.), glm::tvec3<float>(1.), program, wallTexture, GL_TEXTURE2 ) );
        // Cube
        //figures.push_back( new Figure( cube(1, 1, 1), glm::tvec3<float>(0.,0.,0.), glm::tvec3<float>(0.,0.,0.), glm::tvec3<float>(1.), program, wallTexture, GL_TEXTURE2 ) );
        // Floor
        //figures.push_back( new Figure( square(1, 1), glm::tvec3<float>(0.,-0.5,0.), glm::tvec3<float>(-90.,0.,0.), glm::tvec3<float>(3.), program, floorTexture, GL_TEXTURE2 ) );
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
    float ratioCamera = 1.;
    ratioCamera = width / (float)height;
    /*if (width > height)
    {
        ratio = height / (float)width;
        glUniform2f(l, ratio, 1);
    }
    else
    {
        ratio = width / (float)height;
        glUniform2f(l, 1, ratio);
    }*/
        // Perspective and Camera and Light
    float distCamera;
    glm::vec3 cameraPosition, cameraUp;
    glm::vec4 lightPosition;
    glm::vec3 objectPosition(0, 0, 0);

    itMov = itMov + 1;  // cameraRotation

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
        //cameraPosition = glm::vec3(1, 2, distCamera);
        cameraPosition = glm::vec3(0, 0, distCamera);
        //cameraP = glm::vec3(cos(itMov*PI / 180)*distCamera, 0, sin(itMov*PI / 180)*distCamera);
        cameraUp = glm::vec3(0, -1, 0);
        lightPosition = glm::vec4(1, 1, 1, 2);
    }

    float fovCamera = 30.;
    float nearCamera = 1.;
    float farCamera = distCamera*3;

    glm::tmat4x4<GLfloat> perspCameraMatrix = glm::perspective<GLfloat>(fovCamera, ratioCamera, nearCamera, farCamera);
    glm::tmat4x4<GLfloat, glm::precision::packed_highp> lookAtCameraM = glm::lookAt<GLfloat, glm::precision::packed_highp>(cameraPosition, objectPosition, cameraUp);
    glm::tmat4x4<GLfloat, glm::precision::packed_highp> lookAtDepthM = glm::lookAt<GLfloat, glm::precision::packed_highp>(lightPosition, objectPosition, glm::vec3(0,1,0));
    glm::tmat4x4<GLfloat> cameraTransformationMatrix = perspCameraMatrix*lookAtCameraM;
    glm::tmat4x4<GLfloat> depthTransformationMatrix = perspCameraMatrix*lookAtDepthM;

    // objects
    FOR(j,figures.size()){
        for (int i = 0; i < (captureFrame?2:1); i++)
        {
            glUseProgram(figures[j]->Getprogram());

            if(captureFrame){
                if (writingBuffer)
                {
                    figures[j]->SetTexture( wallTexture );
                    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
                    //glDrawBuffer( GL_NONE );
                    glViewport(0, 0, depthTexture.width, depthTexture.height);
                    // ViewPerspective
                    int itPersp = glGetUniformLocation(figures[j]->Getprogram(), "pvTransf");
                    glUniformMatrix4fv(itPersp, 1, false, &(depthTransformationMatrix[0][0]));
                }
                else {
                    figures[j]->SetTexture( depthTexture );
                    glBindFramebuffer(GL_FRAMEBUFFER, 0);
                    //glDrawBuffer( GL_COLOR_ATTACHMENT0 );
                    glViewport(0, 0, width, height);
                    // ViewPerspective
                    int itPersp = glGetUniformLocation(figures[j]->Getprogram(), "pvTransf");
                    glUniformMatrix4fv(itPersp, 1, false, &(cameraTransformationMatrix[0][0]));
                }

                // Indicates we are in the iteration of trying to capture the depth
                int itDepthIteration = glGetUniformLocation(figures[j]->Getprogram(), "depthIteration");
                glUniform1i(itDepthIteration, (captureFrame&&writingBuffer?1:0));
                // the 'far' value for the depth matrix
                int itDepthMaxIteration = glGetUniformLocation(figures[j]->Getprogram(), "depthMax");
                glUniform1f(itDepthMaxIteration, farCamera);

                writingBuffer = !writingBuffer;
            }

            // Uniforms
            // Depth ViewProjection
            int itDepth = glGetUniformLocation(figures[j]->Getprogram(), "depthTransf");
            glUniformMatrix4fv(itDepth, 1, false, &(depthTransformationMatrix[0][0]));

            if( !captureFrame ){
                int itPersp = glGetUniformLocation(figures[j]->Getprogram(), "pvTransf");
                glUniformMatrix4fv(itPersp, 1, false, &(cameraTransformationMatrix[0][0]));
                int itDepthIteration = glGetUniformLocation(figures[j]->Getprogram(), "depthIteration");
                glUniform1i(itDepthIteration, 0);
                int itDepthMaxIteration = glGetUniformLocation(figures[j]->Getprogram(), "depthMax");
                glUniform1f(itDepthMaxIteration, farCamera);
            }

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
