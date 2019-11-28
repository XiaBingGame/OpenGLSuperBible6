#version 420 core

in vec4 vVertex;
out vec4 color;
uniform mat4 mvpMatrix;

layout (binding = 0) uniform sampler1D grasspalette_texture;
layout (binding = 1) uniform sampler2D length_texture;
layout (binding = 2) uniform sampler2D orientation_texture;
layout (binding = 3) uniform sampler2D grasscolor_texture;
layout (binding = 4) uniform sampler2D bend_texture;

const uint lowbits = 0x3FF;
const int gridedgesize = 1024;
float gridedgehalfsize = float(gridedgesize) / 2.0f;

int  random(int seed, int iterations)
{
	int value = seed;
	int n;
	for(n = 0; n < iterations; n++) {
		value = ((value >> 7) ^ (value << 9)) * 15485863;
	}

	return value;
}

vec4 random_vector(int seed)
{
	int r = random(gl_InstanceID, 4);
	int g = random(r, 2);
	int b = random(g, 2);
	int a = random(b, 2);

	return vec4(float(r & lowbits) / float(gridedgesize),
				float(g & lowbits) / float(gridedgesize),
				float(b & lowbits) / float(gridedgesize),
				float(a & lowbits) / float(gridedgesize));
}

mat4 construct_rotation_matrix(float angle)
{
	float st = sin(angle);
	float ct = cos(angle);

	return mat4(vec4(ct, 0.0f, st, 0.0f),
				vec4(0.0f, 1.0f, 0.0f, 0.0f),
				vec4(-st, 0.0f, ct, 0.0f),
				vec4(0.0f, 0.0f, 0.0f, 1.0f));
}
            
void main(void)
{
	vec4 offset = vec4(float(gl_InstanceID >> 10) - gridedgehalfsize,
						0.0f,
						float(gl_InstanceID & lowbits) - gridedgehalfsize,
						0.0f);
	int number1 = random(gl_InstanceID, 3);
	int number2 = random(number1, 2);

	offset += vec4(float(number1 & 0xFF) / 256.0f,
					0.0f,
					float(number2 & 0xFF) / 256.0f,
					0.0f);
	vec2 texcoord = offset.xz / float(gridedgesize) + vec2(0.5f);

	float bend_factor = texture(bend_texture, texcoord).r * 2.0f;
	float bend_amount = cos(vVertex.y);
	
	float angle = texture(orientation_texture, texcoord).r * 2.0 * 3.141592;
	mat4 rot = construct_rotation_matrix(angle);

	vec4 position = (rot * (vVertex + vec4(0.0f, 0.0f, bend_amount * bend_factor, 0.0f))) + offset;
	
    gl_Position = mvpMatrix * position;

	int texSize = 255;
	color = texture(grasspalette_texture, texture(grasscolor_texture, texcoord).r);
		//+ vec4(random_vector(gl_InstanceID).xyz * vec3(0.1, 0.5, 0.1), 1.0);
}
