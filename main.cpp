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
Texture frameTexture, wallTexture;
bool monkey = true;
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
        figures.push_back( new Figure( trisMonkey, glm::tvec3<float>(0,0,0), glm::tvec3<float>(0.,0.,0.), glm::tvec3<float>(1.), program, wallTexture, GL_TEXTURE2 ) );
    }
    else{
        // Create Square and Cube
        figures.push_back( new Figure( square(1, 1), glm::tvec3<float>(0.,0.,0.), glm::tvec3<float>(0.,0.,0.), glm::tvec3<float>(1.), program, wallTexture, GL_TEXTURE2 ) );
        figures.push_back( new Figure( cube(1, 1, 1), glm::tvec3<float>(1.,1.,1.), glm::tvec3<float>(0.,0.,0.), glm::tvec3<float>(1.), program, wallTexture, GL_TEXTURE2 ) );
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
    float ratio = 1.;
    ratio = width / (float)height;
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
    glm::vec3 cameraP, cameraUp;
    glm::vec4 lightP;
    itMov = itMov + .01;
    if (itMov > 360) itMov -= 360;
    if(monkey)
    {
        // monkey
        distCamera = 3;
        cameraP = glm::vec3(cos(itMov*PI / 180)*distCamera, sin(itMov*PI / 180)*distCamera, 0);
        //cameraP = glm::vec3(0, -distCamera, 0);
        cameraUp = glm::vec3(0, 0, -1);
        lightP = glm::vec4(0, 2, 2, 10);
    }
    else
    {
        // square
        distCamera = 2;
        //cameraP = glm::vec3(cos(itMov*PI / 180)*distCamera, 0, sin(itMov*PI / 180)*distCamera);
        cameraP = glm::vec3(0, 0, -distCamera);
        cameraUp = glm::vec3(0, 1, 0);
        lightP = glm::vec4(1, 1, 1, 2);
    }
    float fovV = 30.;
    float nearV = 1.;
    float farV = distCamera*3;
    glm::vec3 objectP(0, 0, 0);
    glm::tmat4x4<GLfloat> perspM = glm::perspective<GLfloat>(fovV, ratio, nearV, farV);
    glm::tmat4x4<GLfloat, glm::precision::packed_highp> lookAtM = glm::lookAt<GLfloat, glm::precision::packed_highp>(cameraP, objectP, cameraUp);
    glm::tmat4x4<GLfloat> resM = perspM*lookAtM;
    // objects
    FOR(j,figures.size()){
        for (int i = 0; i < (captureFrame?2:1); i++)
        {
            if(captureFrame){
                if (writingBuffer)
                {
                    figures[j]->SetTexture( wallTexture );
                    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
                    glViewport(0, 0, frameTexture.width, frameTexture.height);
                }
                else {
                    figures[j]->SetTexture( frameTexture );
                    glBindFramebuffer(GL_FRAMEBUFFER, 0);
                    glViewport(0, 0, width, height);
                }
                writingBuffer = !writingBuffer;
            }

            // TEST SETTEXTURE
            /*if(writingBuffer) figures[j]->SetTexture(frameTexture);
            if(!writingBuffer) figures[j]->SetTexture(wallTexture);
            writingBuffer = !writingBuffer;*/
            glUseProgram(figures[j]->Getprogram());

            // Uniforms
            // TEXTURE
            int itTex = glGetUniformLocation(figures[j]->Getprogram(), "tex");
            glUniform1i(itTex, 2);

            // ViewPerspective
            int itPersp = glGetUniformLocation(figures[j]->Getprogram(), "pvTransf");
            glUniformMatrix4fv(itPersp, 1, false, &(resM[0][0]));

            // Light
            int itLight = glGetUniformLocation(figures[j]->Getprogram(), "light");
            glUniform4f(itLight, lightP.x, lightP.y, lightP.z, lightP.w);

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
