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
bool captureFrame = false;
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
        figures.push_back( new Figure( square(1, 1), true, glm::tvec3<float>(0.,0.,0.), glm::tvec3<float>(0.,0.,0.), glm::tvec3<float>(1.), program, wallTexture, GL_TEXTURE2 ) );
        // Cube
        //figures.push_back( new Figure( cube(1, 1, 1), false, glm::tvec3<float>(0.,0.,0.), glm::tvec3<float>(0.,0.,0.), glm::tvec3<float>(1.), program, wallTexture, GL_TEXTURE2 ) );
        // Floor
        //figures.push_back( new Figure( square(1, 1), true, glm::tvec3<float>(0.,-1,0.), glm::tvec3<float>(-90.,0.,0.), glm::tvec3<float>(3.), program, floorTexture, GL_TEXTURE2 ) );
    }
}

double itMov = 0;
float cameraRatio = 1.;
float cameraDistance = 2;
glm::vec3 cameraPosition = glm::vec3(0, 0, cameraDistance);
glm::vec3 cameraUp = glm::vec3(0, -1, 0);
glm::vec4 lightPosition = glm::vec4(1, 1, 1, 2);
float cameraFovV = 30.;
float cameraNearV = 1.;
float cameraFarV = cameraDistance*3;
glm::vec3 cameraTargetPosition = glm::vec3(0, 0, 0);

void render(const int width, const int height, GLuint bufferToRender, int iteration )
{
    // reseting buffers
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(1, 1, 0, 0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, bufferToRender);
    glViewport(0, 0, width, height);

    // Rotate camera
    itMov = itMov + 1;
    if (itMov > 360) itMov -= 360;
    //cameraPosition = glm::vec3(cos(itMov*PI / 180)*cameraDistance, 1, sin(itMov*PI / 180)*cameraDistance);

    glm::tmat4x4<GLfloat> cameraProjectionMatrix = glm::perspective<GLfloat>(cameraFovV, cameraRatio, cameraNearV, cameraFarV);
    glm::tmat4x4<GLfloat, glm::precision::packed_highp> cameraLookAtMatrix = glm::lookAt<GLfloat, glm::precision::packed_highp>(cameraPosition, cameraTargetPosition, cameraUp);
    glm::tmat4x4<GLfloat> cameraPVMatrix = cameraProjectionMatrix*cameraLookAtMatrix;

    // objects
    FOR(j,figures.size()){
        glUseProgram(figures[j]->Getprogram());

        /*if( figures[j]->GetIsMirror() ){
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_ALWAYS, 1, 0xFF); // Set any stencil to 1
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            glStencilMask(0xFF); // Write to stencil buffer
            glDepthMask(GL_FALSE); // Don't write to depth buffer
            glClear(GL_STENCIL_BUFFER_BIT); // Clear stencil buffer (0 by default)

            // Draw Mirror Content
            glStencilFunc(GL_EQUAL, 1, 0xFF); // Pass test if stencil value is 1
            glStencilMask(0x00); // Don't write anything to stencil buffer
            glDepthMask(GL_TRUE); // Write to depth buffer



            glDisable(GL_STENCIL_TEST);
        }*/
        if( figures[j]->GetIsMirror() ){
            if ( iteration == 0 ){
                //figures[j]->SetTexture( wallTexture );
                render( frameTexture.width, frameTexture.height, frameBuffer, iteration+1 );
                //figures[j]->SetTexture( frameTexture );
                glBindFramebuffer(GL_FRAMEBUFFER, bufferToRender);
                glViewport(0, 0, width, height);
            }
            else{
                //figures[j]->SetTexture( wallTexture );
            }
        }

        // Uniforms
        // ViewPerspective
        int itPersp = glGetUniformLocation(figures[j]->Getprogram(), "pvTransf");
        glUniformMatrix4fv(itPersp, 1, false, &(cameraPVMatrix[0][0]));

        // TEXTURE
        int itTex = glGetUniformLocation(figures[j]->Getprogram(), "tex");
        glUniform1i(itTex, 2);

        // Light
        int itLight = glGetUniformLocation(figures[j]->Getprogram(), "light");
        glUniform4f(itLight, lightPosition.x, lightPosition.y, lightPosition.z, lightPosition.w);

        figures[j]->draw();

        glUseProgram(0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
}

void render(const int width, const int height){
std::cout << "render" << std::endl;
    render( width, height, 0, 0 );
}

int main(void)
{
	runGL(init, render);
}
