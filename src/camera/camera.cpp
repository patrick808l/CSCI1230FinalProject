#include <iostream>
#include "camera.h"

/**
 * @brief initialize camera by storing SceneCameraData and scene dimensions.
 * compute the view and projection matrices.
 */
void Camera::init(SceneCameraData camData, float width, float height) {
    m_cameraData = camData;
    m_width = width;
    m_height = height;
    m_aspectRatio = (float) width / height;

    m_up = m_cameraData.up;
    m_look = m_cameraData.look;
    m_pos = m_cameraData.pos;

    computeViewMatrix();
    computeProjMatrix();
}

/**
 * @brief store camera data in this class. recompute the view and projection matrices
 * @param camData parsed from the scene file
 */
void Camera::updateCamData(SceneCameraData camData) {
    m_cameraData = camData;

    m_up = m_cameraData.up;
    m_look = m_cameraData.look;
    m_pos = m_cameraData.pos;

    computeViewMatrix();
    computeProjMatrix();
}

/**
 * @brief compute the view matrix using the look, up, and pos vectors
 */
void Camera::computeViewMatrix() {
    glm::vec3 w = -glm::normalize(m_look);
    glm::vec3 v = glm::normalize(m_up - glm::dot(m_up, w) * w);
    glm::vec3 u = glm::cross(v, w);

    // column major entries
    glm::mat4 R = glm::mat4(
        u[0], v[0], w[0], 0.f,
        u[1], v[1], w[1], 0.f,
        u[2], v[2], w[2], 0.f,
        0.f, 0.f, 0.f, 1.f
        );
    glm::mat4 T = glm::mat4(
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        -m_pos[0], -m_pos[1], -m_pos[2], 1.f
        );

    V = R * T;
}

/**
 * @brief update the near and far planes.
 * recompute the projection matrix.
 */
void Camera::updatePlanes(float nearPlane, float farPlane) {
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;

    computeProjMatrix();
}

/**
 * @brief compute the projection matrix using the clipping planes, aspect ratio, and height angle
 */
void Camera::computeProjMatrix() {
    float c = -m_nearPlane / m_farPlane;
    glm::mat4 M_pp{
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f / (1.f + c), -1.f,
        0.f, 0.f, -c / (1.f + c), 0.f
    };

    float widthAngle = 2 * glm::atan(m_aspectRatio * glm::tan(m_cameraData.heightAngle / 2.f));
    glm::mat4 S_xyz{
        1.f / (m_farPlane * glm::tan(widthAngle / 2.f)), 0.f, 0.f, 0.f,
        0.f, 1.f / (m_farPlane * glm::tan(m_cameraData.heightAngle / 2.f)), 0.f, 0.f,
        0.f, 0.f, 1.f / m_farPlane, 0.f,
        0.f, 0.f, 0.f, 1.f
    };

    glm::mat4 remapZ{
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, -2.f, 0.f,
        0.f, 0.f, -1.f, 1.f
    };

    Proj = remapZ * M_pp * S_xyz;
}


glm::mat4 Camera::getViewMatrix() const {
    return V;
}

glm::mat4 Camera::getProjMatrix() const {
    return Proj;
}

float Camera::getAspectRatio() const {
    return m_aspectRatio;
}

float Camera::getHeightAngle() const {
    return m_cameraData.heightAngle;
}

float Camera::getFocalLength() const {
    return m_cameraData.focalLength;
}

float Camera::getAperture() const {
    return m_cameraData.aperture;
}

glm::vec4 Camera::getPos() const {
    return glm::vec4{m_pos, 1.f};
}

float Camera::getWidth() const {
    return m_width;
}

float Camera::getHeight() const {
    return m_height;
}




void Camera::moveForward(float deltaTime) {
    m_pos += m_speed * glm::normalize(m_look) * deltaTime;
    computeViewMatrix();
}

void Camera::moveBackward(float deltaTime) {
    m_pos -= m_speed * glm::normalize(m_look) * deltaTime;
    computeViewMatrix();
}

void Camera::moveLeft(float deltaTime) {
    m_pos -= m_speed * glm::normalize(glm::cross(m_look, m_up)) * deltaTime;
    computeViewMatrix();
}

void Camera::moveRight(float deltaTime) {
    m_pos += m_speed * glm::normalize(glm::cross(m_look, m_up)) * deltaTime;
    computeViewMatrix();
}

void Camera::moveUp(float deltaTime) {
    m_pos += m_speed * glm::vec3{0, 1, 0} * deltaTime;
    computeViewMatrix();
}

void Camera::moveDown(float deltaTime) {
    m_pos -= m_speed * glm::vec3{0, 1, 0} * deltaTime;
    computeViewMatrix();
}

/**
 * @brief use rodrigues formula to construct a rotation matrix for given angle about given axis
 */
glm::mat3 Camera::getRotationMatrix(float angle, const glm::vec3& axis) {
    float cos = glm::cos(angle);
    float sin = glm::sin(angle);
    return glm::mat3{
        cos + axis.x*axis.x*(1-cos), axis.x*axis.y*(1-cos) + axis.z*sin, axis.x*axis.z*(1-cos) - axis.y*sin,
        axis.x*axis.y*(1-cos) - axis.z*sin, cos + axis.y*axis.y*(1-cos), axis.y*axis.z*(1-cos) + axis.x*sin,
        axis.x*axis.z*(1-cos) + axis.y*sin, axis.y*axis.z*(1-cos) - axis.x*sin, cos + axis.z*axis.z*(1-cos)
    };
}

/**
 * @brief rotate the camera by applying rotation matrices to the look and up vectors.
 * @param deltaX is the change in mouse x position since the previous timestep
 * @param deltaY is the change in mouse y position since the previous timestep
 */
void Camera::rotateCamera(float deltaX, float deltaY) {
    glm::mat3 rotX = getRotationMatrix(deltaX * m_sensitivity, glm::vec3{0,-1,0});
    glm::mat3 rotY = getRotationMatrix(deltaY * m_sensitivity, -glm::normalize(glm::cross(m_look, m_up)));

    m_look = rotX * rotY * m_look;
    m_up = rotX * rotY * m_up;
    computeViewMatrix();
}
