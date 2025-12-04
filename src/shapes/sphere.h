#ifndef SPHERE_H
#define SPHERE_H

#include "shape.h"

using MakeTileSignature = std::function<void(const glm::vec3& topLeft,
                                             const glm::vec3& topRight,
                                             const glm::vec3& bottomLeft,
                                             const glm::vec3& bottomRight)>;

using MakeWedgeSignature = std::function<void(float curTheta, float nextTheta)>;

Shape Sphere();

#endif // SPHERE_H
