#version 330 core

in vec2 uv;

uniform sampler2D u_texture;

uniform bool horizontal;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
uniform float radius = 2.5;

out vec4 fragColor;

void main() {
    vec2 tex_offset = 1.0 / textureSize(u_texture, 0); // gets size of single texel
    vec3 result = texture(u_texture, uv).rgb * weight[0]; // current fragment's contribution
    if (horizontal) {
        for (int i = 1; i < 5; ++i) {
            result += texture(u_texture, uv + vec2(tex_offset.x * i * radius, 0.0)).rgb * weight[i];
            result += texture(u_texture, uv - vec2(tex_offset.x * i * radius, 0.0)).rgb * weight[i];
        }
    } else {
        for (int i = 1; i < 5; ++i) {
            result += texture(u_texture, uv + vec2(0.0, tex_offset.y * i * radius)).rgb * weight[i];
            result += texture(u_texture, uv - vec2(0.0, tex_offset.y * i * radius)).rgb * weight[i];
        }
    }
    fragColor = vec4(result, 1.0);
}
