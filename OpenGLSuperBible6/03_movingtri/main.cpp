#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/** \addtogroup Chapter03
*   @{
*       \addtogroup movingtri
* 使用 glVertexAttrib4fv() 设置着色器中的顶点属性.
* 
* - 使用 glVertexAttrib4fv() 设置着色器中的顶点属性.
*       @{
*/

class movingtri_app : public OpenGLApp
{
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Moving Triangle";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}

	virtual void startup()
	{
		program = LoadShadersVF("../media/glsl/03_movingtri.vs", "../media/glsl/03_movingtri.fs");
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

	/**
	 * - 使用 glVertexAttrib4fv() 设置着色器中的顶点属性.                                                                     
	 */
	virtual void render(double currentTime)
	{
		static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, green);

		GLfloat attrib[] = { (float)sin(currentTime) * 0.5f,
							 (float)cos(currentTime) * 0.5f,
							 0.0f, 0.0f };

		glVertexAttrib4fv(0, attrib);

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
	movingtri_app a;
	a.run();
	return 0;
}