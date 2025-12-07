#ifndef CYLINDER_H
#define CYLINDER_H

#include "shape.h"

using MakeCylCapTileSignature = std::function<void(const glm::vec3& topLeft,
                                             const glm::vec3& topRight,
                                             const glm::vec3& bottomLeft,
                                             const glm::vec3& bottomRight)>;

using MakeCylCapSliceSignature = std::function<void(float currentTheta, float nextTheta, bool isTopCap)>;

using MakeWallTileSignature = std::function<void(const glm::vec3& topLeft,
                                                 const glm::vec3& topRight,
                                                 const glm::vec3& bottomLeft,
                                                 const glm::vec3& bottomRight)>;

using MakeWallSliceSignature = std::function<void(float currentTheta, float nextTheta)>;

using MakeWedgeSignature = std::function<void(float currentTheta, float nextTheta)>;

Shape Cylinder();

glm::vec2 computeCylinderUV(glm::vec3 ptObjSpace, glm::vec3 n);

#endif // CYLINDER_H
