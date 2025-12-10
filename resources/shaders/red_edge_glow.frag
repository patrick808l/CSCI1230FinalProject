#version 330 core
in vec2 uv;
out vec4 fragColor;

uniform sampler2D u_texture;

float u_intensity = 0.07;
float u_radius = 0.12;
float u_softness = 1.0;
float u_roundness = 0.2;

// distance to a rounded rectangle centered at 0.5,0.5
float sdRoundRect(vec2 uv, float r) {
    vec2 p = uv - vec2(0.5);

    vec2 b = vec2(0.5 - r); //size before rounding

    vec2 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;
}

void main()
{
    vec3 color = texture(u_texture, uv).rgb;

        // distance
        float sd = sdRoundRect(uv, u_roundness);

        // distance inward from the rounded border
        float inward = max(0.0, -sd);

        // linear fade across u_radius thickness
        float edgeLinear = clamp((u_radius - inward) / max(u_radius, 1e-6), 0.0, 1.0);

        float edge = edgeLinear;

        vec3 glowColor = vec3(1.0, 0.0, 0.0);
        color = mix(color, glowColor, edge * u_intensity);

        fragColor = vec4(color, 1.0);
}
