#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "settings.h"
#include "vertexcreator.h"
#include "utils/shaderloader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>

// ================== Rendering the Scene!

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;

    // If you must use this function, do not edit anything above this

    m_lightOrthoMatrix = glm::ortho(-10.f, 10.f, -10.f, 10.f, 1.f, 20.f);
    m_lightPerspectiveMatrix = glm::perspective(glm::radians(45.f), (float)shadowWidth / shadowHeight, 1.f, 20.f);
    m_biasMatrix = glm::mat4{
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
        0.5, 0.5, 0.5, 1.0
    };
}

glm::mat4 Realtime::getLightViewMatrix(const glm::vec3& lightPos, const glm::vec3& lightInvDir, bool isSpotLight) {
    // up vector cannot be parallel to light direction
    glm::vec3 up{0, 1, 0};
    if (glm::length2(glm::cross(lightInvDir, up)) < 0.001f) {
        // cross product is roughly zero so vectors are parallel. choose a different up vector
        up = glm::vec3{1, 0, 0};
    }

    if (isSpotLight) {
        return glm::lookAt(lightPos, lightPos - lightInvDir, up);
    } else {
        // directional light
        return glm::lookAt(lightPos, glm::vec3(0, 0, 0), up);
    }
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here
    glDeleteProgram(m_default_shader);
    glDeleteProgram(m_shadowmap_shader);

    glDeleteTextures(numShadowMaps, &m_depthTextures[0]);
    glDeleteFramebuffers(1, &m_shadowFBO);

    m_shapeManager.finish(this);

    this->doneCurrent();
}

void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();

    // Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    // Allows OpenGL to draw objects appropriately on top of one another
    glEnable(GL_DEPTH_TEST);
    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here

    m_default_shader = ShaderLoader::createShaderProgram(
        ":/resources/shaders/default.vert",
        ":/resources/shaders/default.frag");
    m_shadowmap_shader = ShaderLoader::createShaderProgram(
        ":/resources/shaders/shadowmap.vert",
        ":/resources/shaders/shadowmap.frag"
    );

    makeFBO();

    m_shapeManager.init(this);
    m_shapeManager.updateShapeVertices(this, settings.shapeParameter1, settings.shapeParameter2);
}

/**
 * @brief make framebuffer and depth textures for shadow mapping
 */
void Realtime::makeFBO() {
    this->makeCurrent();

    if (m_haveMadeFBO) {
        glDeleteTextures(numShadowMaps, &m_depthTextures[0]);
        glDeleteFramebuffers(1, &m_shadowFBO);
    }

    glGenFramebuffers(1, &m_shadowFBO);
    glGenTextures(numShadowMaps, &m_depthTextures[0]);

    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFBO);
    for (int texIndex = 0; texIndex < numShadowMaps; texIndex++) {
        glActiveTexture(GL_TEXTURE0 + texIndex);
        glBindTexture(GL_TEXTURE_2D, m_depthTextures[texIndex]);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = {1.f, 1.f, 1.f, 1.f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTextures[texIndex], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    // check that our framebuffer is ok
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "makeFBO: issue with framebuffer: " << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

    m_haveMadeFBO = true;
    this->doneCurrent();
}

void Realtime::shadowMap(const SceneLightData& lightData, int texIndex) {
    this->makeCurrent();

    if (!settings.extraCredit1) {
        return;
    }

    glm::vec3 lightPos;
    glm::mat4 depthProjMatrix;
    glm::mat4 depthViewMatrix;

    switch (lightData.type) {
    case LightType::LIGHT_DIRECTIONAL:
        lightPos = -lightData.dir * dirLightPosOffset;
        depthProjMatrix = m_lightOrthoMatrix;
        depthViewMatrix = getLightViewMatrix(lightPos, -lightData.dir, false);
        break;
    case LightType::LIGHT_SPOT:
        lightPos = lightData.pos;
        depthProjMatrix = m_lightPerspectiveMatrix;
        depthViewMatrix = getLightViewMatrix(lightPos, -lightData.dir, true);
        break;
    default:
        // shadow maps not implemented for point lights
        return;
    }

    glUseProgram(m_shadowmap_shader);

    GLint depthProjMatrixLoc = glGetUniformLocation(m_shadowmap_shader, "depthProjMatrix");
    GLint depthViewMatrixLoc = glGetUniformLocation(m_shadowmap_shader, "depthViewMatrix");

    glUniformMatrix4fv(depthProjMatrixLoc, 1, GL_FALSE, &depthProjMatrix[0][0]);
    glUniformMatrix4fv(depthViewMatrixLoc, 1, GL_FALSE, &depthViewMatrix[0][0]);

    glActiveTexture(GL_TEXTURE0 + texIndex);
    glBindTexture(GL_TEXTURE_2D, m_depthTextures[texIndex]);
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTextures[texIndex], 0);

    glViewport(0, 0, shadowWidth, shadowHeight);
    glClear(GL_DEPTH_BUFFER_BIT);

    // uniforms for each shape. Bind corresponding vao and make draw call for every shape.
    for (RenderShapeData& shapeData : m_renderData.shapes) {
        glBindVertexArray(m_shapeManager.getVao(shapeData));

        GLint modelMatrixLoc = glGetUniformLocation(m_shadowmap_shader, "modelMatrix");
        glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &shapeData.ctm[0][0]);

        glDrawArrays(GL_TRIANGLES, 0, m_shapeManager.getVertexDataSize(shapeData) / 11);

        glBindVertexArray(0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);
    glUseProgram(0);
}

