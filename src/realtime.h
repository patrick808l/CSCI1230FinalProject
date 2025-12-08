#pragma once

// Defined before including GLEW to suppress deprecation messages on macOS
#include "shapes/shapemanager.h"
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <iostream>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>

#include "utils/sceneparser.h"
#include "camera/camera.h"

class Realtime : public QOpenGLWidget
{
public:
    Realtime(QWidget *parent = nullptr);
    void finish();                                      // Called on program exit
    void sceneChanged();
    void settingsChanged();
    void saveViewportImage(std::string filePath);

public slots:
    void tick(QTimerEvent* event);                      // Called once per tick of m_timer

protected:
    void initializeGL() override;                       // Called once at the start of the program
    void paintGL() override;                            // Called whenever the OpenGL context changes or by an update() request
    void resizeGL(int width, int height) override;      // Called when window size changes

private:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    // Tick Related Variables
    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames

    // Input Related Variables
    bool m_mouseDown = false;                           // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;                         // Stores mouse position
    std::unordered_map<Qt::Key, bool> m_keyMap;         // Stores whether keys are pressed or not

    // Device Correction Variables
    double m_devicePixelRatio;


    RenderData m_renderData;
    SceneParser m_sceneParser;
    ShapeManager m_shapeManager;
    bool m_sceneLoaded = false;
    void parseScene();
    void updateShapeVertices();
    Camera m_camera;

    float prevNearPlane = -1;
    float prevFarPlane = -1;
    int prevParam1 = -1;
    int prevParam2 = -1;
    bool prevShadowsEnabled = false;


    GLuint m_default_shader;
    GLuint m_shadowmap_shader;

    void shadowMap(const SceneLightData& lightData, int lightIndex);
    bool m_haveMadeFBO = false;
    void makeFBO();

    const static int numShadowMaps = 8;
    GLuint m_depthTextures[numShadowMaps];
    GLuint m_shadowFBO;
    // int shadowWidth = 1024;
    // int shadowHeight = 1024;
    int shadowWidth = 2048;
    int shadowHeight = 2048;

    glm::mat4 m_lightOrthoMatrix;
    glm::mat4 m_lightPerspectiveMatrix;
    glm::mat4 m_biasMatrix;
    float dirLightPosOffset = 10.f;
    glm::mat4 getLightViewMatrix(const glm::vec3& lightPos, const glm::vec3& lightInvDir, bool isSpotLight);

    // textures
    std::unordered_map<std::string, GLuint> m_textures; // hash for texture filename and texture id
    std::unordered_map<std::string, GLuint> m_normalTextures; // hash for normal texture filename and normal texture id
    std::unordered_map<std::string, GLuint> m_bumpTextures; // hash for bump texture filename and bump texture id
    void createTextureAndNormal();
    void activeTexture(const SceneMaterial& shapeMat);
};
