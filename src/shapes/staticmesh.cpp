#include <iostream>
#include "staticmesh.h"
#include "utils/objfilereader.h"

Shape StaticMesh(std::string meshfile) {
    auto vertexData = std::make_shared<std::vector<GLfloat>>();
    auto parsed = std::make_shared<bool>(false);

    return Shape{
        .getType = []() {
            return PrimitiveType::PRIMITIVE_MESH;
        },

        .updateVertexData = [=](int param1, int param2) {
            if (!*parsed) {
                if (readAndParseFile(meshfile, vertexData)) {
                    std::cout << "successfully parsed meshfile: " << meshfile << std::endl;
                    *parsed = true;
                } else {
                    std::cout << "failed to parse meshfile: " << meshfile << std::endl;
                }
            }
        },

        .getVertexData = [=]() {
            return vertexData;
        },

        .Ibody = [=](double mass) {
            // (represents all meshes as spheres with regards to rotation)
            double I = (mass * 0.25) * (2.0 / 5.0);
            return glm::mat3(
                I, 0, 0,
                0, I, 0,
                0, 0, I
            );
        }
    };
}
