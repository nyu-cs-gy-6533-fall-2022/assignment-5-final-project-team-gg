#version 330 core

out vec4 outColor;

in vec2 Texcoord;

uniform vec3 lightPos;
uniform vec3 lightParams;
uniform vec3 camPos;

uniform sampler2D renderedTex;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColor;
uniform sampler2D gDepth;

uniform int task;
uniform vec2 resolution;

uniform bool vignette;
uniform float vIntensity;
uniform float vAmount;
uniform bool luminance;
uniform vec3 lumWeight;
uniform bool brightness;
uniform float bAmount;
uniform bool contrast;
uniform float cAmount;
uniform bool exposure;
uniform float eAmount;
uniform bool gamma;
uniform float gAmount;
uniform bool negative;
uniform bool saturation;
uniform vec3 satWeight;
uniform float sAmount;

float linearize_depth(float depth);

vec4 Vignette(vec4 color);
vec4 Luminance(vec4 color);
vec4 Brightness(vec4 color);
vec4 Contrast(vec4 color);
vec4 Exposure(vec4 color);
vec4 Gamma(vec4 color);
vec4 Negative(vec4 color);
vec4 Saturation(vec4 color);

void main()
{
    if (task == 2) {
        outColor = texture(renderedTex, Texcoord) * vec4(1.0); 
        //outColor = vec4(1.0, 1.0, 1.0, 1.0);   
    }
    else if (task == 3 || task == 4) {
        vec3 pos = texture(gPosition, Texcoord).rgb;
        vec3 normal = texture(gNormal, Texcoord).rgb;
        vec3 color = texture(gColor, Texcoord).rgb;
        vec3 depth = texture(gDepth, Texcoord).rgb;

        vec3 viewDir = normalize(camPos - pos);
        vec3 lightDir = normalize(lightPos - pos);
        vec3 reflectDir = normalize(reflect(-lightDir, normal));

        vec3 amb = color * lightParams.x;
        vec3 diff = color * max(dot(normal, lightDir), 0.0);
        vec3 spec = vec3(1.0) * pow(max(dot(viewDir, reflectDir), 0.0), lightParams.y);

        vec3 lighting = clamp(amb+diff+spec, 0.0, 1.0);

        if (normal == vec3(0.0)) {
            outColor = vec4(vec3(0.5), 1.0);
        }
        else {
            outColor = vec4(lighting, 1.0);
        }
        //outColor = vec4(texture(gColor, Texcoord).xyz, 1.0);
        //outColor = vec4(vec3(depth), 1.0);
    }


    if (vignette) {
        outColor = Vignette(outColor);
    }
    if (luminance) {
        outColor = Luminance(outColor);
    }
    if (brightness) {
        outColor = Brightness(outColor);
    }
    if (contrast) {
        outColor = Contrast(outColor);
    }
    if (exposure) {
        outColor = Exposure(outColor);
    }
    if (gamma) {
        outColor = Gamma(outColor);
    }
    if (negative) {
        outColor = Negative(outColor);
    }
    if (saturation) {
        outColor = Saturation(outColor);
    }
}

float linearize_depth(float depth) {
    float near = 0.1;
    float far = 100.0;

    float ndcZ = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - ndcZ * (far - near));
}

vec4 Vignette(vec4 color) {
    vec2 uv = gl_FragCoord.xy / resolution;
    uv *= 1.0 - uv.yx;
    float vig = uv.x*uv.y * vIntensity;
    vig = pow(vig, vAmount);
    return color * vec4(vig);
}

vec4 Luminance(vec4 color) {
    float lum = dot(color.rgb, lumWeight);
    return vec4(vec3(lum), 1.0);
}

vec4 Brightness(vec4 color) {
    return color + bAmount;
}

vec4 Contrast(vec4 color) {
    return (color - 0.5) * cAmount*2.0 + 0.5;
}

vec4 Exposure(vec4 color) {
    return color * pow(2.0, eAmount);
}

vec4 Gamma(vec4 color) {
    return pow(color, vec4(gAmount)*2.2);
}

vec4 Negative(vec4 color) {
    return 1.0 - color;
}

vec4 Saturation(vec4 color) {
    float satColor = dot(color.rgb, satWeight);
    return (vec4(satColor) * (1.0 - sAmount)) + (color * (sAmount));
}
