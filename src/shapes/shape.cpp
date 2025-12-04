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
