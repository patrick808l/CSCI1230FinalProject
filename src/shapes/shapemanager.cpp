#include "shapemanager.h"
#include "shapes/staticmesh.h"


// default constructor
ShapeManager::ShapeManager() {
    m_activeModel = "zebra";
}

/**
 * @brief initialize the vbo and vao for each of the shape types.
 * initialize a skeletal model and animator.
 * @param widget allows access to makeCurrent for openGL context
 */
void ShapeManager::init(QOpenGLWidget* widget) {
    m_cone.initGLObjects(widget);
    m_cube.initGLObjects(widget);
    m_sphere.initGLObjects(widget);
    m_cylinder.initGLObjects(widget);

    m_modelFinalTransform = glm::mat4{1.f};

    if (m_activeModel == "vampire") {
        m_model.init(widget, "animated_models/vampire/dancing_vampire.dae");
        m_animation.init("animated_models/vampire/dancing_vampire.dae", &m_model);
        m_animator.init(&m_animation);

        m_modelFinalTransform = glm::rotate(m_modelFinalTransform, glm::radians(90.f), glm::vec3{0, 1, 0});
    }
    else if (m_activeModel == "fish") {
        m_model.init(widget, "animated_models/fish/source/fish_2_anim.fbx");
        m_animation.init("animated_models/fish/source/fish_2_anim.fbx", &m_model);
        m_animator.init(&m_animation);

        m_modelFinalTransform = glm::rotate(m_modelFinalTransform, glm::radians(180.f), glm::vec3{0, 1, 0});
        m_modelFinalTransform = glm::scale(m_modelFinalTransform, glm::vec3{0.02f});
    }
    else if (m_activeModel == "eagle") {
        /// Eagle broken
        m_model.init(widget, "animated_models/eagle/eagle.dae");
        m_animation.init("animated_models/eagle/eagle.dae", &m_model);
        m_animator.init(&m_animation, 0.1f);

        m_modelFinalTransform = glm::rotate(m_modelFinalTransform, glm::radians(90.f), glm::vec3{0, 1, 0});
        m_modelFinalTransform = glm::scale(m_modelFinalTransform, glm::vec3{2.f});
        m_modelFinalTransform = glm::translate(m_modelFinalTransform, glm::vec3{0.f, 0.5f, 0.f});
    }
    else if (m_activeModel == "woodpecker") {
        m_model.init(widget, "animated_models/woodpecker/woodpecker.dae");
        m_animation.init("animated_models/woodpecker/woodpecker.dae", &m_model);
        m_animator.init(&m_animation, 0.2f);

        m_modelFinalTransform = glm::rotate(m_modelFinalTransform, glm::radians(90.f), glm::vec3{0, 1, 0});
        m_modelFinalTransform = glm::scale(m_modelFinalTransform, glm::vec3{2.f});
        m_modelFinalTransform = glm::translate(m_modelFinalTransform, glm::vec3{0.f, 0.25f, 0.f});
    }
    else if (m_activeModel == "zebra") {
        m_model.init(widget, "animated_models/zebra-fbx/source/Zebra.fbx");
        for (int i = 0; i < 17; i++) {
            std::shared_ptr<Animation> newAnimation = std::make_shared<Animation>();
            newAnimation->init("animated_models/zebra-fbx/source/Zebra.fbx", &m_model, i);
            m_animations.push_back(newAnimation);
        }
        // idle initially
        m_animator.init(m_animations[11].get());

        m_modelFinalTransform = glm::scale(m_modelFinalTransform, glm::vec3{0.01f});
        m_modelFinalTransform = glm::rotate(m_modelFinalTransform, glm::radians(90.f), glm::vec3{0, 1, 0});
    }

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

    m_model.deleteGLObjects();
}

/**
 * @brief create and initialize unique mesh objects in the scene. buffer data into respectiev vbos.
 * @param shapes is a vector of RenderShapeData
 */
