#version 330 core

in vec4 posWorldSpace;
in vec3 normalWorldSpace;
in vec4 shadowCoords[8];
in float eyeDepth;

in vec2 uv;
in mat3 TBN;

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

// texture related uniform
struct ShapeTexture {
    sampler2D  textureSampler;
    bool textureIsUsed;
    vec2 textureRepeat;
};
uniform ShapeTexture myTextures;
uniform ShapeTexture myNormals;
uniform ShapeTexture myBumps;
uniform float blend;

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

// texture helper functions
vec4 blendDiffuseWithText();
vec3 getNormalValue();

void main() {
    vec4 dirToCam = normalize(cameraPos - posWorldSpace);
    vec4 illumination = ka * cAmbient;
    vec3 normal = getNormalValue();

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


        //float NdotL = dot(normalize(normalWorldSpace), vec3(dirToLight));
        float NdotL = dot(normal, vec3(dirToLight));
        if (NdotL < 0) continue;

        //vec4 diffuseTerm = kd * cDiffuse * NdotL;
        vec4 diffuse = blendDiffuseWithText();
        vec4 diffuseTerm = diffuse * NdotL;

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

vec4 blendDiffuseWithText(){

    vec4 diffuse = kd * cDiffuse;

    if(myTextures.textureIsUsed && blend != 0.0f){
        vec2 uvRepeat = uv * myTextures.textureRepeat;
        vec4 textureColor = texture(myTextures.textureSampler, uvRepeat);
        return (1.f - blend) * diffuse + blend * textureColor;
    }
    else{
        return diffuse;
    }
}

vec3 getNormalValue(){
    vec3 normal = normalize(normalWorldSpace);

    // normal mapping take priority if both mapping are used somehow...
    if(myNormals.textureIsUsed){
        vec3 normalMap = texture(myNormals.textureSampler, uv * myNormals.textureRepeat).xyz;
        normalMap = normalMap * 2.0 - 1.0;
        normal = normalize(TBN * normalMap);
    }
    else if(myBumps.textureIsUsed){
        vec2 uvRepeat = uv * myBumps.textureRepeat;
        ivec2 bumpTextureSize = textureSize(myBumps.textureSampler, 0);
        vec2 texel = 1.0 / vec2(bumpTextureSize);

        float height = texture(myBumps.textureSampler, uvRepeat).x;

        // height from neighboring pixels
        float heightRight = texture(myBumps.textureSampler, uvRepeat + vec2(texel.x, 0.0)).x;
        float heightUp = texture(myBumps.textureSampler, uvRepeat + vec2(0.0, texel.y)).x;

        // finite difference to approximate partial derivatives of Height(u, v) or the height
        float dHdu = heightRight - height;
        float dHdv = heightUp - height;

        float bumpScale = 3.0f;
        dHdu *= bumpScale;
        dHdv *= bumpScale;

        vec3 dpdu = vec3(1.0, 0.0, 0.0);
        vec3 dpdv = vec3(0.0, 1.0, 0.0);
        vec3 n = vec3(0.0, 0.0, 1.0);

        vec3 perturbedPtU = dpdu + dHdu * n; //dpdu + dHdu * n + h * dndu, dndu = 0
        vec3 perturbedPtV = dpdv + dHdv * n; //dpdv + dHdv * n + h * dndv, dndv = 0

        vec3 normalBump = normalize(cross(perturbedPtU, perturbedPtV));
        normal = normalize(TBN * normalBump);
    }

    return normal;
}