void Realtime::paintGL() {
    if (!m_sceneLoaded) {
        return;
    }

    // Shadow map: render from the pov of each light
    int lightIndex = 0;
    for (const SceneLightData& lightData : m_renderData.lights) {
        shadowMap(lightData, lightIndex);
        lightIndex++;
    }

    // Students: anything requiring OpenGL calls every frame should be done here
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_default_shader);

    // global uniforms for entire scene
    GLint kaLoc = glGetUniformLocation(m_default_shader, "ka");
    GLint kdLoc = glGetUniformLocation(m_default_shader, "kd");
    GLint ksLoc = glGetUniformLocation(m_default_shader, "ks");
    glUniform1f(kaLoc, m_renderData.globalData.ka);
    glUniform1f(kdLoc, m_renderData.globalData.kd);
    glUniform1f(ksLoc, m_renderData.globalData.ks);

    GLint cameraPosLoc = glGetUniformLocation(m_default_shader, "cameraPos");
    glUniform4fv(cameraPosLoc, 1, &m_camera.getPos()[0]);

    GLint numLightsLoc = glGetUniformLocation(m_default_shader, "numLights");
    int numLights = m_renderData.lights.size();
    glUniform1i(numLightsLoc, numLights);

    GLint shadowsEnabledLoc = glGetUniformLocation(m_default_shader, "shadowsEnabled");
    glUniform1i(shadowsEnabledLoc, settings.extraCredit1);

    GLint fogEnabledLoc = glGetUniformLocation(m_default_shader, "fogEnabled");
    glUniform1i(fogEnabledLoc, settings.extraCredit2);


    for (int texIndex = 0; texIndex < numShadowMaps; texIndex++) {
        glActiveTexture(GL_TEXTURE0 + texIndex);
        glBindTexture(GL_TEXTURE_2D, m_depthTextures[texIndex]);
        std::string texUniform = "depthTextures[" + std::to_string(texIndex) + "]";
        GLint textureLoc = glGetUniformLocation(m_default_shader, texUniform.c_str());
        glUniform1i(textureLoc, texIndex);
    }


    lightIndex = 0;
    // uniforms for each light
    for (SceneLightData& lightData : m_renderData.lights) {
        glm::vec3 lightPos;
        glm::mat4 depthProjMatrix;
        glm::mat4 depthViewMatrix;

        switch (lightData.type) {
        case LightType::LIGHT_DIRECTIONAL:
            lightPos = -lightData.dir * dirLightPosOffset;
            depthProjMatrix = m_lightOrthoMatrix;
            depthViewMatrix = getLightViewMatrix(lightPos, -lightData.dir, false);
            break;
        case LightType::LIGHT_SPOT:
            lightPos = lightData.pos;
            depthProjMatrix = m_lightPerspectiveMatrix;
            depthViewMatrix = getLightViewMatrix(lightPos, -lightData.dir, true);
            break;
        default:
            // shadow maps not implemented for point lights
            break;
        }

        glm::mat4 depthBiasVP = m_biasMatrix * depthProjMatrix * depthViewMatrix;
        std::string uniformDepthBiasVP = "depthBiasVPs[" + std::to_string(lightIndex) + "]";
        GLint depthBiasVPLoc = glGetUniformLocation(m_default_shader, uniformDepthBiasVP.c_str());
        glUniformMatrix4fv(depthBiasVPLoc, 1, GL_FALSE, &depthBiasVP[0][0]);


        std::string lightsUniform = "lights[" + std::to_string(lightIndex) + "]";
        std::string lightsUniformLightType = lightsUniform + ".lightType";
        std::string lightsUniformPos = lightsUniform + ".pos";
        std::string lightsUniformDir = lightsUniform + ".dir";
        std::string lightsUniformColor = lightsUniform + ".color";
        std::string lightsUniformAttenCoeff = lightsUniform + ".attenCoeff";
        std::string lightsUniformAngle = lightsUniform + ".angle";
        std::string lightsUniformPenumbra = lightsUniform + ".penumbra";

        GLint lightTypeLoc = glGetUniformLocation(m_default_shader, lightsUniformLightType.c_str());
        GLint posLoc = glGetUniformLocation(m_default_shader, lightsUniformPos.c_str());
        GLint dirLoc = glGetUniformLocation(m_default_shader, lightsUniformDir.c_str());
        GLint colorLoc = glGetUniformLocation(m_default_shader, lightsUniformColor.c_str());
        GLint attenCoeffLoc = glGetUniformLocation(m_default_shader, lightsUniformAttenCoeff.c_str());
        GLint angleLoc = glGetUniformLocation(m_default_shader, lightsUniformAngle.c_str());
        GLint penumbraLoc = glGetUniformLocation(m_default_shader, lightsUniformPenumbra.c_str());

        glUniform1i(lightTypeLoc, static_cast<GLint>(lightData.type));
        glUniform4fv(posLoc, 1, &lightData.pos[0]);
        glUniform4fv(dirLoc, 1, &lightData.dir[0]);
        glUniform4fv(colorLoc, 1, &lightData.color[0]);
        glUniform3fv(attenCoeffLoc, 1, &lightData.function[0]);
        glUniform1f(angleLoc, lightData.angle);
        glUniform1f(penumbraLoc, lightData.penumbra);

        lightIndex++;
    }

    // uniforms for each shape. Bind corresponding vao and make draw call for every shape.
    for (RenderShapeData& shapeData : m_renderData.shapes) {
        glBindVertexArray(m_shapeManager.getVao(shapeData));

        GLint modelMatrixLoc = glGetUniformLocation(m_default_shader, "modelMatrix");
        GLint viewMatrixLoc = glGetUniformLocation(m_default_shader, "viewMatrix");
        GLint projectionMatrixLoc = glGetUniformLocation(m_default_shader, "projectionMatrix");
        glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &shapeData.ctm[0][0]);
        glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_camera.getViewMatrix()[0][0]);
        glUniformMatrix4fv(projectionMatrixLoc, 1, GL_FALSE, &m_camera.getProjMatrix()[0][0]);

        // material constants
        GLint shininessLoc = glGetUniformLocation(m_default_shader, "shininess");
        GLint cAmbientLoc = glGetUniformLocation(m_default_shader, "cAmbient");
        GLint cDiffuseLoc = glGetUniformLocation(m_default_shader, "cDiffuse");
        GLint cSpecularLoc = glGetUniformLocation(m_default_shader, "cSpecular");
        glUniform1f(shininessLoc, shapeData.primitive.material.shininess);
        glUniform4fv(cAmbientLoc, 1, &shapeData.primitive.material.cAmbient[0]);
        glUniform4fv(cDiffuseLoc, 1, &shapeData.primitive.material.cDiffuse[0]);
        glUniform4fv(cSpecularLoc, 1, &shapeData.primitive.material.cSpecular[0]);

        GLint m_blendLocation = glGetUniformLocation(m_default_shader, "blend");
        glUniform1f(m_blendLocation, shapeData.primitive.material.blend);
        activeTexture(shapeData.primitive.material);

        glDrawArrays(GL_TRIANGLES, 0, m_shapeManager.getVertexDataSize(shapeData) / 11);

        glBindVertexArray(0);
    }

    glUseProgram(0);
}

