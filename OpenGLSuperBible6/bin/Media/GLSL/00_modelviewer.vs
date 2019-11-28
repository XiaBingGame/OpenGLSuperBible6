#version 430 core
layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;

uniform mat4 mv_matrix;
uniform mat4 mvproj_matrix;
uniform vec3 light_direction;

out VS_OUT
{
	vec4 color;
} vs_out;

void main(void)
{
	gl_Position = mvproj_matrix*position;

	// 光照方向
	vec3 L = light_direction;
	vec4 diffColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	vec4 ambColor = vec4(vec3(0.2), 1.0f);

	vec3 N = normalize(mat3(mv_matrix)*normal);

	vs_out.color = clamp(max(dot(N, L), 0.0) * diffColor + ambColor, 0.0f, 1.0f);
}
