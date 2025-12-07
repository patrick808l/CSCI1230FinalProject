#ifndef SPHERE_H
#define SPHERE_H

#include "shape.h"

using MakeTileSignature = std::function<void(const glm::vec3& topLeft,
                                             const glm::vec3& topRight,
                                             const glm::vec3& bottomLeft,
                                             const glm::vec3& bottomRight)>;

using MakeWedgeSignature = std::function<void(float curTheta, float nextTheta)>;

glm::vec2 computeSphereUV(glm::vec3 pt);
void checksphereSeamU(glm::vec2 &uvTL, glm::vec2 &uvTR, glm::vec2 &uvBL, glm::vec2 &uvBR);

Shape Sphere();

#endif // SPHERE_H
