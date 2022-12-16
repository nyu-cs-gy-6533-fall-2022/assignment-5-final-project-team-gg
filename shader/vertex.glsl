#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;

out vec3 n;
out vec3 color;
out vec3 pos;
out vec2 Texcoord;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform vec3 triangleColor;

uniform int task;

void main()
{
    n = mat3(transpose(inverse(modelMatrix))) * normal;
    color = triangleColor;
    pos = vec3(modelMatrix * vec4(position, 1.0));
    Texcoord = texcoord;
    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}

