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

class prefixsum2d_app : public OpenGLApp
{
public:
	prefixsum2d_app() : prefix_sum_prog(0) {}

	void init()
	{
		static const char title[] = "OpenGL SuperBible - 2D Prefix Sum";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		int i;

		glGenTextures(3, images);

		images[0] =  loadKTX("../media/textures/star.ktx");

		for (i = 1; i < 3; i++)
		{
			glBindTexture(GL_TEXTURE_2D, images[i]);
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, NUM_ELEMENTS, NUM_ELEMENTS);
		}

		glGenVertexArrays(1, &dummy_vao);
		glBindVertexArray(dummy_vao);

		load_shaders();
	}

	virtual void render(double currentTime)
	{
		glUseProgram(prefix_sum_prog);

		glBindImageTexture(0, images[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
		glBindImageTexture(1, images[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

		glDispatchCompute(NUM_ELEMENTS, 1, 1);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glBindImageTexture(0, images[1], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
		glBindImageTexture(1, images[2], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

		glDispatchCompute(NUM_ELEMENTS, 1, 1);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glBindTexture(GL_TEXTURE_2D, images[2]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, images[2]);

		glUseProgram(show_image_prog);

		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glBindVertexArray(dummy_vao);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	virtual void shutdown()
	{
		glDeleteTextures(3, images);
		glDeleteProgram(prefix_sum_prog);
		glDeleteProgram(show_image_prog);
		glDeleteVertexArrays(1, &dummy_vao);
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
	GLuint images[3];

	GLuint  prefix_sum_prog;
	GLuint  show_image_prog;
	GLuint  dummy_vao;
};

void prefixsum2d_app::prefix_sum(const float * input, float * output, int elements)
{
	float f = 0.0f;
	int i;

	for (i = 0; i < elements; i++)
	{
		f += input[i];
		output[i] = f;
	}
}

void prefixsum2d_app::load_shaders()
{
	if (prefix_sum_prog)
		glDeleteProgram(prefix_sum_prog);

	prefix_sum_prog = LoadShadersCS("../media/glsl/10_prefixsum2d/prefixsum2d.cs");

	if(show_image_prog)
		glDeleteProgram(show_image_prog);
	show_image_prog = LoadShadersVF("../media/glsl/10_prefixsum2d/showimage.vs", "../media/glsl/10_prefixsum2d/showimage.fs");
}

int main(int argc, char** argv)
{
	prefixsum2d_app a;
	a.run();
	return 0;
}