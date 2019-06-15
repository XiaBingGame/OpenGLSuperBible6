#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/** \addtogroup Chapter09*   @{*       \addtogroup starfield* 通过在着色器程序中计算点的z位置以及点的相应大小.**	- random_float() 得到随机数.*	- 允许 GL_PROGRAM_POINT_SIZE 表示想要设置为自己的点大小.*	- 顶点着色器中设置 gl_PointSize 为自己想要的点大小.*	- gl_PointCoord 可以得到相应的点的纹理坐标.*       @{*/

static unsigned int seed = 0x13371337;

static inline float random_float()
{
	float res;
	unsigned int tmp;

	seed *= 16807;

	tmp = seed ^ (seed >> 4) ^ (seed << 15);

	*((unsigned int *) &res) = (tmp >> 9) | 0x3F800000;

	return (res - 1.0f);
}

enum
{
	NUM_STARS           = 2000
};

class starfield_app : public OpenGLApp
{
public:
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Starfield";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		render_prog = LoadShadersVF("../media/glsl/09_starfield.vs", "../media/glsl/09_starfield.fs");
		uniforms.time = glGetUniformLocation(render_prog, "time");
		uniforms.proj_matrix = glGetUniformLocation(render_prog, "proj_matrix");
		star_texture = loadKTX("../media/textures/star.ktx");
		glGenVertexArrays(1, &star_vao);
		glBindVertexArray(star_vao);

		struct star_t
		{
			glm::vec3     position;
			glm::vec3     color;
		};

		// 生成一个 buffer, 其内设置了所有星星的位置和颜色. 而后设置为顶点属性0和1.
		glGenBuffers(1, &star_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, star_buffer);
		glBufferData(GL_ARRAY_BUFFER, NUM_STARS * sizeof(star_t), NULL, GL_STATIC_DRAW);

		star_t * star = (star_t *)glMapBufferRange(GL_ARRAY_BUFFER, 0, NUM_STARS * sizeof(star_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
		int i;

		for (i = 0; i < 1000; i++)
		{
			star[i].position[0] = (random_float() * 2.0f - 1.0f) * 100.0f;
			star[i].position[1] = (random_float() * 2.0f - 1.0f) * 100.0f;
			star[i].position[2] = random_float();
			star[i].color[0] = 0.8f + random_float() * 0.2f;
			star[i].color[1] = 0.8f + random_float() * 0.2f;
			star[i].color[2] = 0.8f + random_float() * 0.2f;
		}

		glUnmapBuffer(GL_ARRAY_BUFFER);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(star_t), NULL);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(star_t), (void *)sizeof(glm::vec3));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
	}

	virtual void render(double currentTime)
	{
		static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		static const GLfloat one[] = { 1.0f };
		float t = (float)currentTime;
		glm::mat4 proj_matrix = glm::perspective(glm::radians(50.0f),
			(float)info.windowWidth / (float)info.windowHeight,
			0.1f,
			1000.0f);

		t *= 0.1f;
		t -= floor(t);

		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glClearBufferfv(GL_COLOR, 0, black);
		glClearBufferfv(GL_DEPTH, 0, one);

		glUseProgram(render_prog);

		glUniform1f(uniforms.time, t);
		glUniformMatrix4fv(uniforms.proj_matrix, 1, GL_FALSE, glm::value_ptr(proj_matrix));

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		glBindVertexArray(star_vao);

		glEnable(GL_PROGRAM_POINT_SIZE);
		glDrawArrays(GL_POINTS, 0, NUM_STARS);
	}

	virtual void shutdown()
	{
		glDeleteVertexArrays(1, &star_vao);
		glDeleteProgram(render_prog);
	}

protected:
	GLuint          render_prog;
	GLuint          star_texture;
	GLuint          star_vao;
	GLuint          star_buffer;

	struct
	{
		int         time;
		int         proj_matrix;
	} uniforms;
};

/** @} @} */

int main(int argc, char** argv)
{
	starfield_app a;
	a.run();
	return 0;
}