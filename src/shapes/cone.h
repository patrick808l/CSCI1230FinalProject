#ifndef CONE_H
#define CONE_H

#include "shape.h"

using MakeCapTileSignature = std::function<void(const glm::vec3& topLeft,
                                             const glm::vec3& topRight,
                                             const glm::vec3& bottomLeft,
                                             const glm::vec3& bottomRight)>;

using MakeCapSliceSignature = std::function<void(float currentTheta, float nextTheta)>;

using CalcNormSignature = std::function<glm::vec3(const glm::vec3& pt)>;

using MakeSlopeTileSignature = std::function<void(const glm::vec3& topLeft,
                                                  const glm::vec3& topRight,
                                                  const glm::vec3& bottomLeft,
                                                  const glm::vec3& bottomRight,
                                                  bool atTip)>;

using GetRadiusSignature = std::function<float(float y)>;

using MakeSlopeSliceSignature = std::function<void(float currentTheta, float nextTheta)>;

using MakeWedgeSignature = std::function<void(float currentTheta, float nextTheta)>;

Shape Cone();

glm::vec2 computeConeUV(glm::vec3 ptObjSpace, glm::vec3 n);

#endif // CONE_H
