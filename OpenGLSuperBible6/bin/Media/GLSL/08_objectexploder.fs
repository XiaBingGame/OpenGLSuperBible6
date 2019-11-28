#version 430 core

in GS_OUT
{
	vec3 normal;
	vec4 color;
} fs_in;

out vec4 output_color;
                                                                  
void main(void)
{
	output_color = vec4(1.0) * abs(normalize(fs_in.normal).z);
}
