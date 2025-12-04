#include <iostream>
#include "mesh.h"
#include "utils/objfilereader.h"

Shape Mesh(std::string meshfile) {
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
        }
    };
}
