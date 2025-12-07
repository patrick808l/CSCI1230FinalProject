#version 330 core

in vec2 uv;

uniform sampler2D u_texture;
uniform float exposure;

out vec4 fragColor;

void main() {
    // const float gamma = 2.2;
    vec3 hdrColor = texture(u_texture, uv).rgb;

    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    // gamma correction removed
    // mapped = pow(mapped, vec3(1.0 / gamma));

    fragColor = vec4(mapped, 1.0);
}
