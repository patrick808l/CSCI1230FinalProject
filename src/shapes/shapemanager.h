#ifndef SHAPEMANAGER_H
#define SHAPEMANAGER_H

#include "postprocessing.h"
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
    ShapeManager();

    void init(QOpenGLWidget* widget);
    void finish(QOpenGLWidget* widget);

    void updateShapeVertices(QOpenGLWidget* widget, int param1, int param2);

    void parseMeshes(QOpenGLWidget *widget, std::vector<RenderShapeData> &shapes);

    GLuint getVao(const RenderShapeData& shapeData);

    int getVertexDataSize(const RenderShapeData& shapeData);

    void advanceCurAnimation(float deltaTime);
    void setAnimation(std::string animationName);
    std::string getCurrentAnimationName();

    void prepareModelUniforms(QOpenGLWidget* widget, GLuint shader);
    void drawAnimatedModelDefault(QOpenGLWidget* widget, PostProcessor *postprocessor, bool post_processing_enabled, GLuint shader);
    void drawAnimatedModelShadow(QOpenGLWidget* widget, GLuint shadowFBO, int shadowWidth, int shadowHeight, GLuint shader);
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
    std::string m_activeModel;
    Model m_model;
    Animation m_animation;
    std::vector<std::shared_ptr<Animation>> m_animations;
    Animator m_animator;
    void queueNewAnimation(int animationIndex);
    void forceNewAnimation(int animationIndex);
    glm::mat4 m_modelFinalTransform;
    RenderShapeData m_modelShapeData;
    bool m_playerIsModel = false;
};

#endif // SHAPEMANAGER_H
