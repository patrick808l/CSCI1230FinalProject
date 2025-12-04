#version 330 core

in vec4 posWorldSpace;
in vec3 normalWorldSpace;
in vec4 shadowCoords[8];
in float eyeDepth;

out vec4 fragColor;

uniform float ka, kd, ks, shininess;
uniform vec4 cAmbient, cDiffuse, cSpecular;
uniform vec4 cameraPos;

uniform bool fogEnabled;
const float fog_maxdist = 10.f;
const float fog_mindist = 0.1f;
const vec4 fog_color = vec4(0.4f, 0.4f, 0.4f, 1.f);
const float fog_density = 0.2f;

uniform bool shadowsEnabled;
uniform sampler2D depthTextures[8];

uniform int numLights;

/*
 * lightType: 0 = point light
 *            1 = direction light
 *            2 = spot light
 * pos: light position, defined for point lights and spot lights
 * dir: light direction, defined for direction lights and spot lights
 * attenCoeff: attenuation coefficients, defined for point lights and spot lights
 * angle: the total angle of a spot light
 * penumbra: the angle where dropoff takes place, defined for spot lights
 */
struct Light{
    int lightType;
    vec4 pos;
    vec4 dir;
    vec4 color;
    vec3 attenCoeff;
    float angle;
    float penumbra;
};
uniform Light lights[8];


const float bias = 0.01;
const float shadowVisibility = 0.5;

void main() {
    vec4 dirToCam = normalize(cameraPos - posWorldSpace);
    vec4 illumination = ka * cAmbient;

    for (int i = 0; i < numLights; i++) {
        float visibility = 1.0;

        float distToLight;
        float attenFactor;
        vec4 dirToLight;

        switch(lights[i].lightType) {
        case 0: // point light
            distToLight = length(posWorldSpace - lights[i].pos);
            attenFactor = min(1, 1 / (lights[i].attenCoeff[0] + (distToLight * lights[i].attenCoeff[1]) + (distToLight*distToLight * lights[i].attenCoeff[2])));
            dirToLight = normalize(lights[i].pos - posWorldSpace);
            break;
        case 1: // directional light
            attenFactor = 1.0;
            dirToLight = -normalize(lights[i].dir);

            if (shadowsEnabled) {
                if (texture( depthTextures[i], shadowCoords[i].xy ).r < shadowCoords[i].z - bias) {
                    visibility = shadowVisibility;
                }
            }
            break;
        case 2: // spot light
            distToLight = length(posWorldSpace - lights[i].pos);
            attenFactor = min(1, 1 / (lights[i].attenCoeff[0] + (distToLight * lights[i].attenCoeff[1]) + (distToLight*distToLight * lights[i].attenCoeff[2])));
            dirToLight = normalize(lights[i].pos - posWorldSpace);
            // angle between current direction and spotlight direction
            float x = acos(dot(-dirToLight, normalize(lights[i].dir)));

            float outerAngle = lights[i].angle;
            float innerAngle = outerAngle - lights[i].penumbra;
            float angleRatio = (x - innerAngle) / (outerAngle - innerAngle);
            float falloff = -2 * angleRatio*angleRatio*angleRatio + 3 * angleRatio*angleRatio;

            if (x <= innerAngle) {
                // do nothing more to attenuation
            } else if (innerAngle < x && x <= outerAngle) {
                // decrease attenuation factor by considering falloff
                attenFactor *= 1 - falloff;
            } else {
                // receieved intensity is zero
                attenFactor = 0;
            }

            if (shadowsEnabled) {
                if (texture( depthTextures[i], (shadowCoords[i].xy / shadowCoords[i].w) ).r < (shadowCoords[i].z - bias) / shadowCoords[i].w) {
                    visibility = shadowVisibility;
                }
            }
            break;
        default:
            break;
        }

        float NdotL = dot(normalize(normalWorldSpace), vec3(dirToLight));
        if (NdotL < 0) continue;

        vec4 diffuseTerm = kd * cDiffuse * NdotL;

        vec3 reflectedLightDir = reflect(vec3(-dirToLight), normalize(normalWorldSpace));
        float RdotV = dot(reflectedLightDir, vec3(dirToCam));
        vec4 specularTerm;
        if (RdotV < 0 || (RdotV == 0 && shininess <= 0)) {
            // avoid undefined behavior for pow
            specularTerm = vec4(0, 0, 0, 0);
        } else {
            specularTerm = ks * cSpecular * pow(clamp(RdotV, 0, 1), shininess);
        }

        illumination += visibility * (attenFactor * lights[i].color) * (diffuseTerm + specularTerm);
    }

    float fog_factor = 1.f;
    if (fogEnabled) {
        fog_factor = (fog_maxdist - eyeDepth) / (fog_maxdist - fog_mindist);
        // fog_factor = exp(-fog_density * dist);
        fog_factor = clamp(fog_factor, 0.f, 1.f);
    }

    fragColor = mix(fog_color, illumination, fog_factor);
}
