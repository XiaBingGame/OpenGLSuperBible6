#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/** \addtogroup Chapter02
*   @{
*       \addtogroup singlepoint
* ����һ����.
* 
*	- glPointSize() --- ���õ��С
*	- glDrawArrays() --- ��������
*	- ������ɫ��ֻ�����˱���gl_Position, Ϊ vec4(0.0, 0.0, 0.5, 1.0)
*       @{
*/
class singlepoint_app : public OpenGLApp
{
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Single Point";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}

	void startup()
	{
		render_program = LoadShadersVF("../media/glsl/02_singlepoint.vs", "../media/glsl/02_singlepoint.fs");
		glGenVertexArrays(1, &verter_array_object);
		glBindVertexArray(verter_array_object);
	}

	/**
	 * - glPointSize() --- ���õ��С
	 * - glDrawArrays() --- ��������
	**/
	void render(double currentTime)
	{
		const GLfloat color[] = {(float)sin(currentTime)*0.5f+0.5f,
			(float)cos(currentTime)*0.5f+0.5f,
			0.0f, 1.0f};
		glClearBufferfv(GL_COLOR, 0, color);

		glUseProgram(render_program);
		glPointSize(40.0f);
		glDrawArrays(GL_POINTS,0, 1);
		// glPointSize(1.0f);
	}

	void shutdown()
	{
		glDeleteVertexArrays(1, &verter_array_object);
		glDeleteProgram(render_program);
	}

protected:
	GLuint		render_program;
	GLuint		verter_array_object;
};
/** @} @} */
int main(int argc, char** argv)
{
	singlepoint_app a;
	a.run();
	return 0;
}