void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here
    m_camera.init(m_renderData.cameraData, w, h);

    makeFBO();
}

void Realtime::parseScene() {
    if (!m_sceneParser.parse(settings.sceneFilePath, m_renderData)) {
        std::cerr << "error parsing scene" << std::endl;
    } else {
        std::cout << "scene parsed successfully with " << m_renderData.shapes.size() << " shape" << std::endl;
    }

    m_camera.updateCamData(m_renderData.cameraData);
    m_shapeManager.parseMeshes(this, m_renderData.shapes);

    m_sceneLoaded = true;
}

void Realtime::sceneChanged() {
    parseScene();
    createTextureAndNormal();

    update(); // asks for a PaintGL() call to occur
}

/**
 * @brief Update shape vertices if the shape parameters have changed.
 * Update the camera projection matrix if the near,far planes have changed in settings.
 */
void Realtime::settingsChanged() {
    // recompute shape vertices using new shape parameters in settings (if they have changed)
    if (settings.shapeParameter1 != prevParam1 || settings.shapeParameter2 != prevParam2) {
        // updateShapeVertices();
        m_shapeManager.updateShapeVertices(this, settings.shapeParameter1, settings.shapeParameter2);
        prevParam1 = settings.shapeParameter1;
        prevParam2 = settings.shapeParameter2;
    }

    // update camera planes and recompute projection matrix if planes have changed
    if (settings.nearPlane != prevNearPlane || settings.farPlane != prevFarPlane) {
        m_camera.updatePlanes(settings.nearPlane, settings.farPlane);
        prevNearPlane = settings.nearPlane;
        prevFarPlane = settings.farPlane;
    }

    update(); // asks for a PaintGL() call to occur
}

