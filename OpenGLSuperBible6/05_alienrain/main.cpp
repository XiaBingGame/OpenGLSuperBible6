#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

/** \addtogroup Chapter05*   @{*       \addtogroup alienrain** ʹ�� uniform block. ��ʹ����������, ��������ѡ��ͬ�ĵ�������.*	- ���� float random_float() ���� 0.0 �� 1.0 �����������.*	- ����ʹ�� 2D ��������. ��������������64��2D����.*	- ���ɵ� Buffer �� GL_UNIFORM_BUFFER, ���� uniform buffer �󶨵�.*	- ʹ�� glBindBufferBase �� GL_UNIFORM_BUFFER �� buffer ����һ�� uniform buffer �󶨵�.
*	- ʹ�� glMapBufferRange ӳ��һ�η�Χ�� buffer.
*	- ʹ���� glVertexAttrib*() �� glVertexAttribI*() �� li ����.*	- ��ɫ����, ʹ�� uniform block ������ 256 ��С������, ��������ȡ������һ��������Ⱦ*       @{*/


// Random number generator
static unsigned int seed = 0x13371337;

/*
 * �������һ�����������, �� 0.0 �� 1.0.
 */

static inline float random_float()
{
	float res;
	unsigned int tmp;

	seed *= 16807;
	tmp = seed ^ (seed >> 4) ^ (seed << 15);
	*((unsigned int *) &res) = (tmp >> 9) | 0x3F800000;

	return (res - 1.0f);
}

class alienrain_app : public OpenGLApp
{
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Alien Rain";

		OpenGLApp::init();

		memcpy(info.title, title, sizeof(title));
	}

	void startup()
	{
		render_prog = LoadShadersVF("../media/glsl/05_alienrain.vs", "../media/glsl/05_alienrain.fs");
		glGenVertexArrays(1, &render_vao);
		glBindVertexArray(render_vao);

		tex_alien_array = loadKTX("../media/textures/aliens.ktx", 0);

		glBindTexture(GL_TEXTURE_2D_ARRAY, tex_alien_array);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		// ���ɵ� Buffer �� GL_UNIFORM_BUFFER, ���� uniform buffer �󶨵�.
		glGenBuffers(1, &rain_buffer);
		glBindBuffer(GL_UNIFORM_BUFFER, rain_buffer);
		glBufferData(GL_UNIFORM_BUFFER, 256 * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);

		for (int i = 0; i < 256; i++)
		{
			droplet_x_offset[i] = random_float() * 2.0f - 1.0f;
			droplet_rot_speed[i] = (random_float() + 0.5f) * ((i & 1) ? -3.0f : 3.0f);
			droplet_fall_speed[i] = random_float() + 0.2f;
		}

		glBindVertexArray(render_vao);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	void render(double currentTime)
	{
		static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		float t = (float)currentTime;

		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glClearBufferfv(GL_COLOR, 0, black);

		glUseProgram(render_prog);

		// ʹ�� glBindBufferBase �� GL_UNIFORM_BUFFER �� buffer ����һ�� uniform buffer �󶨵�.
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, rain_buffer);
		// ʹ�� glMapBufferRange ӳ��һ�η�Χ�� buffer.
		glm::vec4 * droplet = (glm::vec4 *)glMapBufferRange(GL_UNIFORM_BUFFER, 0, 256 * sizeof(glm::vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

		for (int i = 0; i < 256; i++)
		{
			droplet[i][0] = droplet_x_offset[i];
			droplet[i][1] = 2.0f - fmodf((t + float(i)) * droplet_fall_speed[i], 4.31f);
			droplet[i][2] = t * droplet_rot_speed[i];
			droplet[i][3] = 0.0f;
		}
		glUnmapBuffer(GL_UNIFORM_BUFFER);

		// ʹ���� glVertexAttrib*() �� glVertexAttribI*() �� li ����.
		int alien_index;
		for (alien_index = 0; alien_index < 256; alien_index++)
		{
			glVertexAttribI1i(0, alien_index);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
	}

protected:
	GLuint          render_prog;
	GLuint          render_vao;

	GLuint          tex_alien_array;
	GLuint          rain_buffer;

	float           droplet_x_offset[256];
	float           droplet_rot_speed[256];
	float           droplet_fall_speed[256];
};

/** @} @} */

int main(int argc, char** argv)
{
	alienrain_app a;
	a.run();
	return 0;
}