
#version 330 core
in vec2 uv;

uniform sampler2D u_texture;
uniform float threshold;

out vec4 fragColor;

void main()
{
    vec4 color = texture(u_texture, uv);
    // check whether fragment output is higher than threshold, if so output as brightness color
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    float w = smoothstep(threshold - 0.2, threshold + 0.2, brightness);
    fragColor = color * w;
}
