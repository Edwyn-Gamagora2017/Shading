#include "Figure.h"
#include <iostream>

Figure::Figure( std::vector<Triangle> triangles, bool isMirror, glm::tvec3<float> translation, glm::tvec3<float> rotation, glm::tvec3<float> scale, GLuint program, Texture const &texture, GLenum textureIndex )
{
    /* GENERAL DATA */
    this->isMirror = isMirror;

    /* VERTICES, NORMALS */
    this->triangles = triangles;
    this->nTriangles = triangles.size();
    glGenVertexArrays(1, &this->vertexArray);
    glGenBuffers(1, &this->vertexBuffer);

    /* TRANSFORMATIONS */
    this->translation = translation;
    this->rotation = rotation;
    this->scale = scale;
    this->calculateModeTransf();

    /* PROGRAM */
    this->program = program;
    /* TEXTURE */
	this->textureIndex = textureIndex;
	this->SetTexture( texture );
}

GLuint Figure::Getprogram() { return program; }
GLuint Figure::GetvertexArray() { return vertexArray; }
size_t Figure::GetnTriangles() { return nTriangles; }
Texture Figure::Gettexture() { return texture; }
GLenum Figure::GettextureIndex() { return textureIndex; }
GLuint Figure::GetvertexBuffer() { return vertexBuffer; }
bool Figure::GetIsMirror(){ return this->isMirror; }

void Figure::setTranslation(glm::tvec3<float> translation){ this->translation = translation; this->calculateModeTransf(); }
void Figure::setRotation(glm::tvec3<float> rotation){ this->rotation = rotation; this->calculateModeTransf(); }
void Figure::setScale(glm::tvec3<float> scale){ this->scale = scale; this->calculateModeTransf(); }
glm::tvec3<float> Figure::getTranslation(){ return this->translation; }
glm::tvec3<float> Figure::getRotation(){ return this->rotation; }
glm::tvec3<float> Figure::getScale(){ return this->scale; }
glm::tmat4x4<float> Figure::getModelTransf(){ return this->modelTransf; }

void Figure::initPointersAndBuffers()
{
    /* VERTEX and NORMAL */
    // VertexArray
	glBindVertexArray(this->vertexArray);
	// Buffer
	glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer);

	// fill the buffer
	int size = sizeof(Triangle) * this->nTriangles;
	glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, this->triangles.data());

	// set the VAO
	/*
	3: 3 floats
	GL_FLOAT: c'est des floats
	GL_FALSE: on ne veut pas les normaliser
	3 * 4 * 2: c'est l'espace entre chaque nombre // 3 valeurs * 4 float size * 2 a cause de la normal
	3 float
	3 sizeof(float)
	2 (il y a les normals à passer)
	*/
	int sizeVec3 = 3 * 4;
	int sizeVec2 = 2 * 4;
	// Vertices
	glVertexAttribPointer(Figure::vertexArrayPosition, 3, GL_FLOAT, GL_FALSE, sizeVec3 * 2 + sizeVec2, 0);
	glEnableVertexAttribArray(Figure::vertexArrayPosition);
	// Normals
	glVertexAttribPointer(Figure::normalArrayPosition, 3, GL_FLOAT, GL_FALSE, sizeVec3 * 2 + sizeVec2, (void*) (sizeVec3));
	glEnableVertexAttribArray(Figure::normalArrayPosition);
	// UV
	glVertexAttribPointer(Figure::textureUVPosition, 2, GL_FLOAT, GL_FALSE, sizeVec3 * 2 + sizeVec2, (void*) (2*sizeVec3));
	glEnableVertexAttribArray(Figure::textureUVPosition);

	//glUseProgram(this->program);
}

void Figure::closePointersAndBuffers()
{
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDisableVertexAttribArray(Figure::vertexArrayPosition);
    glDisableVertexAttribArray(Figure::normalArrayPosition);
    glDisableVertexAttribArray(Figure::textureUVPosition);

    //glUseProgram(0);
}

void Figure::SetTexture( const Texture &texture )
{
    this->texture = texture;
}

void Figure::draw()
{
    this->initPointersAndBuffers();

    glActiveTexture(this->textureIndex);
    glBindTexture(GL_TEXTURE_2D, this->texture.tex);

    // Model
    int itModel = glGetUniformLocation(this->program, "modelTransf");
    glUniformMatrix4fv(itModel, 1, false, &(this->modelTransf[0][0]));

    glDrawArrays(GL_TRIANGLES, 0, this->nTriangles * 3);

    glActiveTexture(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    this->closePointersAndBuffers();
}

void Figure::calculateModeTransf()
{
    glm::tmat4x4<float> identityMatrix      = glm::tmat4x4<float>(1.);
    glm::tmat4x4<float> translationTransf   = glm::translate(identityMatrix, this->translation);
    glm::tmat4x4<float> rotationTransfX     = glm::rotate(identityMatrix, degreesToRadians(this->rotation.x), glm::vec3(1.,0.,0.));
    glm::tmat4x4<float> rotationTransfY     = glm::rotate(identityMatrix, degreesToRadians(this->rotation.y), glm::vec3(0.,1.,0.));
    glm::tmat4x4<float> rotationTransfZ     = glm::rotate(identityMatrix, degreesToRadians(this->rotation.z), glm::vec3(0.,0.,1.));
    glm::tmat4x4<float> scaleTransf         = glm::scale(identityMatrix, this->scale);
    this->modelTransf = translationTransf*rotationTransfX*rotationTransfY*rotationTransfZ*scaleTransf;
}

Figure::~Figure()
{
    //dtor
}

Figure& Figure::operator=(const Figure& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    //assignment operator
    return *this;
}
