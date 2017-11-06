#ifndef FIGURE_H
#define FIGURE_H

#include "../library.h"
#include <string>

struct Texture{
    GLuint tex;
    int width;
    int height;
};

class Figure
{
    public:
        static const int vertexArrayPosition = 0;
        static const int normalArrayPosition = 1;
        static const int textureUVPosition = 2;

        /** Default constructor */
        Figure( std::vector<Triangle> triangles, bool isMirror, glm::tvec3<float> translation, glm::tvec3<float> rotation, glm::tvec3<float> scale, GLuint program, Texture const &texture, GLenum textureIndex );
        /** Default destructor */
        virtual ~Figure();
        /** Assignment operator
         *  \param other Object to assign from
         *  \return A reference to this
         */
        Figure& operator=(const Figure& other);

        GLuint Getprogram();
        GLuint GetvertexArray();
        size_t GetnTriangles();
        Texture Gettexture();
        GLenum GettextureIndex();
        GLuint GetvertexBuffer();
        bool GetIsMirror();
        void SetTexture( const Texture &tex );

        void setTranslation(glm::tvec3<float> translation);
        void setRotation(glm::tvec3<float> rotation);
        void setScale(glm::tvec3<float> scale);
        glm::tvec3<float> getTranslation();
        glm::tvec3<float> getRotation();
        glm::tvec3<float> getScale();
        glm::tmat4x4<float> getModelTransf();

        void draw();

    protected:
        std::vector<Triangle> triangles; //!< Member variable "vertexBuffer"
        glm::tvec3<float> translation;
        glm::tvec3<float> rotation;
        glm::tvec3<float> scale;
        glm::tmat4x4<float> modelTransf;
        bool isMirror;

        GLuint program; //!< Member variable "program"
        GLuint vertexArray; //!< Member variable "vertexArray"
        size_t nTriangles; //!< Member variable "nTriangles"
        Texture texture; //!< Member variable "texture"
        GLenum textureIndex; //!< GLTexture to be used form the Figure. eg: GLTexture0
        GLuint vertexBuffer; //!< Member variable "vertexBuffer"

    private:
        void initPointersAndBuffers();
        void closePointersAndBuffers();
        void calculateModeTransf();
};

#endif // FIGURE_H
