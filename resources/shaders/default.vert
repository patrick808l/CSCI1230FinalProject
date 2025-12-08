#version 330 core

layout(location = 0) in vec3 posObjSpace;
layout(location = 1) in vec3 normalObjSpace;
layout(location = 2) in vec2 uvIn;
layout(location = 3) in vec3 tangent;

out vec4 posWorldSpace;
out vec3 normalWorldSpace;
// distance from camera in camera space
out float eyeDepth;

// texture uv coordinate and Tangent-Bitangent-Normal matrix
out vec2 uv;
out mat3 TBN;

uniform mat4 modelMatrix, viewMatrix, projectionMatrix;

void main() {
    posWorldSpace = modelMatrix * vec4(posObjSpace, 1.0);

    mat3 modelInvTranspose = inverse(transpose(mat3(modelMatrix)));
    normalWorldSpace = modelInvTranspose * normalObjSpace;

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
