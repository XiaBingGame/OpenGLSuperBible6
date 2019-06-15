#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/** \addtogroup Chapter08*   @{*       \addtogroup normalviewer* 几何着色器绘制一个模型的法线线型模式.**	- 几何着色器输入三角形, 输出线条. 分别绘制两个线条, 一个三角形重中心的线条, 一个三角形第一个顶点的线条.*       @{*/

class normalviewer_app : public OpenGLApp
{
public:
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Normal Line Viewer";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		program = LoadShadersVGF("../media/glsl/08_normalviewer.vs", "../media/glsl/08_normalviewer.gs", "../media/glsl/08_normalviewer.fs");
		mv_location = glGetUniformLocation(program, "mv_matrix");
		proj_location = glGetUniformLocation(program, "proj_matrix");
		normal_length_location = glGetUniformLocation(program, "normal_length");

		object.load("../media/models/bunny_1k.sbm");

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
		glUniformMatrix4fv(proj_location, 1, GL_FALSE, glm::value_ptr(proj_matrix));

		glm::mat4 mv_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians((float)currentTime * 45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians((float)currentTime * 81.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(mv_location, 1, GL_FALSE, glm::value_ptr(mv_matrix));

		glUniform1f(normal_length_location, 0.01f); // sinf((float)currentTime * 8.0f) * cosf((float)currentTime * 6.0f) * 0.3f + 0.5f);

		object.render();
	}

	virtual void shutdown()
	{
		object.free();
		glDeleteProgram(program);
	}

protected:
	GLuint          program;
	
	GLuint			mv_location;
	GLuint			proj_location;
	GLuint			normal_length_location;

	sb6::object		object;
};

int main(int argc, char** argv)
{
	normalviewer_app a;
	a.run();
	return 0;
}