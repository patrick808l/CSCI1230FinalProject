#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 in_uv;

//"out" variable representing a UV coordinate
out vec2 uv;

void main() {
    uv = in_uv;

    gl_Position = vec4(position, 1.0);
}
