#pragma once

#include <glm/glm.hpp>
#include "utils/scenedata.h"

// A class representing a virtual camera.

// Feel free to make your own design choices for Camera class, the functions below are all optional / for your convenience.
// You can either implement and use these getters, or make your own design.
// If you decide to make your own design, feel free to delete these as TAs won't rely on them to grade your assignments.

class Camera {
public:
    void init(SceneCameraData cameraData, float width, float height);

    void updateCamData(SceneCameraData cameraData);
    void updatePlanes(float nearPlane, float farPlane);


    // Returns the view matrix for the current camera settings.
    // You might also want to define another function that return the inverse of the view matrix.
    glm::mat4 getViewMatrix() const;
    glm::mat4 getInverseViewMatrix() const;

    glm::mat4 getProjMatrix() const;
    glm::mat4 getInverseProjMatrix() const;

    // Returns the aspect ratio of the camera.
    float getAspectRatio() const;

    // Returns the height angle of the camera in RADIANS.
    float getHeightAngle() const;

    // Returns the focal length of this camera.
    // This is for the depth of field extra-credit feature only;
    // You can ignore if you are not attempting to implement depth of field.
    float getFocalLength() const;

    // Returns the focal length of this camera.
    // This is for the depth of field extra-credit feature only;
    // You can ignore if you are not attempting to implement depth of field.
    float getAperture() const;

    // Return the position of the camera in world space.
    glm::vec4 getPos() const;

    float getWidth() const;
    float getHeight() const;


    void moveForward(float deltaTime);
    void moveBackward(float deltaTime);
    void moveLeft(float deltaTime);
    void moveRight(float deltaTime);
    void moveUp(float deltaTime);
    void moveDown(float deltaTime);

    void rotateCamera(float deltaX, float deltaY);
private:
    // speed of camera in world space units per second
    float m_speed = 5.f;
    // sensitivity of mouse for camera rotation
    float m_sensitivity = 0.01f;

    SceneCameraData m_cameraData;
    float m_aspectRatio;
    float m_width;
    float m_height;

    void computeViewMatrix();
    void computeProjMatrix();

    glm::mat3 getRotationMatrix(float angle, const glm::vec3& axis);

    glm::vec3 m_up;
    glm::vec3 m_look;
    glm::vec3 m_pos;

    // View matrix and its inverse
    glm::mat4 V;
    glm::mat4 V_inv;

    // Projection matrix and its inverse
    glm::mat4 Proj;
    glm::mat4 Proj_inv;

    // positions of the clipping planes
    float m_nearPlane;
    float m_farPlane;
};