void ShapeManager::parseMeshes(QOpenGLWidget *widget, std::vector<RenderShapeData>& shapes) {
    for (const RenderShapeData& shapeData : shapes) {
        if (shapeData.primitive.type == PrimitiveType::PRIMITIVE_MESH &&
                !meshMap.contains(shapeData.primitive.meshfile)) {
            // create new mesh and update its vertices. tessellation params ignored.
            meshMap[shapeData.primitive.meshfile] = StaticMesh(shapeData.primitive.meshfile);
            meshMap[shapeData.primitive.meshfile].updateVertexData(0, 0);
            meshMap[shapeData.primitive.meshfile].initGLObjects(widget);
            meshMap[shapeData.primitive.meshfile].bufferData(widget);
        }

        if (shapeData.primitive.type == PrimitiveType::PRIMITIVE_ANIMATED_MODEL) {
            // store shapeData or primitive or rigid body in a field where transform matrix can be accessed
            m_modelShapeData = shapeData;
            m_playerIsModel = true;
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
    case PrimitiveType::PRIMITIVE_ANIMATED_MODEL:
        throw std::runtime_error("getShape: called on animated model which does not have a Shape");
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

/**
 * @brief update skeletal mesh by progressing the current animation
 * @param deltaTime is the time since the last update
 */
void ShapeManager::advanceCurAnimation(float deltaTime) {
    m_animator.UpdateAnimation(deltaTime);
}

/**
 * @brief change the mesh to a new animation
 * @param animationIndex into m_animations
 */
void ShapeManager::queueNewAnimation(int animationIndex) {
    if (animationIndex < m_animations.size()) {
        m_animator.QueueAnimation(m_animations[animationIndex].get());
    } else {
        std::cerr << "queueNewAnimation called with invalid index " << animationIndex << std::endl;
    }
}

void ShapeManager::forceNewAnimation(int animationIndex) {
    if (animationIndex < m_animations.size()) {
        m_animator.ForceAnimation(m_animations[animationIndex].get());
    } else {
        std::cerr << "forceNewAnimation called with invalid index " << animationIndex << std::endl;
    }
}

void ShapeManager::setAnimation(std::string animationName) {
    if (m_activeModel == "zebra") {
        if (animationName == "idle") {
            forceNewAnimation(11);
        } else if (animationName == "trot") {
            forceNewAnimation(15);
        } else if (animationName == "trot slow") {
            forceNewAnimation(14);
        } else if (animationName == "gallop") {
            forceNewAnimation(4);
        } else if (animationName == "jump") {
            forceNewAnimation(13);
            // queue idle after jump
            queueNewAnimation(11);
        } else if (animationName == "walk backward") {
            forceNewAnimation(16);
        } else {
            std::cerr << "setAnimation receieved unrecognized animationName: " << animationName << std::endl;
        }
    }
}

std::string ShapeManager::getCurrentAnimationName() {
    if (m_activeModel == "zebra") {
        /// this should be optimized, maybe with an unordered map
        int animationIndex = -1;
        for (int i = 0; i < 17; i++) {
            if (m_animator.CurrentAnimation == m_animations[i].get()) {
                animationIndex = i;
                break;
            }
        }

        switch (animationIndex) {
        case 4:
            return "gallop";
        case 11:
            return "idle";
        case 13:
            return "jump";
        case 14:
            return "trot slow";
        case 15:
            return "trot";
        case 16:
            return "walk backward";
        }
        return "unused zebra action";
    }
    return "none";
}


void ShapeManager::prepareModelUniforms(QOpenGLWidget* widget, GLuint shader) {
    widget->makeCurrent();
    glUseProgram(shader);

    std::vector<glm::mat4> transforms = m_animator.GetFinalBoneMatrices();
    for (int i = 0; i < transforms.size(); i++) {
        std::string uniformFinalBones = "finalBonesMatrices[" + std::to_string(i) + "]";
        GLint finalBonesMatrixLoc = glGetUniformLocation(shader, uniformFinalBones.c_str());
        glUniformMatrix4fv(finalBonesMatrixLoc, 1, GL_FALSE, &transforms[i][0][0]);
    }

    glm::mat4 modelMatrix;
    if (m_playerIsModel) {
        modelMatrix = m_modelShapeData.getMovedCTM();
        // rotate so model faces away from the camera
        modelMatrix = modelMatrix * m_modelFinalTransform;
    } else {
        modelMatrix = m_modelFinalTransform;
    }
    GLint modelMatrixLoc = glGetUniformLocation(shader, "modelMatrix");
    glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &modelMatrix[0][0]);
}

/**
 * @brief ShapeManager::drawAnimatedModelDefault
 * @param widget allows access to makeCurrent for openGL context
 * @param postprocessor used to bind framebuffer if postprocessing enabled
 * @param post_processing_enabled whether to use postprocessor. if false postprocessor may be nullptr
 * @param shader is the default shader program used for rendering the scene
 */
void ShapeManager::drawAnimatedModelDefault(QOpenGLWidget* widget, PostProcessor* postprocessor, bool post_processing_enabled, GLuint shader) {
    prepareModelUniforms(widget, shader);
    m_model.Draw(postprocessor, post_processing_enabled, shader);
}

/**
 * @brief ShapeManager::drawAnimatedModelShadow
 * @param widget allows access to makeCurrent for openGL context
 * @param shadowFBO is the framebuffer for shadow mapping into depth textures
 * @param shadowWidth of the shadowFBO
 * @param shadowHeight of the shadowFBO
 * @param shader is the shadowmap shader program used for rendering the scene from lights' POVs into depth buffers
 */
void ShapeManager::drawAnimatedModelShadow(QOpenGLWidget* widget, GLuint shadowFBO, int shadowWidth, int shadowHeight, GLuint shader) {
    prepareModelUniforms(widget, shader);
    m_model.DrawShadow(shadowFBO, shadowWidth, shadowHeight, shader);
}
