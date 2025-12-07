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

        glm::vec2 uvTL = computeSphereUV(topLeft);
        glm::vec2 uvBL = computeSphereUV(bottomLeft);
        glm::vec2 uvBR = computeSphereUV(bottomRight);
        glm::vec2 uvTR = computeSphereUV(topRight);

        glm::vec3 tan1 = computeTangent(topLeft, bottomRight, bottomLeft,
                                        uvTL, uvBR, uvBL);
        glm::vec3 tan2 = computeTangent(topLeft, topRight, bottomRight,
                                        uvTL, uvTR, uvBR);

        checksphereSeamU(uvTL, uvTR, uvBL, uvBR);

        // normals are normalize(2x, 2y, 2z) = normalize(x, y, z)
        insertVec3(vertexData, topLeft);
        insertVec3(vertexData, glm::normalize(topLeft));
        insertVec2(vertexData, uvTL);
        insertVec3(vertexData, tan1);

        insertVec3(vertexData, bottomRight);
        insertVec3(vertexData, glm::normalize(bottomRight));
        insertVec2(vertexData, uvBR);
        insertVec3(vertexData, tan1);

        insertVec3(vertexData, bottomLeft);
        insertVec3(vertexData, glm::normalize(bottomLeft));
        insertVec2(vertexData, uvBL);
        insertVec3(vertexData, tan1);


        insertVec3(vertexData, topLeft);
        insertVec3(vertexData, glm::normalize(topLeft));
        insertVec2(vertexData, uvTL);
        insertVec3(vertexData, tan2);

        insertVec3(vertexData, topRight);
        insertVec3(vertexData, glm::normalize(topRight));
        insertVec2(vertexData, uvTR);
        insertVec3(vertexData, tan2);

        insertVec3(vertexData, bottomRight);
        insertVec3(vertexData, glm::normalize(bottomRight));
        insertVec2(vertexData, uvBR);
        insertVec3(vertexData, tan2);
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

glm::vec2 computeSphereUV(glm::vec3 ptObjSpace){
    float theta = atan2(ptObjSpace.z, ptObjSpace.x);
    float u;

    // atan2 give theta between -PI to PI, not continuous
    if(theta < 0.f)
        u = -theta / (2.f * M_PI);
    else
        u = 1.f - (theta / (2.f * M_PI));

    // asin give between -PI/2 to PI/2
    float thetaLatitude = asin(glm::clamp((ptObjSpace.y / 0.5f), -1.f, 1.f));
    float v = (thetaLatitude / M_PI) + 0.5f;

    return glm::vec2(u, v);
}

void checksphereSeamU(glm::vec2 &uvTL, glm::vec2 &uvTR,
                              glm::vec2 &uvBL, glm::vec2 &uvBR){

    float minU = std::min(std::min(uvTL.x, uvTR.x), std::min(uvBL.x, uvBR.x));
    float maxU = std::max(std::max(uvTL.x, uvTR.x), std::max(uvBL.x, uvBR.x));

    if(maxU - minU > 0.5f){
        if (uvTL.x < 0.5f) uvTL.x += 1.0f;
        if (uvTR.x < 0.5f) uvTR.x += 1.0f;
        if (uvBL.x < 0.5f) uvBL.x += 1.0f;
        if (uvBR.x < 0.5f) uvBR.x += 1.0f;
    }
}
