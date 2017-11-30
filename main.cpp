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
Texture frameTexture, wallTexture, mirrorTexture, portalTexture;
bool monkey = false;
bool captureFrame = false;
std::vector<Figure *> figures;
Figure * mirror, * portal1, * portal2;

bool rotateCamera = true;
float cameraPositionHeight = 2.;
float cameraMoveStep = 0.1;

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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if (glfwGetKey( window, GLFW_KEY_SPACE ) == GLFW_PRESS){
        rotateCamera = !rotateCamera;
    }
    if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
        cameraPositionHeight += cameraMoveStep;
    }
    if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
        cameraPositionHeight -= cameraMoveStep;
    }
}

void init()
{
	// PROGRAM
    program = buildProgram( "cube.vsl", "cube.fsl" );

	// TEXTURE
    initTexture( readPPM("texture-arrow"), &wallTexture );
    initTexture( singleColor(100,100,1,1,1), &mirrorTexture );
    initTexture( singleColor(100,100,1,1,0.), &portalTexture );
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
        //std::vector<Triangle> mesh = readStl("monkey.stl");
        std::vector<Triangle> mesh = readOff("mesh/triceratops");
        // UV
        FOR(i,mesh.size())
            FOR(j,3)
                mesh[i].pn[j].uv = glm::vec2(mesh[i].pn[j].p.x,mesh[i].pn[j].p.z);
        //figures.push_back( new Figure( mesh, false, glm::tvec3<float>(-1,1,0), glm::tvec3<float>(-90.,0.,0.), glm::tvec3<float>(0.5), program, wallTexture, GL_TEXTURE2 ) );
        figures.push_back( new Figure( mesh, false, glm::tvec3<float>(-1.2,1,0), glm::tvec3<float>(0.,0.,0.), glm::tvec3<float>(1.), program, wallTexture, GL_TEXTURE2 ) );

        //figures.push_back( new Figure( square(1, 1), true, glm::tvec3<float>(0.,0.,0.), glm::tvec3<float>(0.,0.,0.), glm::tvec3<float>(1.), program, wallTexture, GL_TEXTURE2 ) );
        // Cube
        figures.push_back( new Figure( cube(1, 1, 1), false, glm::tvec3<float>(0.,1.,0.), glm::tvec3<float>(0.,0.,0.), glm::tvec3<float>(1.), program, wallTexture, GL_TEXTURE2 ) );
        figures.push_back( new Figure( cube(1, 1, 1), false, glm::tvec3<float>(1.1,1.,0.), glm::tvec3<float>(45.,0.,0.), glm::tvec3<float>(1.), program, wallTexture, GL_TEXTURE2 ) );
        // Floor
        //figures.push_back(new Figure( square(1, 1), true, glm::tvec3<float>(0.,0.,0.), glm::tvec3<float>(-90.,0.,0.), glm::tvec3<float>(3.), program, mirrorTexture, GL_TEXTURE2 ));
        mirror = new Figure( square(1, 1), true, glm::tvec3<float>(0.,0.,0.), glm::tvec3<float>(-90.,0.,0.), glm::tvec3<float>(2.), program, mirrorTexture, GL_TEXTURE2 );
        portal1 = new Figure( square(1, 1), false, glm::tvec3<float>(-2.5,1.,0.), glm::tvec3<float>(0.,90.,0.), glm::tvec3<float>(2.), program, portalTexture, GL_TEXTURE2 );
        portal2 = new Figure( square(1, 1), false, glm::tvec3<float>(2.5,1.,0.), glm::tvec3<float>(0.,-90.,0.), glm::tvec3<float>(2.), program, portalTexture, GL_TEXTURE2 );
    }
}

double itMov = 0;
float cameraRatio = 1.;
float cameraDistance = 2;
glm::vec3 cameraPosition = glm::vec3(2, 2, cameraDistance);
glm::vec3 cameraUp = glm::vec3(0, -1, 0);
glm::vec4 lightPosition = glm::vec4(2, 2, 1.5, 6.);
float cameraFovV = 30.;
float cameraNearV = 1.;
float cameraFarV = cameraDistance*6;
glm::vec3 cameraTargetPosition = glm::vec3(0, 1, 0);
glm::tmat4x4<GLfloat> identity = glm::tmat4x4<GLfloat>(1.);

