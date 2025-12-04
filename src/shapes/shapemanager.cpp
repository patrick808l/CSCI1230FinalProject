#include "shapemanager.h"

ShapeManager::ShapeManager() {}

/**
 * @brief initialize the vbo and vao for each of the shape types.
 * @param widget allows access to makeCurrent for openGL context
 */
void ShapeManager::init(QOpenGLWidget* widget) {
    m_cone.initGLObjects(widget);
    m_cube.initGLObjects(widget);
    m_sphere.initGLObjects(widget);
    m_cylinder.initGLObjects(widget);

    m_initialized = true;
}

/**
 * @brief delete the vbo and vao for each of the shape types.
 * @param widget allows access to makeCurrent for openGL context
 */
void ShapeManager::finish(QOpenGLWidget* widget) {
    m_cone.deleteGLObjects(widget);
    m_cube.deleteGLObjects(widget);
    m_sphere.deleteGLObjects(widget);
    m_cylinder.deleteGLObjects(widget);
}

/**
 * @brief create and initialize unique mesh objects in the scene. buffer data into respectiev vbos.
 * @param shapes is a vector of RenderShapeData
 */
void ShapeManager::parseMeshes(QOpenGLWidget *widget, const std::vector<RenderShapeData>& shapes) {
    for (const RenderShapeData& shapeData : shapes) {
        if (shapeData.primitive.type == PrimitiveType::PRIMITIVE_MESH &&
                !meshMap.contains(shapeData.primitive.meshfile)) {
            // create new mesh and update its vertices. tessellation params ignored.
            meshMap[shapeData.primitive.meshfile] = Mesh(shapeData.primitive.meshfile);
            meshMap[shapeData.primitive.meshfile].updateVertexData(0, 0);
            meshMap[shapeData.primitive.meshfile].initGLObjects(widget);
            meshMap[shapeData.primitive.meshfile].bufferData(widget);
        }
    }
}

/**
 * @brief for each shape type, update its vertices with the given tessellation parameters.
 *      If the vbo and vao have been initialized, buffer the new vertex data into the vbo.
 * @param widget allows access to makeCurrent for openGL context
 * @param param1 is the first shape tessellation parameter
 * @param param2 is the second shape tessellation parameter
 */
void ShapeManager::updateShapeVertices(QOpenGLWidget *widget, int param1, int param2) {
    m_cone.updateVertexData(param1, param2);
    m_cube.updateVertexData(param1, param2);
    m_sphere.updateVertexData(param1, param2);
    m_cylinder.updateVertexData(param1, param2);

    if (m_initialized) {
        m_cone.bufferData(widget);
        m_cube.bufferData(widget);
        m_sphere.bufferData(widget);
        m_cylinder.bufferData(widget);
    }
}

/**
 * @brief given a RenderShapeData, switch on its primitive type in order to return the corresponding Shape object.
 * @param shapeData of a shape in the scene being rendered.
 * @return the Shape object
 */
const Shape& ShapeManager::getShape(const RenderShapeData& shapeData) {
    switch (shapeData.primitive.type) {
    case PrimitiveType::PRIMITIVE_CONE:
        return m_cone;
    case PrimitiveType::PRIMITIVE_CUBE:
        return m_cube;
    case PrimitiveType::PRIMITIVE_SPHERE:
        return m_sphere;
    case PrimitiveType::PRIMITIVE_CYLINDER:
        return m_cylinder;
    case PrimitiveType::PRIMITIVE_MESH:
        if (meshMap.count(shapeData.primitive.meshfile)) {
            return meshMap[shapeData.primitive.meshfile];
        } else {
            throw std::runtime_error("getShape: tried to get mesh that doesn't exist");
        }
    }
}

/**
 * @brief returns the int to which the corresponding vao is bound.
 * @param shapeData of a shape in the scene being rendered.
 * @return the vao corresponding to the given shape's type
 */
GLuint ShapeManager::getVao(const RenderShapeData& shapeData) {
    return getShape(shapeData).vao;
}

/**
 * @brief returns the size of the corresponding vbo so the glDrawArrays call can be made.
 * @param shapeData of a shape in the scene being rendered.
 * @return the number of floats in the shape type's vbo.
 */
int ShapeManager::getVertexDataSize(const RenderShapeData& shapeData) {
    return getShape(shapeData).getVertexData()->size();
}

