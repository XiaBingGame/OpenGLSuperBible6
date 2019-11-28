#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/** \addtogroup Chapter08*   @{*       \addtogroup objectexploder* 通过几何着色器实现模型向外爆炸的效果.**	- 在几何着色器中移动三角形的位置.*       @{*/

class objectexploder_app : public OpenGLApp
{
public:
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Geometry Shader Culling";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		program = LoadShadersVGF("../media/glsl/08_objectexploder.vs", "../media/glsl/08_objectexploder.gs", "../media/glsl/08_objectexploder.fs");

		mv_location = glGetUniformLocation(program, "mvMatrix");
		proj_location = glGetUniformLocation(program, "projMatrix");
		explode_factor_location = glGetUniformLocation(program, "explode_factor");

		object.load("../media/models/bunny_1k.sbm");
		//glGenVertexArrays(1, &vao);
		//glBindVertexArray(vao);
		
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	}

	virtual void render(double currentTime)
	{
		static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		static const GLfloat one = 1.0f;
		float f = (float)currentTime;

		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glClearBufferfv(GL_COLOR, 0, black);
		glClearBufferfv(GL_DEPTH, 0, &one);

		glUseProgram(program);

		glm::mat4 proj_matrix = glm::perspective(glm::radians(50.0f),
			(float)info.windowWidth / (float)info.windowHeight,
			0.1f,
			1000.0f);
		glm::mat4 mv_matrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, -1.5f)) *
								glm::rotate(glm::mat4(1.0f), glm::radians((float)currentTime * 45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
								glm::rotate(glm::mat4(1.0f), glm::radians((float)currentTime * 81.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		glUniformMatrix4fv(proj_location, 1, GL_FALSE, glm::value_ptr(proj_matrix));
		glUniformMatrix4fv(mv_location, 1, GL_FALSE, glm::value_ptr(mv_matrix));

		glUniform1f(explode_factor_location, sinf((float)currentTime * 8.0f) * cosf((float)currentTime * 6.0f) * 0.7f + 0.1f);

		object.render();
	}

	virtual void shutdown()
	{
		object.free();
		glDeleteVertexArrays(1, &vao);
		glDeleteProgram(program);
	}

protected:
	GLuint          program;
	GLuint          vao;

	GLuint			mv_location;
	GLuint			proj_location;
	GLuint			explode_factor_location;

	sb6::object		object;
};

/** @} @} */

int main(int argc, char** argv)
{
	objectexploder_app a;
	a.run();
	return 0;
}