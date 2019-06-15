#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/** \addtogroup Chapter02
*   @{
*       \addtogroup singletri
* 使用着色器绘制三角形.
*	- - glDrawArrays() --- 绘制数组, 使用 GL_TRIANGLES 作为参数.
*       @{
*/

class singletri_app : public OpenGLApp
{
public:
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Single Triangle";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		program = LoadShadersVF("../media/glsl/02_singletri.vs", "../media/glsl/02_singletri.fs");
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glEnable(GL_DEPTH_TEST);
	}

	/**
	 * - glDrawArrays() --- 绘制数组, 使用 GL_TRIANGLES 作为参数.
	 * 
	**/
	virtual void render(double currentTime)
	{
		static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, green);
		static float one = 1.0;
		glClearBufferfv(GL_DEPTH, 0, &one);
		glUseProgram(program);
		glUniform1f(glGetUniformLocation(program, "z"), -1.0);
		glUniform3f(glGetUniformLocation(program, "c"), 1.0, 0.0, 0.0);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glUniform1f(glGetUniformLocation(program, "z"), 0.0);
		glUniform3f(glGetUniformLocation(program, "c"), 1.0, 1.0, 0.0);
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
	singletri_app a;
	a.run();
	return 0;
}