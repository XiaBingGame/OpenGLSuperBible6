#version 420 core

out vec4 color;
uniform vec3 c;
                                                                  
void main(void)
{
    color = vec4(c, 1.0);
}
