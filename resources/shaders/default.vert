#version 330 core

layout(location = 0) in vec3 posObjSpace;
layout(location = 1) in vec3 normalObjSpace;

out vec4 posWorldSpace;
out vec3 normalWorldSpace;
// positions in perspective light spaces
out vec4 shadowCoords[8];
// distance from camera in camera space
out float eyeDepth;

uniform mat4 modelMatrix, viewMatrix, projectionMatrix;

// bias * projection * view matrices for up to 8 shadow maps
uniform mat4 depthBiasVPs[8];

void main() {
    posWorldSpace = modelMatrix * vec4(posObjSpace, 1.0);

    mat3 modelInvTranspose = inverse(transpose(mat3(modelMatrix)));
    normalWorldSpace = modelInvTranspose * normalObjSpace;

    for (int i = 0; i < 8; i++) {
        shadowCoords[i] = depthBiasVPs[i] * posWorldSpace;
    }

    vec4 viewPos = viewMatrix * posWorldSpace;
    eyeDepth = -viewPos.z;

    gl_Position = projectionMatrix * viewPos;
}
