#include "sceneparser.h"
#include "scenefilereader.h"
#include <glm/gtx/transform.hpp>

#include "shapes/sphere.h"
#include "shapes/cube.h"
#include "shapes/cone.h"
#include "shapes/cylinder.h"

#include <chrono>
#include <iostream>

bool SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readJSON();
    if (!success) {
        return false;
    }

    // TODO: Use your Lab 5 code here
    renderData.globalData = fileReader.getGlobalData();
    renderData.cameraData = fileReader.getCameraData();

    renderData.shapes.clear();
    renderData.lights.clear();
    traverseTree(fileReader.getRootNode(), glm::mat4(1.f), renderData);

    return true;
}

void SceneParser::traverseTree(SceneNode* curNode, glm::mat4 ctm, RenderData &renderData) {
    // update ctm with this node's transformations
    for(SceneTransformation* transformation : curNode->transformations) {
        switch(transformation->type) {
        case TransformationType::TRANSFORMATION_TRANSLATE:
            ctm *= glm::translate(transformation->translate);
            break;
        case TransformationType::TRANSFORMATION_SCALE:
            ctm *= glm::scale(transformation->scale);
            break;
        case TransformationType::TRANSFORMATION_ROTATE:
            ctm *= glm::rotate(transformation->angle, transformation->rotate);
            break;
        case TransformationType::TRANSFORMATION_MATRIX:
            ctm *= transformation->matrix;
            break;
        }
    }

    // add shapes to render data
    for(ScenePrimitive* scenePrimitive : curNode->primitives) {
        RenderShapeData shapeData{*scenePrimitive, ctm};
        renderData.shapes.push_back(shapeData);
    }

    // add lights to render data
    for(SceneLight* light : curNode->lights) {
        SceneLightData lightData;
        LightType lightType = light->type;

        lightData.id = light->id;
        lightData.type = lightType;
        lightData.color = light->color;
        lightData.function = light->function;

        // for point lights and spot lights, transform the origin in camera space to the camera's position in world space using the CTM
        if (lightType == LightType::LIGHT_POINT || lightType == LightType::LIGHT_SPOT) {
            lightData.pos = ctm * glm::vec4(0., 0., 0., 1.);
        }

        // for spot lights and directional lights, transform dir to world space using the CTM
        if (lightType == LightType::LIGHT_SPOT || lightType == LightType::LIGHT_DIRECTIONAL) {
            lightData.dir = ctm * light->dir;
        }

        // for only spot lights, store angle and penumbra in lightData
        if (lightType == LightType::LIGHT_SPOT) {
            lightData.angle = light->angle;
            lightData.penumbra = light->penumbra;
        }

        renderData.lights.push_back(lightData);
    }

    // recursively traverse child nodes
    for(SceneNode* child : curNode->children) {
        traverseTree(child, ctm, renderData);
    }
}
