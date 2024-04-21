#version 330

out vec4 FragColor;
in vec3 outcol;
void main()
{
   gl_FragColor  = vec4(outcol, 1.0f);
} 