#include "cylinder.h"

static const int MIN_PARAM1 = 1;
static const int MIN_PARAM2 = 3;

Shape Cylinder() {
    auto vertexData = std::make_shared<std::vector<GLfloat>>();
    auto m_param1 = std::make_shared<int>(0);
    auto m_param2 = std::make_shared<int>(0);
    float m_radius = 0.5f;

    MakeCylCapTileSignature makeCapTile = [=](const glm::vec3& topLeft,
                                           const glm::vec3& topRight,
                                           const glm::vec3& bottomLeft,
                                           const glm::vec3& bottomRight) {
        glm::vec3 normal1 = -glm::normalize(glm::cross(bottomRight - bottomLeft, topLeft - bottomLeft));

        glm::vec3 n(0.f, -1.f, 0.f);
        if(normal1.y > 0.0) n = glm::vec3(0.f, 1.f, 0.f);

        glm::vec2 uvTL = computeCylinderUV(topLeft, n);
        glm::vec2 uvBL = computeCylinderUV(bottomLeft, n);
        glm::vec2 uvBR = computeCylinderUV(bottomRight, n);
        glm::vec2 uvTR = computeCylinderUV(topRight, n);

        glm::vec3 tan1 = computeTangent(topLeft, bottomRight, bottomLeft,
                                        uvTL, uvBR, uvBL);
        glm::vec3 tan2 = computeTangent(topLeft, topRight, bottomRight,
                                        uvTL, uvTR, uvBR);

        insertVec3(vertexData, topLeft);
        insertVec3(vertexData, normal1);
        insertVec2(vertexData, uvTL);
        insertVec3(vertexData, tan1);

        insertVec3(vertexData, bottomRight);
        insertVec3(vertexData, normal1);
        insertVec2(vertexData, uvBR);
        insertVec3(vertexData, tan1);

        insertVec3(vertexData, bottomLeft);
        insertVec3(vertexData, normal1);
        insertVec2(vertexData, uvBL);
        insertVec3(vertexData, tan1);

        glm::vec3 normal2 = -glm::normalize(glm::cross(topLeft - topRight, bottomRight - topRight));

        insertVec3(vertexData, topLeft);
        insertVec3(vertexData, normal2);
        insertVec2(vertexData, uvTL);
        insertVec3(vertexData, tan2);

        insertVec3(vertexData, topRight);
        insertVec3(vertexData, normal2);
        insertVec2(vertexData, uvTR);
        insertVec3(vertexData, tan2);

        insertVec3(vertexData, bottomRight);
        insertVec3(vertexData, normal2);
        insertVec2(vertexData, uvBR);
        insertVec3(vertexData, tan2);
    };

    MakeCylCapSliceSignature makeCapSlice = [=](float currentTheta, float nextTheta, bool isTopCap) {
        float rStep = m_radius / *m_param1;

        for (int t = 0; t < *m_param1; t++) {
            float currentR = t * rStep;
            float nextR = (t+1) * rStep;

            float y;
            if (isTopCap) {
                y = 0.5;
            } else {
                y = -0.5;
            }
            glm::vec3 topLeft = cylindricalToCartesian(nextR, currentTheta, y);
            glm::vec3 topRight = cylindricalToCartesian(nextR, nextTheta, y);
            glm::vec3 bottomLeft = cylindricalToCartesian(currentR, currentTheta, y);
            glm::vec3 bottomRight = cylindricalToCartesian(currentR, nextTheta, y);

            if (isTopCap) {
                makeCapTile(topRight, topLeft, bottomRight, bottomLeft);
            } else {
                makeCapTile(topLeft, topRight, bottomLeft, bottomRight);
            }
        }
    };

    MakeWallTileSignature makeWallTile = [=](const glm::vec3& topLeft,
                                             const glm::vec3& topRight,
                                             const glm::vec3& bottomLeft,
                                             const glm::vec3& bottomRight) {
        glm::vec3 leftNorm = glm::normalize(glm::vec3{topLeft.x, 0, topLeft.z});
        glm::vec3 rightNorm = glm::normalize(glm::vec3{topRight.x, 0, topRight.z});

        glm::vec2 uvTL = computeCylinderUV(topLeft, leftNorm);
        glm::vec2 uvBL = computeCylinderUV(bottomLeft, leftNorm);
        glm::vec2 uvBR = computeCylinderUV(bottomRight, rightNorm);
        glm::vec2 uvTR = computeCylinderUV(topRight, rightNorm);

        glm::vec3 tan1 = computeTangent(topLeft, bottomRight, bottomLeft,
                                        uvTL, uvBR, uvBL);
        glm::vec3 tan2 = computeTangent(topLeft, topRight, bottomRight,
                                        uvTL, uvTR, uvBR);

        insertVec3(vertexData, topLeft);
        insertVec3(vertexData, leftNorm);
        insertVec2(vertexData, uvTL);
        insertVec3(vertexData, tan1);

        insertVec3(vertexData, bottomRight);
        insertVec3(vertexData, rightNorm);
        insertVec2(vertexData, uvBR);
        insertVec3(vertexData, tan1);

        insertVec3(vertexData, bottomLeft);
        insertVec3(vertexData, leftNorm);
        insertVec2(vertexData, uvBL);
        insertVec3(vertexData, tan1);

        insertVec3(vertexData, topLeft);
        insertVec3(vertexData, leftNorm);
        insertVec2(vertexData, uvTL);
        insertVec3(vertexData, tan2);

        insertVec3(vertexData, topRight);
        insertVec3(vertexData, rightNorm);
        insertVec2(vertexData, uvTR);
        insertVec3(vertexData, tan2);

        insertVec3(vertexData, bottomRight);
        insertVec3(vertexData, rightNorm);
        insertVec2(vertexData, uvBR);
        insertVec3(vertexData, tan2);
    };

    MakeWallSliceSignature makeWallSlice = [=](float currentTheta, float nextTheta) {
        float startY = -0.5f;
        float yStep = 1.f / *m_param1;

        for (int t = 0; t < *m_param1; t++) {
            float currentY = startY + t * yStep;
            float nextY = startY + (t+1) * yStep;

            glm::vec3 topLeft = cylindricalToCartesian(0.5, currentTheta, nextY);
            glm::vec3 topRight = cylindricalToCartesian(0.5, nextTheta, nextY);
            glm::vec3 bottomLeft = cylindricalToCartesian(0.5, currentTheta, currentY);
            glm::vec3 bottomRight = cylindricalToCartesian(0.5, nextTheta, currentY);

            makeWallTile(topLeft, topRight, bottomLeft, bottomRight);
        }
    };

    MakeWedgeSignature makeWedge = [=](float currentTheta, float nextTheta) {
        makeCapSlice(currentTheta, nextTheta, true);
        makeCapSlice(currentTheta, nextTheta, false);
        makeWallSlice(currentTheta, nextTheta);
    };

    return Shape{
        .getType = []() {
            return PrimitiveType::PRIMITIVE_CYLINDER;
        },

        .updateVertexData = [=](int param1, int param2) {
            *m_param1 = glm::max(param1, MIN_PARAM1);
            *m_param2 = glm::max(param2, MIN_PARAM2);
            vertexData->clear();

            float thetaStep = glm::radians(360.f / *m_param2);

            for (int t = 0; t < *m_param2; t++) {
                float currentTheta = t * thetaStep;
                float nextTheta = (t+1) * thetaStep;

                makeWedge(currentTheta, nextTheta);
            }
        },

        .getVertexData = [=]() {
            return vertexData;
        }
    };
}

glm::vec2 computeCylinderUV(glm::vec3 ptObjSpace, glm::vec3 n){
    // top cap
    if(n.y == 1.0f){
        float u = ptObjSpace.x + 0.5f;
        float v = 0.5f - ptObjSpace.z;
        return glm::vec2(u, v);
    }
    // bot cap
    else if(n.y == -1.0f){
        float u = ptObjSpace.x + 0.5f;
        float v = ptObjSpace.z + 0.5f;
        return glm::vec2(u, v);
    }

    // body
    float v = ptObjSpace.y + 0.5f;

    float theta = atan2(ptObjSpace.z, ptObjSpace.x);
    float u;
    if(theta < 0.f)
        u = -theta / (2.f * M_PI);
    else
        u = 1.f - (theta / (2.f * M_PI));

    return glm::vec2(u, v);
}

