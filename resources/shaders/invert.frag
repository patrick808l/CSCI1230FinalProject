#version 330 core

in vec2 uv;

uniform sampler2D u_texture;

out vec4 fragColor;

void main()
{
    fragColor = texture(u_texture, uv);
    fragColor.rgb = vec3(1.0) - fragColor.rgb;
}