// ================== Camera Movement!

void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

void Realtime::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        // Use deltaX and deltaY here to rotate
        m_camera.rotateCamera(deltaX, deltaY);

        update(); // asks for a PaintGL() call to occur
    }
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // Use deltaTime and m_keyMap here to move around
    if (m_keyMap[Qt::Key::Key_W]) {
        m_camera.moveForward(deltaTime);
    }
    if (m_keyMap[Qt::Key::Key_S]) {
        m_camera.moveBackward(deltaTime);
    }
    if (m_keyMap[Qt::Key::Key_A]) {
        m_camera.moveLeft(deltaTime);
    }
    if (m_keyMap[Qt::Key::Key_D]) {
        m_camera.moveRight(deltaTime);
    }
    if (m_keyMap[Qt::Key::Key_Space]) {
        m_camera.moveUp(deltaTime);
    }
    if (m_keyMap[Qt::Key::Key_Control]) {
        m_camera.moveDown(deltaTime);
    }


    update(); // asks for a PaintGL() call to occur
}

/**
 * @brief Helpers for textures
 */
// generate openGL textures and store ids to hash if textures are used
void Realtime::createTextureAndNormal(){
    this->makeCurrent();

    vertexCreator::createTexture(m_textures, m_renderData.shapes);
    vertexCreator::createNormalText(m_normalTextures, m_renderData.shapes);
    vertexCreator::createBumpText(m_bumpTextures, m_renderData.shapes);

    this->doneCurrent();
}

