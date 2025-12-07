#version 330 core

//Create a UV coordinate in variable
in vec2 uv;

uniform sampler2D u_texture;

out vec4 fragColor;

void main() {
    //Set fragColor using the sampler2D at the UV coordinate
    fragColor = texture(u_texture, uv);
}
