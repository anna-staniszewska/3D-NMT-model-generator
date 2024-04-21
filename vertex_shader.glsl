#version 330 

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 incol;
layout (location = 2) in vec3 incol2;
layout (location = 3) in vec3 aNormal;
layout (location = 4) in vec2 vtexcoord0;


out vec3 FragPos;
out vec3 outcol;
out vec3 outcol2;
out vec3 Normal;
out vec2 texcoord0;

uniform mat4 MVP;

void main()
{
    FragPos = aPos;
    Normal = aNormal;  
    outcol = incol;
    outcol2 = incol2;
    texcoord0 = vtexcoord0;

    gl_Position = MVP * vec4(aPos, 1.0);
}