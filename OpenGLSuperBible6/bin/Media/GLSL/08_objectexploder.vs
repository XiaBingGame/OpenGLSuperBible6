#version 430 core
in vec4 vVertex;
in vec3 vNormal;

out VS_OUT
{
	vec3 normal;
	vec4 color;
} vertex;

uniform mat4 mvMatrix;
uniform mat4 projMatrix;

void main(void)
{
	gl_Position = projMatrix * mvMatrix * vVertex;
	vertex.color = vVertex * 2.0 + vec4(0.5, 0.5, 0.5, 0.0);
	vertex.normal = normalize(mat3(mvMatrix) * vNormal);
}
