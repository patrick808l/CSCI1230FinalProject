#version 330 core

// input vertex data, different for all executions of this shader
layout(location = 0) in vec3 posObjSpace;

// values that stay constant for the whole mesh

uniform mat4 depthProjMatrix;
uniform mat4 depthViewMatrix;
uniform mat4 modelMatrix;

void main() {
    mat4 depthMVP = depthProjMatrix * depthViewMatrix * modelMatrix;
    gl_Position = depthMVP * vec4(posObjSpace, 1);
}
