#ifndef SHAPEMANAGER_H
#define SHAPEMANAGER_H

#include "cone.h"
#include "cube.h"
#include "sphere.h"
#include "cylinder.h"
#include "mesh.h"
#include "utils/sceneparser.h"

class ShapeManager
{
public:
    ShapeManager();

    void init(QOpenGLWidget* widget);
    void finish(QOpenGLWidget* widget);

    void updateShapeVertices(QOpenGLWidget* widget, int param1, int param2);

    void parseMeshes(QOpenGLWidget *widget, const std::vector<RenderShapeData>& shapes);

    GLuint getVao(const RenderShapeData& shapeData);

    int getVertexDataSize(const RenderShapeData& shapeData);
private:
    bool m_initialized = false;

    Shape m_cone = Cone();
    Shape m_cube = Cube();
    Shape m_sphere = Sphere();
    Shape m_cylinder = Cylinder();

    const Shape& getShape(const RenderShapeData& shapeData);

    // unordered map from meshfile to (Mesh) Shape objects
    std::unordered_map<std::string, Shape> meshMap;
};

#endif // SHAPEMANAGER_H
