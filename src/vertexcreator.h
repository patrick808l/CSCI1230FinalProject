#ifndef VERTEXCREATOR_H
#define VERTEXCREATOR_H

#include "GL/glew.h" // Must always be first include
#include <utils/scenedata.h>
#include <utils/sceneparser.h>
#include <unordered_map>

class vertexCreator
{
public:
    vertexCreator();

    // final project gear up
    // Helper function to create openGL texture based on filename and return its id
    static GLuint createOpenGLTexture(const std::string &filename);

    // Create openGL textures for all the shapes and store their id to hash
    static void createTexture(std::unordered_map<std::string, GLuint>& m_textures,
                              std::vector<RenderShapeData>& shapes);

    static void createNormalText(std::unordered_map<std::string, GLuint>& m_normalTextures,
                              std::vector<RenderShapeData>& shapes);

    static void createBumpText(std::unordered_map<std::string, GLuint>& m_bumpTextures,
                                 std::vector<RenderShapeData>& shapes);
};

#endif // VERTEXCREATOR_H
