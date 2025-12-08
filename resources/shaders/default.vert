#version 330 core

layout(location = 0) in vec3 posObjSpace;
layout(location = 1) in vec3 normalObjSpace;
layout(location = 2) in vec2 uvIn;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;
layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;

out vec4 posWorldSpace;
out vec3 normalWorldSpace;
// positions in perspective light spaces
out vec4 shadowCoords[8];
// distance from camera in camera space
out float eyeDepth;

// texture uv coordinate and Tangent-Bitangent-Normal matrix
out vec2 uv;
out mat3 TBN;

uniform mat4 modelMatrix, viewMatrix, projectionMatrix;

// bias * projection * view matrices for up to 8 shadow maps
uniform mat4 depthBiasVPs[8];

// skeletal animation
const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];


void main() {
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


    posWorldSpace = modelMatrix * totalPosition;

    mat3 modelInvTranspose = inverse(transpose(mat3(modelMatrix)));
    normalWorldSpace = modelInvTranspose * normalObjSpace;

    for (int i = 0; i < 8; i++) {
        shadowCoords[i] = depthBiasVPs[i] * posWorldSpace;
    }

    vec4 viewPos = viewMatrix * posWorldSpace;
    eyeDepth = -viewPos.z;

    gl_Position = projectionMatrix * viewPos;


    uv = uvIn;
    vec3 tangentWorldSpace = normalize(modelInvTranspose * tangent);
    //Orthogonalization to make the tangent perpendicular to the normal
    tangentWorldSpace = normalize( tangentWorldSpace - normalWorldSpace * dot(normalWorldSpace, tangentWorldSpace) );
    vec3 bitanWorldSpace = normalize(cross(normalWorldSpace, tangentWorldSpace));
    TBN = mat3(tangentWorldSpace, bitanWorldSpace, normalWorldSpace);
}
