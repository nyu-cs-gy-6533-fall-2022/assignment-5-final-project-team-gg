#version 330 core

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec3 gPosition;
layout (location = 2) out vec3 gNormal;
layout (location = 3) out vec4 gColor;
layout (location = 4) out vec3 gDepth;

in vec3 n;
in vec3 color;
in vec3 pos;
in vec2 Texcoord;

uniform vec3 triangleColor;
uniform vec3 lightPos;
uniform vec3 lightParams;
uniform vec3 camPos;
uniform sampler2D globeTex;

uniform int task;

void main()
{
    vec3 col = color;
    vec3 normal = normalize(n);
    vec3 lightDir = normalize(lightPos - pos);

    if (task == 1 || task == 2) {
        //col = vec3(1.0, 1.0, 1.0);
        col = texture(globeTex, Texcoord).xyz;
        col = clamp( col * lightParams.x + 
            col * max(0.0, dot(normal, lightDir)) + 
            vec3(1.0) * pow(max(0.0, dot( normalize(camPos - pos), normalize( reflect(-lightDir, normal)))), lightParams.y),
            0.0, 1.0);

        //outColor = texture(globeTex, Texcoord) * vec4(col, 1.0);
        //outColor = texture(globeTex, Texcoord);
        //outColor = vec4(Texcoord, 1.0, 1.0);
        outColor = vec4(col, 1.0);

    }
    else if (task == 3) {
        outColor = texture(globeTex, Texcoord);
        gPosition = pos;
        gNormal = normalize(n);
        gColor = vec4(texture(globeTex, Texcoord).xyz, 1.0);
    }
    else if (task == 4) {
        outColor = vec4(col, 1.0);
        gPosition = pos;
        gNormal = normalize(n);
        gColor = vec4(col, 1.0);
        gDepth = vec3(gl_FragCoord.z);
    }
    else {
        col = clamp( triangleColor * lightParams.x + 
            triangleColor * max(0.0, dot(normal, lightDir)) + 
            vec3(1.0) * pow(max(0.0, dot( normalize(camPos - pos), normalize( reflect(-lightDir, normal)))), lightParams.y),
            0.0, 1.0);

        outColor = vec4(col, 1.0);
    }
}
