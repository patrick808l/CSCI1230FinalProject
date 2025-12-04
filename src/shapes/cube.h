#ifndef CUBE_H
#define CUBE_H

#include "shape.h"

using MakeTileSignature = std::function<void(const glm::vec3& topLeft,
                                             const glm::vec3& topRight,
                                             const glm::vec3& bottomLeft,
                                             const glm::vec3& bottomRight)>;

using MakeFaceSignature = std::function<void(const glm::vec3& topLeft,
                                             const glm::vec3& topRight,
                                             const glm::vec3& bottomLeft,
                                             const glm::vec3& bottomRight)>;

Shape Cube();

#endif // CUBE_H
