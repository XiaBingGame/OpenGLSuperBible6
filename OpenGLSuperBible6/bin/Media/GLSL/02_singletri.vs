#version 420 core

uniform float z;
void main(void)
{
    const vec4 vertices[] = vec4[](vec4( 0.25, -0.25, -1.0, 1.0),
                                   vec4(-0.25, -0.25, -1.0, 1.0),
                                   vec4( 0.25,  0.25, -1.0, 1.0));
            
    gl_Position = vertices[gl_VertexID];
	gl_Position.z = z;
}