// active texture slots and pass uniforms to fragment shader
void Realtime::activeTexture(const SceneMaterial& shapeMat){
    if(shapeMat.textureMap.isUsed){
        GLuint textureId = m_textures[shapeMat.textureMap.filename];
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);

        GLint m_textSampLocation = glGetUniformLocation(m_default_shader, "myTextures.textureSampler");
        GLint m_textIsUsedLocation = glGetUniformLocation(m_default_shader, "myTextures.textureIsUsed");
        glUniform1i(m_textSampLocation, 0);
        glUniform1i(m_textIsUsedLocation, true);

        glm::vec2 repeatUV = glm::vec2(shapeMat.textureMap.repeatU, shapeMat.textureMap.repeatV);
        GLint  m_textRepeatLocation = glGetUniformLocation(m_default_shader, "myTextures.textureRepeat");
        glUniform2fv(m_textRepeatLocation, 1, &repeatUV[0]);
    }
    else{
        GLint m_textIsUsedLocation = glGetUniformLocation(m_default_shader, "myTextures.textureIsUsed");
        glUniform1i(m_textIsUsedLocation, false);
    }

    if(shapeMat.normalMap.isUsed){
        GLuint normalId = m_normalTextures[shapeMat.normalMap.filename];
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalId);

        GLint m_normSampLocation = glGetUniformLocation(m_default_shader, "myNormals.textureSampler");
        GLint m_normIsUsedLocation = glGetUniformLocation(m_default_shader, "myNormals.textureIsUsed");
        glUniform1i(m_normSampLocation, 1);
        glUniform1i(m_normIsUsedLocation, true);

        glm::vec2 repeatUV = glm::vec2(shapeMat.normalMap.repeatU, shapeMat.normalMap.repeatV);
        GLint m_normRepeatLocation = glGetUniformLocation(m_default_shader, "myNormals.textureRepeat");
        glUniform2fv(m_normRepeatLocation, 1, &repeatUV[0]);
    }
    else{
        GLint m_normIsUsedLocation = glGetUniformLocation(m_default_shader, "myNormals.textureIsUsed");
        glUniform1i(m_normIsUsedLocation, false);
    }

    if(shapeMat.bumpMap.isUsed){
        GLuint bumpId = m_bumpTextures[shapeMat.bumpMap.filename];
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, bumpId);

        GLint m_bumpSampLocation = glGetUniformLocation(m_default_shader, "myBumps.textureSampler");
        GLint m_bumpIsUsedLocation = glGetUniformLocation(m_default_shader, "myBumps.textureIsUsed");
        glUniform1i(m_bumpSampLocation, 2);
        glUniform1i(m_bumpIsUsedLocation, true);

        glm::vec2 repeatUV = glm::vec2(shapeMat.bumpMap.repeatU, shapeMat.bumpMap.repeatV);
        GLint m_bumpRepeatLocation = glGetUniformLocation(m_default_shader, "myBumps.textureRepeat");
        glUniform2fv(m_bumpRepeatLocation, 1, &repeatUV[0]);
    }
    else{
        GLint m_bumpIsUsedLocation = glGetUniformLocation(m_default_shader, "myBumps.textureIsUsed");
        glUniform1i(m_bumpIsUsedLocation, false);
    }
}

// DO NOT EDIT
void Realtime::saveViewportImage(std::string filePath) {
    // Make sure we have the right context and everything has been drawn
    makeCurrent();

    // int fixedWidth = 1024;
    // int fixedHeight = 768;

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    int fixedWidth = viewport[2];
    int fixedHeight = viewport[3];


    // Create Frame Buffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a color attachment texture
    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fixedWidth, fixedHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Optional: Create a depth buffer if your rendering uses depth testing
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fixedWidth, fixedHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    // Render to the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fixedWidth, fixedHeight);

    // Clear and render your scene here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();

    // Read pixels from framebuffer
    std::vector<unsigned char> pixels(fixedWidth * fixedHeight * 3);
    glReadPixels(0, 0, fixedWidth, fixedHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Unbind the framebuffer to return to default rendering to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

    // Convert to QImage
    QImage image(pixels.data(), fixedWidth, fixedHeight, QImage::Format_RGB888);
    QImage flippedImage = image.mirrored(); // Flip the image vertically

    // Save to file using Qt
    QString qFilePath = QString::fromStdString(filePath);
    if (!flippedImage.save(qFilePath)) {
        std::cerr << "Failed to save image to " << filePath << std::endl;
    }

    // Clean up
    glDeleteTextures(1, &texture);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
}
