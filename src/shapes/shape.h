#ifndef SHAPE_H
#define SHAPE_H

#include <functional>
#include <memory>
#include <GL/glew.h>
#include <QOpenGLWidget>
#include "utils/scenedata.h"

using GetTypeSignature = auto()->PrimitiveType;
// compute vertices using tessellation parameters
using UpdateVertexDataSignature = auto(int param1, int param2)->void;
using GetVertexDataSignature = auto()->std::shared_ptr<std::vector<GLfloat>>;

struct Shape {
    std::function<GetTypeSignature> getType;
    std::function<UpdateVertexDataSignature> updateVertexData;
    std::function<GetVertexDataSignature> getVertexData;

    GLuint vbo;
    GLuint vao;
    void initGLObjects(QOpenGLWidget* widget) {
        widget->makeCurrent();

        glGenBuffers(1, &vbo);
        glGenVertexArrays(1, &vao);

        widget->doneCurrent();
    };
    void bufferData(QOpenGLWidget* widget) {
        widget->makeCurrent();

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        std::vector<GLfloat> vertData = *getVertexData();
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertData.size(), vertData.data(), GL_STATIC_DRAW);

        glBindVertexArray(vao);
        // position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT),
                              reinterpret_cast<void*>(0));
        // normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT),
                              reinterpret_cast<void*>(3 * sizeof(GL_FLOAT)));

        // unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        widget->doneCurrent();
    };
    void deleteGLObjects(QOpenGLWidget* widget) {
        widget->makeCurrent();

        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);

        widget->doneCurrent();
    };
};


void insertVec3(std::shared_ptr<std::vector<GLfloat>> data, const glm::vec3& v);

glm::vec3 sphericalToCartesian(float phi, float theta);
glm::vec3 cylindricalToCartesian(float r, float theta, float y);

#endif // SHAPE_H
