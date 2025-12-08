#ifndef SHAPEMANAGER_H
#define SHAPEMANAGER_H

#include "skeletal_animation/animation.h"
#include "skeletal_animation/animator.h"
#include "skeletal_animation/model_animation.h"

#include "cone.h"
#include "cube.h"
#include "sphere.h"
#include "cylinder.h"
#include "utils/sceneparser.h"



class ShapeManager
{
public:
    ShapeManager(QOpenGLWidget* widget);

    void init(QOpenGLWidget* widget);
    void finish(QOpenGLWidget* widget);

    void updateShapeVertices(QOpenGLWidget* widget, int param1, int param2);

    void parseMeshes(QOpenGLWidget *widget, const std::vector<RenderShapeData>& shapes);

    GLuint getVao(const RenderShapeData& shapeData);

    int getVertexDataSize(const RenderShapeData& shapeData);

    void updateAnimation(float deltaTime);
    void drawAnimatedModel(QOpenGLWidget* widget, GLuint shader);
private:
    bool m_initialized = false;

    Shape m_cone = Cone();
    Shape m_cube = Cube();
    Shape m_sphere = Sphere();
    Shape m_cylinder = Cylinder();

    const Shape& getShape(const RenderShapeData& shapeData);

    // unordered map from meshfile to (Mesh) Shape objects
    std::unordered_map<std::string, Shape> meshMap;


    // skeletal animation
    Model m_model;
    Animation m_animation;
    Animator m_animator;
};

#endif // SHAPEMANAGER_H
