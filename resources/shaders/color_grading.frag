#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D u_texture;

uniform int u_enabled;
uniform float u_contrast;
uniform float u_saturation;
uniform float u_gamma;
uniform vec3 u_lift;
uniform vec3 u_gain;

vec3 applySaturation(vec3 c, float s) {
    float l = dot(c, vec3(0.2126, 0.7152, 0.0722));
    return mix(vec3(l), c, s);
}

void main() {
    vec3 c = texture(u_texture, TexCoords).rgb;
    if (u_enabled == 0) {
        FragColor = vec4(c, 1.0);
        return;
    }

    c = c + u_lift; // lift shadows
    c = c * u_gain; // gain highlights

    // contrast around mid-gray
    c = (c - 0.5) * u_contrast + 0.5;

    // saturation
    c = applySaturation(c, u_saturation);

    // gamma
    c = pow(max(c, 0.0), vec3(1.0 / max(u_gamma, 0.001)));

    FragColor = vec4(clamp(c, 0.0, 1.0), 1.0);
}
