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
glm::vec2 computeCubeUV(glm::vec3 ptObjSpace, glm::vec3 n);

#endif // CUBE_H
