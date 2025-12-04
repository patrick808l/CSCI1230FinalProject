#include "sphere.h"

static const int MIN_PARAM1 = 2;
static const int MIN_PARAM2 = 3;

Shape Sphere() {
    auto vertexData = std::make_shared<std::vector<GLfloat>>();
    auto m_param1 = std::make_shared<int>(0);
    auto m_param2 = std::make_shared<int>(0);

    MakeTileSignature makeTile = [=](const glm::vec3& topLeft,
                          const glm::vec3& topRight,
                          const glm::vec3& bottomLeft,
                          const glm::vec3& bottomRight) {
        // normals are normalize(2x, 2y, 2z) = normalize(x, y, z)
        insertVec3(vertexData, topLeft);
        insertVec3(vertexData, glm::normalize(topLeft));
        insertVec3(vertexData, bottomRight);
        insertVec3(vertexData, glm::normalize(bottomRight));
        insertVec3(vertexData, bottomLeft);
        insertVec3(vertexData, glm::normalize(bottomLeft));

        insertVec3(vertexData, topLeft);
        insertVec3(vertexData, glm::normalize(topLeft));
        insertVec3(vertexData, topRight);
        insertVec3(vertexData, glm::normalize(topRight));
        insertVec3(vertexData, bottomRight);
        insertVec3(vertexData, glm::normalize(bottomRight));
    };

    MakeWedgeSignature makeWedge = [=](float curTheta, float nextTheta) {
        float startPhi = 0.f;
        float phiStep = glm::radians(180.f / *m_param1);

        float r = 0.5f;

        // make wedges from the bottom of the sphere to the top by updating phi
        for (int t = 0; t < *m_param1; t++) {
            float currentPhi = startPhi + t * phiStep;
            float nextPhi = startPhi + (t + 1.f) * phiStep;

            glm::vec3 topLeft = sphericalToCartesian(nextPhi, curTheta);
            glm::vec3 topRight = sphericalToCartesian(nextPhi, nextTheta);
            glm::vec3 bottomLeft = sphericalToCartesian(currentPhi, curTheta);
            glm::vec3 bottomRight = sphericalToCartesian(currentPhi, nextTheta);

            makeTile(topLeft, topRight, bottomLeft, bottomRight);
        }
    };

    return Shape{
        .getType = []() {
            return PrimitiveType::PRIMITIVE_SPHERE;
        },

        .updateVertexData = [=](int param1, int param2) {
            *m_param1 = glm::max(param1, MIN_PARAM1);
            *m_param2 = glm::max(param2, MIN_PARAM2);
            vertexData->clear();

            float thetaStep = glm::radians(360.f / *m_param2);

            for (int t = 0; t < *m_param2; t++) {
                float currentTheta = (float)t * thetaStep;
                float nextTheta = (float)(t+1) * thetaStep;

                makeWedge(currentTheta, nextTheta);
            }
        },

        .getVertexData = [=]() {
            return vertexData;
        }
    };
}
