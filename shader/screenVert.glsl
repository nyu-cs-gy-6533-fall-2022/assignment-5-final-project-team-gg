#version 330 core

layout (location = 0) in vec2 quadPos;
layout (location = 1) in vec2 quadTexcoord;

out vec2 Texcoord;

void main()
{
    Texcoord = quadTexcoord;
    gl_Position = vec4(quadPos, 0.0, 1.0);
}

