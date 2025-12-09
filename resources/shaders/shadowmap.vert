#version 330 core

// input vertex data, different for all executions of this shader
layout(location = 0) in vec3 posObjSpace;
layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;

// values that stay constant for the whole mesh

uniform mat4 depthProjMatrix;
uniform mat4 depthViewMatrix;
uniform mat4 modelMatrix;

uniform bool isSkeletalMesh;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

void main() {
    mat4 depthMVP = depthProjMatrix * depthViewMatrix * modelMatrix;

    if (isSkeletalMesh) {
        vec4 totalPosition = vec4(0.f);
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            if (boneIds[i] == -1) {
                continue;
            }
            if (boneIds[i] >= MAX_BONES) {
                totalPosition = vec4(posObjSpace, 1.f);
            }
            vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(posObjSpace, 1.f);
            totalPosition += localPosition * weights[i];
            // vec3 localNormal = mat3(finalBonesMatrices[boneIds[i]]) * normalObjSpace; ///?
        }
        gl_Position = depthMVP * totalPosition;
    } else {
        gl_Position = depthMVP * vec4(posObjSpace, 1);
    }
}
