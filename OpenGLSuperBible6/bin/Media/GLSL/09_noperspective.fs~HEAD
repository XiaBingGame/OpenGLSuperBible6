#version 420 core

out vec4 color;

uniform sampler2D tex_checker;
uniform bool use_perspective = true;

in VS_OUT
{
	vec2 tc;
	noperspective vec2 np_tc;
} fs_in;
                                                                  
void main(void)
{
	vec2 tc = mix(fs_in.np_tc, fs_in.tc, bvec2(use_perspective));
    color = texture(tex_checker, tc).rrrr;
}