void render(const int width, const int height){
    // reseting buffers
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(.5, .7, .5, 0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

 //   glBindFramebuffer(GL_FRAMEBUFFER, bufferToRender);
    glViewport(0, 0, width, height);

    // Rotate camera
    itMov = itMov + (rotateCamera?0.05:0);
    if (itMov > 360) itMov -= 360;
    cameraPosition = glm::vec3(cos(itMov*PI / 180)*cameraDistance, cameraPositionHeight, sin(itMov*PI / 180)*cameraDistance);

    glm::tmat4x4<GLfloat> cameraProjectionMatrix = glm::perspective<GLfloat>(cameraFovV, cameraRatio, cameraNearV, cameraFarV);
    glm::tmat4x4<GLfloat, glm::precision::packed_highp> cameraLookAtMatrix = glm::lookAt<GLfloat, glm::precision::packed_highp>(cameraPosition, cameraTargetPosition, cameraUp);
    glm::tmat4x4<GLfloat> cameraPVMatrix = cameraProjectionMatrix*cameraLookAtMatrix;
    glm::tmat4x4<GLfloat> mirrorModelMatrix = mirror->getModelTransf()*glm::scale(glm::vec3( 1.,1.,-1. ))*glm::inverse( mirror->getModelTransf() );
    //glm::tmat4x4<GLfloat> portal1ModelMatrix = portal1->getModelTransf()*glm::scale(glm::vec3( 1.,1.,-1. ))*glm::inverse( portal->getModelTransf() );
    glm::tmat4x4<GLfloat> portal1ModelMatrix = glm::scale(glm::vec3( 1.,1.,-1. ))*portal1->getModelTransf()*glm::scale(glm::vec3( 1.,1.,-1. ))*glm::inverse( portal2->getModelTransf() );
    glm::tmat4x4<GLfloat> portal2ModelMatrix = glm::scale(glm::vec3( 1.,1.,-1. ))*portal2->getModelTransf()*glm::scale(glm::vec3( 1.,1.,-1. ))*glm::inverse( portal1->getModelTransf() );

    glUseProgram( program );

    // Uniforms
    // ViewPerspective
    int itPersp = glGetUniformLocation(program, "pvTransf");
    glUniformMatrix4fv(itPersp, 1, false, &(cameraPVMatrix[0][0]));
    int itMirror = glGetUniformLocation(program, "mirrorModelTransf");
    glUniformMatrix4fv(itMirror, 1, false, &(identity[0][0]));

    // TEXTURE
    int itTex = glGetUniformLocation(program, "tex");
    glUniform1i(itTex, 2);

    // Light
    int itLight = glGetUniformLocation(program, "lightInfo");
    glUniform4f(itLight, lightPosition.x, lightPosition.y, lightPosition.z, lightPosition.w);

    // Render Main Camera
    FOR(i,figures.size()){
        figures[i]->draw();
    }

    // Render Stencil = MIRROR
//*
    glEnable(GL_STENCIL_TEST);
    glClear(GL_STENCIL_BUFFER_BIT); // Clear stencil buffer (0 by default)

    // Draw Mirror
    glStencilFunc(GL_ALWAYS, 1, 0xFF); // Set any stencil to 1
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    //glStencilMask(0xFF); // Write to stencil buffer
    glDepthMask(GL_FALSE); // Don't write to depth buffer

    glUniformMatrix4fv(itMirror, 1, false, &(identity[0][0]));
    mirror->draw();

    glStencilFunc(GL_EQUAL, 1, 0xFF); // Pass test if stencil value is 1
    //glStencilMask(0x00); // Don't write anything to stencil buffer
    glDepthMask(GL_TRUE); // Write to depth buffer

    // Render Mirror Camera
    glUniformMatrix4fv(itMirror, 1, false, &(mirrorModelMatrix[0][0]));
    for(auto &f : figures)
    {
        f->draw();
    }

//    glDisable(GL_STENCIL_TEST);
    // END Render Stencil = MIRROR
//*/
//*
    // Render Stencil = Portal1
    glClear(GL_STENCIL_BUFFER_BIT); // Clear stencil buffer (0 by default)

    // Draw Portal
    glStencilFunc(GL_ALWAYS, 1, 0xFF); // Set any stencil to 1
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glDepthMask(GL_FALSE); // Don't write to depth buffer

    glUniformMatrix4fv(itMirror, 1, false, &(identity[0][0]));
    portal1->draw();

    glStencilFunc(GL_EQUAL, 1, 0xFF); // Pass test if stencil value is 1
    glDepthMask(GL_TRUE); // Write to depth buffer

    // Render Mirror Camera
    glUniformMatrix4fv(itMirror, 1, false, &(portal1ModelMatrix[0][0]));
    for(auto &f : figures)
    {
        f->draw();
    }
    // END Render Stencil = Portal1
//*/
//*
    // Render Stencil = Portal2
    glClear(GL_STENCIL_BUFFER_BIT); // Clear stencil buffer (0 by default)

    // Draw Portal
    glStencilFunc(GL_ALWAYS, 1, 0xFF); // Set any stencil to 1
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glDepthMask(GL_FALSE); // Don't write to depth buffer

    glUniformMatrix4fv(itMirror, 1, false, &(identity[0][0]));
    portal2->draw();

    glStencilFunc(GL_EQUAL, 1, 0xFF); // Pass test if stencil value is 1
    glDepthMask(GL_TRUE); // Write to depth buffer

    // Render Mirror Camera
    glUniformMatrix4fv(itMirror, 1, false, &(portal2ModelMatrix[0][0]));
    for(auto &f : figures)
    {
        f->draw();
    }

    glDisable(GL_STENCIL_TEST);
    // END Render Stencil = Portal2
//*/
    glUseProgram(0);
//    glDisable(GL_DEPTH_TEST);
}

int main(void)
{
	runGL(init, render, key_callback);
}
