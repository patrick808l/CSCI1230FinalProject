#include "cube.h"

static const int MIN_PARAM1 = 1;

Shape Cube() {
    auto vertexData = std::make_shared<std::vector<GLfloat>>();
    auto m_param1 = std::make_shared<int>(0);

    MakeTileSignature makeTile = [=](const glm::vec3& topLeft,
                                     const glm::vec3& topRight,
                                     const glm::vec3& bottomLeft,
                                     const glm::vec3& bottomRight) {
        glm::vec3 normal1 = glm::normalize(glm::cross(bottomRight - bottomLeft, topLeft - bottomLeft));

        insertVec3(vertexData, topLeft);
        insertVec3(vertexData, normal1);
        insertVec3(vertexData, bottomLeft);
        insertVec3(vertexData, normal1);
        insertVec3(vertexData, bottomRight);
        insertVec3(vertexData, normal1);

        glm::vec3 normal2 = glm::normalize(glm::cross(topLeft - topRight, bottomRight - topRight));

        insertVec3(vertexData, topLeft);
        insertVec3(vertexData, normal2);
        insertVec3(vertexData, bottomRight);
        insertVec3(vertexData, normal2);
        insertVec3(vertexData, topRight);
        insertVec3(vertexData, normal2);
    };

    MakeFaceSignature makeFace = [=](const glm::vec3& topLeft,
                                     const glm::vec3& topRight,
                                     const glm::vec3& bottomLeft,
                                     const glm::vec3& bottomRight) {
        for (int i = 0; i < *m_param1; i++) {
            for (int j = 0; j < *m_param1; j++) {
                // to get corners of tiles, weight in right and down directions by j and i
                glm::vec3 tl = topLeft + ((float)j * (topRight - topLeft) / (float)*m_param1) + ((float)i * (bottomLeft - topLeft) / (float)*m_param1);
                glm::vec3 tr = topLeft + ((float)(j+1) * (topRight - topLeft) / (float)*m_param1) + ((float)i * (bottomLeft - topLeft) / (float)*m_param1);
                glm::vec3 bl = topLeft + ((float)j * (topRight - topLeft) / (float)*m_param1) + ((float)(i+1) * (bottomLeft - topLeft) / (float)*m_param1);
                glm::vec3 br = topLeft + ((float)(j+1) * (topRight - topLeft) / (float)*m_param1) + ((float)(i+1) * (bottomLeft - topLeft) / (float)*m_param1);
                makeTile(tl, tr, bl, br);
            }
        }
    };

    return Shape{
        .getType = []() {
            return PrimitiveType::PRIMITIVE_CUBE;
        },

        .updateVertexData = [=](int param1, int param2) {
            *m_param1 = glm::max(param1, MIN_PARAM1);
            // param2 unused
            vertexData->clear();

            glm::vec3 frontTopLeft = glm::vec3{0.5f, -0.5f, 0.5f};
            glm::vec3 frontTopRight = glm::vec3{0.5f, 0.5f, 0.5f};
            glm::vec3 frontBottomRight = glm::vec3{0.5f, 0.5f, -0.5f};
            glm::vec3 frontBottomLeft = glm::vec3{0.5f, -0.5f, -0.5f};

            glm::vec3 backTopLeft = glm::vec3{-0.5f, -0.5f, 0.5f};
            glm::vec3 backTopRight = glm::vec3{-0.5f, 0.5f, 0.5f};
            glm::vec3 backBottomRight = glm::vec3{-0.5f, 0.5f, -0.5f};
            glm::vec3 backBottomLeft = glm::vec3{-0.5f, -0.5f, -0.5f};

            // front face (+x)
            makeFace(frontTopLeft, frontTopRight, frontBottomLeft, frontBottomRight);

            // back face (-x)
            makeFace(backTopRight, backTopLeft, backBottomRight, backBottomLeft);

            // top face (+z)
            makeFace(backTopLeft, backTopRight, frontTopLeft, frontTopRight);

            // bottom face (-z)
            makeFace(backBottomRight, backBottomLeft, frontBottomRight, frontBottomLeft);

            // right face (+y)
            makeFace(frontTopRight, backTopRight, frontBottomRight, backBottomRight);

            // left face (-y)
            makeFace(backTopLeft, frontTopLeft, backBottomLeft, frontBottomLeft);
        },

        .getVertexData = [=]() {
            return vertexData;
        }
    };
}
