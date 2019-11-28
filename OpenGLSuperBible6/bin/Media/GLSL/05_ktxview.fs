#version 430 core

uniform sampler2D s;
uniform float exposure;
out vec4 color;
uniform vec2 viewport;

void main(void)
{
	int ix =  int(gl_FragCoord.x / 10);
	int iy =  int(gl_FragCoord.y / 10);
	vec4 c = vec4(0.0);
	for(int i = 0; i < 50; i++)
	{
		for(int j = 0; j < 50; j++)
		{
			c += texture(s, vec2(ix*10+i, iy*10+j) / textureSize(s, 0));
		}
	}
	c /= 2500;
	c.w = 1.0;
	int xoff = int(gl_FragCoord.x) - ix * 10;
	int yoff = int(gl_FragCoord.y) - iy * 10;
	if(xoff < 2 || yoff < 2)
		c = vec4(0.0, 0.0, 0.0, 1.0);
/*
	if(c.x > c.y && c.x > c.z)
		c = vec4(1.0, 0.0, 0.0, 1.0);
	if(c.y > c.x && c.y > c.z)
		c = vec4(0.0, 1.0, 0.0, 1.0);
	if(c.z > c.y && c.z > c.x)
		c = vec4(0.0, 0.0, 1.0, 1.0);
	if(c.x < 0.10 && c.y < 0.10 && c.z < 0.10)
		c = vec4(0.0, 0.0, 0.0, 1.0);
	if(c.x > 0.90 && c.y > 0.90 && c.z > 0.90)
		c = vec4(0.0, 0.0, 0.0, 1.0);
*/

    color = c;
    //color = texture(s, gl_FragCoord.xy / viewport) * exposure;
	//color = texture(s, gl_FragCoord.xy);
	//color = texelFetch(s, ivec2(gl_FragCoord.xy * textureSize(s, 0)/viewport), 0);
}
