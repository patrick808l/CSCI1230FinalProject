#include "shape.h"

// common function implementations needed between shapes

/**
 * @brief helper function to insert all three fields of a vec3 into the given vector.
 */
void insertVec3(std::shared_ptr<std::vector<float>> data, const glm::vec3& v) {
    data->push_back(v.x);
    data->push_back(v.y);
    data->push_back(v.z);
}

void insertVec2(std::shared_ptr<std::vector<float>> data, glm::vec2& v) {
    data->push_back(v.x);
    data->push_back(v.y);
}

/**
 * @brief helper function to compute tangent vector, used for TBN matrix for texture.
 */
glm::vec3 computeTangent(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2,
                         glm::vec2 uv0, glm::vec2 uv1, glm::vec2 uv2){

    glm::vec3 edge1 = p1 - p0;
    glm::vec3 edge2 = p2 - p0;
    glm::vec2 deltaUV1 = uv1 - uv0;
    glm::vec2 deltaUV2 = uv2 - uv0;

    float tem = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
    if(fabs(tem) < 1e-6f) return glm::normalize(glm::vec3(1,0,0));
    float f = 1.0f/tem;

    glm::vec3 tangent;
    tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
    return glm::normalize(tangent);
}

/**
 * @brief helper function to convert spherical coordinates to cartesian (xyz)
 * @param phi is the spherical latitude in radians
 * @param theta is the spherical longitude in radians
 * @return a vector with x, y, and z
 */
glm::vec3 sphericalToCartesian(float phi, float theta) {
    float r = 0.5f;
    return glm::vec3{
        r * glm::sin(phi) * glm::cos(theta),
        r * glm::cos(phi),
        -r * glm::sin(phi) * glm::sin(theta)
    };
}

/**
 * @brief helper function to convert cylindrical coordinates to cartesian (xyz)
 * @param r is the radius (distance from the vertical axis)
 * @param theta is the cylindrical longitude in radians
 * @param y coordinate, same in both cylindrical and cartesian
 * @return a vector with x, y, and z
 */
glm::vec3 cylindricalToCartesian(float r, float theta, float y) {
    return glm::vec3{
        r * glm::cos(theta),
        y,
        r * glm::sin(theta)
    };
}
