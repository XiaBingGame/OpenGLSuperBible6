#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define INTERPOLATE_COLOR

/** \addtogroup Chapter03
*   @{
*       \addtogroup fragcolorfrompos
* 根据片段的位置设置片段颜色.
*
* - 片段着色其实使用 gl_FragCoord 的位置设置片段颜色.
*       @{
*/

class fragcolorfrompos_app : public OpenGLApp
{
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Fragment Color From Position";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}

	virtual void startup()
	{
#ifdef INTERPOLATE_COLOR
		program = LoadShadersVF("../media/glsl/03_fragcolorfrompos_01.vs", "../media/glsl/03_fragcolorfrompos_01.fs");
#else
		program = LoadShadersVF("../media/glsl/03_fragcolorfrompos_02.vs", "../media/glsl/03_fragcolorfrompos_02.fs");
#endif
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

	virtual void render(double currentTime)
	{
		static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, green);

		glUseProgram(program);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	virtual void shutdown()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteProgram(program);
	}

protected:
	GLuint          program;
	GLuint          vao;
};

/** @} @} */

int main(int argc, char** argv)
{
	fragcolorfrompos_app a;
	a.run();
	return 0;
}