#version 330 core
in vec2 uv;

uniform sampler2D curr_texture;
uniform sampler2D u_texture;
uniform float exposure;

out vec4 fragColor;

void main() {
    fragColor = vec4((texture(curr_texture, uv).rgb + texture(u_texture, uv).rgb), 1.0);
}
