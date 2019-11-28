#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define NUM_ELEMENTS 2048

static inline float random_float()
{
	static unsigned int seed = 0x13371337;

	float res;
	unsigned int tmp;

	seed *= 16807;

	tmp = seed ^ (seed >> 4) ^ (seed << 15);

	*((unsigned int *) &res) = (tmp >> 9) | 0x3F800000;

	return (res - 1.0f);
}

class prefixsum_app : public OpenGLApp
{
public:
	prefixsum_app() : prefix_sum_prog(0) {}

	void init()
	{
		static const char title[] = "OpenGL SuperBible - 1D Prefix Sum";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		glGenBuffers(2, data_buffer);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, data_buffer[0]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_ELEMENTS * sizeof(float), NULL, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, data_buffer[1]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_ELEMENTS * sizeof(float), NULL, GL_DYNAMIC_COPY);

		int i;

		for (i = 0; i < NUM_ELEMENTS; i++)
		{
			input_data[i] = random_float();
		}

		prefix_sum(input_data, output_data, NUM_ELEMENTS);

		load_shaders();
	}

	virtual void render(double currentTime)
	{
		float * ptr;

		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, data_buffer[0], 0, sizeof(float) * NUM_ELEMENTS);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(float) * NUM_ELEMENTS, input_data);

		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, data_buffer[1], 0, sizeof(float) * NUM_ELEMENTS);

		glUseProgram(prefix_sum_prog);
		glDispatchCompute(1, 1, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		glFinish();

		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, data_buffer[1], 0, sizeof(float) * NUM_ELEMENTS);
		ptr = (float *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(float) * NUM_ELEMENTS, GL_MAP_READ_BIT);

		char buffer[1024];
		sprintf(buffer, "%2.2f %2.2f %2.2f %2.2f %2.2f %2.2f %2.2f %2.2f "
			"%2.2f %2.2f %2.2f %2.2f %2.2f %2.2f %2.2f %2.2f\n",
			ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7],
			ptr[8], ptr[9], ptr[10], ptr[11], ptr[12], ptr[13], ptr[14], ptr[15]);

		printf(buffer);

		//OutputDebugStringA(buffer);

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}

	virtual void shutdown()
	{
		glDeleteBuffers(2, data_buffer);
		glDeleteProgram(prefix_sum_prog);
	}

	void onKey(int key, int scancode, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			switch(key)
			{
			case 'R':   load_shaders();
				break;
			default:
				break;
			}
		}
		OpenGLApp::onKey(key, scancode, action, mods);
	}

private:
	void prefix_sum(const float * input, float * output, int elements);
	void load_shaders();

protected:
	GLuint  data_buffer[2];

	float input_data[NUM_ELEMENTS];
	float output_data[NUM_ELEMENTS];

	GLuint  prefix_sum_prog;
};

void prefixsum_app::prefix_sum(const float * input, float * output, int elements)
{
	float f = 0.0f;
	int i;

	for (i = 0; i < elements; i++)
	{
		f += input[i];
		output[i] = f;
	}
}

void prefixsum_app::load_shaders()
{
	if (prefix_sum_prog)
		glDeleteProgram(prefix_sum_prog);

	prefix_sum_prog = LoadShadersCS("../media/glsl/10_prefixsum/prefixsum.cs");

	glShaderStorageBlockBinding(prefix_sum_prog, 0, 0);
	glShaderStorageBlockBinding(prefix_sum_prog, 1, 1);
}

int main(int argc, char** argv)
{
	prefixsum_app a;
	a.run();
	return 0;
}