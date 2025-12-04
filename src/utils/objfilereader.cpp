#include "objfilereader.h"
#include <glm/glm.hpp>
#include <qdir.h>
#include <unordered_map>
#include <iostream>

/**
 * @brief push a vertex and a normal into vertexData in vbo format
 *      (pos x, pos y, pos z, normal x, normal y, normal z)
 * @param vertData is a Mesh's vertex data to push into
 * @param vertex is a position in object space
 * @param normal is a normal in object space
 */
void pushVertexData(std::shared_ptr<std::vector<GLfloat>> vertData, const glm::vec3& vertex, const glm::vec3& normal) {
    vertData->push_back(vertex.x);
    vertData->push_back(vertex.y);
    vertData->push_back(vertex.z);

    vertData->push_back(normal.x);
    vertData->push_back(normal.y);
    vertData->push_back(normal.z);
}

/**
 * @brief parse 3 floats from the given line. push them as a vec3 into the dest vector.
 * @param line is a trimmed line from the obj file
 * @param dest is the destination vector which holds vec3's of parsed floats
 */
void parse3Floats(QString line, std::vector<glm::vec3>& dest) {
    QStringList stringList = line.split(' ', Qt::SkipEmptyParts);
    bool ok_x, ok_y, ok_z;
    ok_x = ok_y = ok_z = false;
    float x, y, z;
    x = stringList.at(1).toFloat(&ok_x);
    y = stringList.at(2).toFloat(&ok_y);
    z = stringList.at(3).toFloat(&ok_z);
    if (ok_x && ok_y && ok_z) {
        dest.push_back(glm::vec3{x, y, z});
    } else {
        std::cout << "error parsing float from line: " << line.toStdString() << std::endl;
    }
}

/**
 * @brief read in a meshfile and parse vertex data into the given vector pointer.
 * @param vertData, pointer to a vector in which to place the vertex data.
 *      vertData consists of interweaved vertices (vec3) and normals (vec3).
 * @return 1 for success, 0 for failure
 */
int readAndParseFile(std::string meshfile, std::shared_ptr<std::vector<GLfloat>> vertData) {
    vertData->clear();

    bool meshfileIncludesNormals = true;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> vertexNormals;

    // maps vertex index (in vbo) to object index
    std::vector<int> objIndices;
    // map 1-index of vertex (int) to normals of faces that neighbor vertex
    std::unordered_map<int, std::vector<glm::vec3>> faceNormalMap;

    QFile file(meshfile.c_str());
    if (!file.open(QFile::ReadOnly)) {
        std::cout << "could not open " << meshfile << std::endl;
        return 0;
    }

    /*
     * parse through file line by line
     * if doesn't include vertex normals, must compute them ourselves
     */
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.length() == 0) continue;
        QStringList stringList = line.split(' ', Qt::SkipEmptyParts);

        if (stringList.at(0) == "v") {
            parse3Floats(line, vertices);
        } else if (stringList.at(0) == "vn") {
            parse3Floats(line, vertexNormals);
        } else if (stringList.at(0) == "f") {
            // if we encounter an 'f' line before seeing any normals, the meshfile does not include vertex normals
            if (vertexNormals.size() == 0) {
                meshfileIncludesNormals = false;
            }
            // number of coordinates in the face line. could be 4, 5, ... . 4 implies f and 3 coordinates.
            int numCoords = stringList.size();

            // for more than 3 coordinates per face line, construct triangles from 123, 134, ...
            QStringList coord1List = stringList.at(1).split("/");
            QStringList coord2List;
            QStringList coord3List;
            for (int coord2 = 2; coord2 <= numCoords - 2; coord2++) {
                coord2List = stringList.at(coord2).split("/");
                coord3List = stringList.at(coord2 + 1).split("/");

                bool ok_v1i, ok_v2i, ok_v3i, ok_n1i, ok_n2i, ok_n3i;
                ok_v1i = ok_v2i = ok_v3i = ok_n1i = ok_n2i = ok_n3i = false;
                int v1i, v2i, v3i, n1i, n2i, n3i;
                // parse vertex indices
                v1i = coord1List.at(0).toInt(&ok_v1i);
                v2i = coord2List.at(0).toInt(&ok_v2i);
                v3i = coord3List.at(0).toInt(&ok_v3i);

                if (meshfileIncludesNormals) {
                    // parse vertex normal indices
                    n1i = coord1List.at(2).toInt(&ok_n1i);
                    n2i = coord2List.at(2).toInt(&ok_n2i);
                    n3i = coord3List.at(2).toInt(&ok_n3i);
                    if (ok_v1i && ok_v2i && ok_v3i && ok_n1i && ok_n2i && ok_n3i) {
                        pushVertexData(vertData, vertices[v1i - 1], vertexNormals[n1i - 1]);
                        pushVertexData(vertData, vertices[v2i - 1], vertexNormals[n2i - 1]);
                        pushVertexData(vertData, vertices[v3i - 1], vertexNormals[n3i - 1]);
                    } else {
                        std::cout << "error parsing int from line: " << line.toStdString() << std::endl;
                    }
                } else {
                    if (ok_v1i && ok_v2i && ok_v3i) {
                        // set normals to zero for now, compute them later
                        pushVertexData(vertData, vertices[v1i - 1], glm::vec3{0,0,0});
                        pushVertexData(vertData, vertices[v2i - 1], glm::vec3{0,0,0});
                        pushVertexData(vertData, vertices[v3i - 1], glm::vec3{0,0,0});
                        objIndices.push_back(v1i);
                        objIndices.push_back(v2i);
                        objIndices.push_back(v3i);

                        if (!meshfileIncludesNormals) {
                            if (!faceNormalMap.contains(v1i)) {
                                faceNormalMap[v1i] = std::vector<glm::vec3>();
                            }
                            if (!faceNormalMap.contains(v2i)) {
                                faceNormalMap[v2i] = std::vector<glm::vec3>();
                            }
                            if (!faceNormalMap.contains(v3i)) {
                                faceNormalMap[v3i] = std::vector<glm::vec3>();
                            }

                            // compute face normal via cross product of vertices at given indices
                            glm::vec3 cross = glm::cross(vertices[v2i - 1] - vertices[v1i - 1], vertices[v3i - 1] - vertices[v1i - 1]);
                            // store face normal (unnormalized) in faceNormalMap
                            faceNormalMap[v1i].push_back(cross);
                            faceNormalMap[v2i].push_back(cross);
                            faceNormalMap[v3i].push_back(cross);
                        }
                    } else {
                        std::cout << "error parsing int from line: " << line.toStdString() << std::endl;
                    }
                }
            }
        }
        // ignore lines with any other headers (e.g. #)
    }

    if (!meshfileIncludesNormals) {
        // compute vertex normals for each vertex using the neighboring faces
        for (int vertIndex = 1; vertIndex <= vertData->size() / 6; vertIndex++) {
            int objIndex = objIndices[vertIndex - 1];
            glm::vec3 vertexNormal{0};
            for (glm::vec3 faceNormal : faceNormalMap[objIndex]) {
                vertexNormal += faceNormal;
            }
            vertexNormal = glm::normalize(vertexNormal);
            // replace placeholder zeros in vertData with vertex normals
            (*vertData)[(vertIndex - 1) * 6 + 3] = vertexNormal.x;
            (*vertData)[(vertIndex - 1) * 6 + 4] = vertexNormal.y;
            (*vertData)[(vertIndex - 1) * 6 + 5] = vertexNormal.z;
        }
    }

    return 1;
}
