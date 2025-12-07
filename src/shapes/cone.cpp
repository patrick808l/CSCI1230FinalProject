#include "cone.h"

static const int MIN_PARAM1 = 1;
static const int MIN_PARAM2 = 3;

Shape Cone() {
    auto vertexData = std::make_shared<std::vector<GLfloat>>();
    auto m_param1 = std::make_shared<int>(0);
    auto m_param2 = std::make_shared<int>(0);

    MakeCapTileSignature makeCapTile = [=](const glm::vec3& topLeft,
                                           const glm::vec3& topRight,
                                           const glm::vec3& bottomLeft,
                                           const glm::vec3& bottomRight) {
        glm::vec3 n1(0.f, -1.f, 0.f);
        glm::vec2 uvTL = computeConeUV(topLeft, n1);
        glm::vec2 uvBL = computeConeUV(bottomLeft, n1);
        glm::vec2 uvBR = computeConeUV(bottomRight, n1);
        glm::vec2 uvTR = computeConeUV(topRight, n1);

        glm::vec3 tan1 = computeTangent(topLeft, bottomRight, bottomLeft,
                                        uvTL, uvBR, uvBL);
        glm::vec3 tan2 = computeTangent(topLeft, topRight, bottomRight,
                                        uvTL, uvTR, uvBR);

        glm::vec3 normal1 = -glm::normalize(glm::cross(bottomRight - bottomLeft, topLeft - bottomLeft));

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

    MakeCapSliceSignature makeCapSlice = [=](float currentTheta, float nextTheta) {
        float rStep = 0.5f / *m_param1;

        for (int t = 0; t < *m_param1; t++) {
            float currentR = t * rStep;
            float nextR = (t+1) * rStep;

            glm::vec3 topLeft = cylindricalToCartesian(nextR, currentTheta, -0.5);
            glm::vec3 topRight = cylindricalToCartesian(nextR, nextTheta, -0.5);
            glm::vec3 bottomLeft = cylindricalToCartesian(currentR, currentTheta, -0.5);
            glm::vec3 bottomRight = cylindricalToCartesian(currentR, nextTheta, -0.5);

            makeCapTile(topLeft, topRight, bottomLeft, bottomRight);
        }
    };

    CalcNormSignature calcNorm = [=](const glm::vec3& pt) {
        float xNorm = (2 * pt.x);
        float yNorm = -(1.f/4.f) * (2.f * pt.y - 1.f);
        float zNorm = (2 * pt.z);

        return glm::normalize(glm::vec3{ xNorm, yNorm, zNorm });
    };

    MakeSlopeTileSignature makeSlopeTile = [=](const glm::vec3& topLeft,
                                               const glm::vec3& topRight,
                                               const glm::vec3& bottomLeft,
                                               const glm::vec3& bottomRight,
                                               bool atTip) {
        glm::vec3 bottomLeftNorm = calcNorm(bottomLeft);
        glm::vec3 bottomRightNorm = calcNorm(bottomRight);
        glm::vec3 topLeftNorm;
        glm::vec3 topRightNorm;
        if (atTip) {
            // topLeft == topRight at tip
            topLeftNorm = glm::normalize(bottomLeftNorm + bottomRightNorm);
            topRightNorm = topLeftNorm;
        } else {
            topLeftNorm = calcNorm(topLeft);
            topRightNorm = calcNorm(topRight);
        }

        glm::vec2 uvTL = computeConeUV(topLeft, topLeftNorm);
        glm::vec2 uvBL = computeConeUV(bottomLeft, bottomLeftNorm);
        glm::vec2 uvBR = computeConeUV(bottomRight, bottomRightNorm);
        glm::vec2 uvTR = computeConeUV(topRight, topRightNorm);

        glm::vec3 tan1 = computeTangent(topLeft, bottomRight, bottomLeft,
                                        uvTL, uvBR, uvBL);
        glm::vec3 tan2 = computeTangent(topLeft, topRightNorm, bottomRight,
                                        uvTL, uvTR, uvBR);

        insertVec3(vertexData, topLeft);
        insertVec3(vertexData, topLeftNorm);
        insertVec2(vertexData, uvTL);
        insertVec3(vertexData, tan1);

        insertVec3(vertexData, bottomRight);
        insertVec3(vertexData, bottomRightNorm);
        insertVec2(vertexData, uvBR);
        insertVec3(vertexData, tan1);

        insertVec3(vertexData, bottomLeft);
        insertVec3(vertexData, bottomLeftNorm);
        insertVec2(vertexData, uvBL);
        insertVec3(vertexData, tan1);


        insertVec3(vertexData, topLeft);
        insertVec3(vertexData, topLeftNorm);
        insertVec2(vertexData, uvTL);
        insertVec3(vertexData, tan2);

        insertVec3(vertexData, topRight);
        insertVec3(vertexData, topRightNorm);
        insertVec2(vertexData, uvTR);
        insertVec3(vertexData, tan2);

        insertVec3(vertexData, bottomRight);
        insertVec3(vertexData, bottomRightNorm);
        insertVec2(vertexData, uvBR);
        insertVec3(vertexData, tan2);
    };

    GetRadiusSignature getRadius = [=](float y) {
        return (0.5f - y) / 2.f;
    };

    MakeSlopeSliceSignature makeSlopeSlice = [=](float currentTheta, float nextTheta) {
        float startY = -0.5f;
        float yStep = 1.f / *m_param1;

        for (int t = 0; t < *m_param1; t++) {
            float currentY = startY + t * yStep;
            float nextY = startY + (t+1) * yStep;

            bool atTip = (t == *m_param1 - 1);
            glm::vec3 topLeft = cylindricalToCartesian(getRadius(nextY), currentTheta, nextY);
            glm::vec3 topRight = cylindricalToCartesian(getRadius(nextY), nextTheta, nextY);
            glm::vec3 bottomLeft = cylindricalToCartesian(getRadius(currentY), currentTheta, currentY);
            glm::vec3 bottomRight = cylindricalToCartesian(getRadius(currentY), nextTheta, currentY);

            makeSlopeTile(topLeft, topRight, bottomLeft, bottomRight, atTip);
        }
    };

    MakeWedgeSignature makeWedge = [=](float currentTheta, float nextTheta) {
        makeCapSlice(currentTheta, nextTheta);
        makeSlopeSlice(currentTheta, nextTheta);
    };

    return Shape{
        .getType = []() {
            return PrimitiveType::PRIMITIVE_CONE;
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

glm::vec2 computeConeUV(glm::vec3 ptObjSpace, glm::vec3 n){
    float margin = 0.00001f;

    // bottom flat surface
    if(n.y == -1.0f){
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
