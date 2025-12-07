#pragma once

#include <vector>
#include <string>
#include "scenedata.h"
#include "shapes/shape.h"

// Forward declaration of rigid body and collider
struct RigidBody;
struct Collider;

// Struct which contains data for a single primitive, to be used for rendering
struct RenderShapeData {
    // Shape shape;
    ScenePrimitive primitive;
    RigidBody* rb; // a rigid body to describe how to shape moves

    /// Get the raw cumulative transform matrix, w/o rigid body transformations
    glm::mat4 getCTM();
    /// Get the cumulative transform matrix with the rigid body transformation applied to it
    glm::mat4 getMovedCTM();

    RenderShapeData(ScenePrimitive primitive, RigidBody* rb, glm::mat4 ctm);
private:
    glm::mat4 ctm; // the cumulative transformation matrix
};

// Struct which contains all the data needed to render a scene
struct RenderData {
    SceneGlobalData globalData;
    SceneCameraData cameraData;

    std::vector<SceneLightData> lights;
    std::vector<RenderShapeData> shapes;

    RenderData();
    std::vector<Collider*>* colliders();
private:
    std::vector<Collider*>* m_colliders;
};

class SceneParser {
public:
    // Parse the scene and store the results in renderData.
    // @param filepath    The path of the scene file to load.
    // @param renderData  On return, this will contain the metadata of the loaded scene.
    // @return            A boolean value indicating whether the parse was successful.
    static bool parse(std::string filepath, RenderData &renderData);
private:
    static void traverseTree(SceneNode* curNode, glm::mat4 ctm, RenderData &renderData);

    static Shape constructShape(PrimitiveType pType);
};
