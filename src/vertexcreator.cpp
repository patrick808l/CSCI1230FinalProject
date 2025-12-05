#include "vertexcreator.h"

#include <iostream>
#include <QOpenGLWidget>

vertexCreator::vertexCreator() {}

/**
 * @brief Custom helpers for final project gear up
 */
GLuint vertexCreator::createOpenGLTexture(const std::string &filename) {
    std::string fileStr = filename; //":/resources/" +
    QString filepath = QString::fromStdString(fileStr);
    std::cout << fileStr << std::endl;

    QImage img(filepath);
    if (img.isNull()){
        std::cerr << "error loading Qimage" << std::endl;
    }
    img = img.convertToFormat(QImage::Format_RGBA8888).mirrored();

    GLuint textID;
    glGenTextures(1, &textID);
    glBindTexture(GL_TEXTURE_2D, textID);

    // Load image into texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());

    // Task 6: Set min and mag filters' interpolation mode to linear
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
    return textID;
}

void vertexCreator::createTexture(std::unordered_map<std::string, GLuint>& m_textures,
                                  std::vector<RenderShapeData>& shapes){

    for (auto &text : m_textures) {
        if(text.second) glDeleteTextures(1, &text.second);
    }
    m_textures.clear();

    for (const auto& shape : shapes) {
        const SceneMaterial& shapeMat = shape.primitive.material;

        if (shapeMat.textureMap.isUsed) {
            const std::string& filename = shapeMat.textureMap.filename;
            if (!m_textures.count(filename)) {
                m_textures[filename] = createOpenGLTexture(filename);
            }
        }
    }
}

void vertexCreator::createNormalText(std::unordered_map<std::string, GLuint>& m_normalTextures,
                             std::vector<RenderShapeData>& shapes){

    for (auto &text : m_normalTextures) {
        if(text.second) glDeleteTextures(1, &text.second);
    }
    m_normalTextures.clear();

    for (const auto& shape : shapes) {
        const SceneMaterial& shapeMat = shape.primitive.material;

        if (shapeMat.normalMap.isUsed) {
            const std::string& filename = shapeMat.normalMap.filename;
            if (!m_normalTextures.count(filename)) {
                m_normalTextures[filename] = createOpenGLTexture(filename);
            }
        }
    }
}

void vertexCreator::createBumpText(std::unordered_map<std::string, GLuint>& m_bumpTextures,
                           std::vector<RenderShapeData>& shapes){

    for (auto &text : m_bumpTextures) {
        if(text.second) glDeleteTextures(1, &text.second);
    }
    m_bumpTextures.clear();

    for (const auto& shape : shapes) {
        const SceneMaterial& shapeMat = shape.primitive.material;

        if (shapeMat.bumpMap.isUsed) {
            const std::string& filename = shapeMat.bumpMap.filename;
            if (!m_bumpTextures.count(filename)) {
                m_bumpTextures[filename] = createOpenGLTexture(filename);
            }
        }
    }
}
