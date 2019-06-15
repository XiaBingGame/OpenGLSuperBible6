#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/** \addtogroup Chapter09*   @{*       \addtogroup shapedpoints* 本例通过绘制一个大小200的点, 而后片段着色器中根据 gl_PointCoord 计算出不同的形状, 该内置变量沿着点插值纹理坐标.**	- random_float() 通过该函数产生随机数.*       @{*/

// Random number generator
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


class shapedpoints_app : public OpenGLApp
{
public:
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Shaped Points";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		render_prog = LoadShadersVF("../media/glsl/09_shapedpoints.vs", "../media/glsl/09_shapedpoints.fs");
		glGenVertexArrays(1, &render_vao);
		glBindVertexArray(render_vao);
	}

	virtual void render(double currentTime)
	{
		static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		static const GLfloat one[] = { 1.0f };
		float t = (float)currentTime;

		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glClearBufferfv(GL_COLOR, 0, black);
		glClearBufferfv(GL_DEPTH, 0, one);

		glUseProgram(render_prog);

		glPointSize(200.0f);
		//glBindVertexArray(render_vao);
		glDrawArrays(GL_POINTS, 0, 4);
	}

	virtual void shutdown()
	{
		glDeleteVertexArrays(1, &render_vao);
		glDeleteProgram(render_prog);
	}

protected:
	GLuint          render_prog;
	GLuint          render_vao;
};

/** @} @} */

int main(int argc, char** argv)
{
	shapedpoints_app a;
	a.run();
	return 0;
}