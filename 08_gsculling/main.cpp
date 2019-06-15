#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/** \addtogroup Chapter06*   @{*       \addtogroup gsculling* 几何着色器, 根据需要是否抛弃一个图元.**	- 顶点着色器计算出位置和颜色, 并传递法线.*	- 几何着色器: 根据不同的视点计算出一个图元是否面向该视点, 如果没有面向则抛弃该视点.*       @{*/

class gsculling_app : public OpenGLApp
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
		program = LoadShadersVGF("../media/glsl/08_gsculling.vs", "../media/glsl/08_gsculling.gs", "../media/glsl/08_gsculling.fs");

		mv_location = glGetUniformLocation(program, "mvMatrix");
		mvp_location = glGetUniformLocation(program, "mvpMatrix");
		viewpoint_location = glGetUniformLocation(program, "viewpoint");

		object.load("../media/models/bunny_1k.sbm");
		//glGenVertexArrays(1, &vao);
		//glBindVertexArray(vao);
		glDisable(GL_CULL_FACE);

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
        glm::mat4 mv_matrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, -1.5f));

        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(proj_matrix * mv_matrix));
        glUniformMatrix4fv(mv_location, 1, GL_FALSE, glm::value_ptr(mv_matrix));

        GLfloat vViewpoint[] = { sinf(f * 2.1f) * 70.0f, cosf(f * 1.4f) * 70.0f, sinf(f * 0.7f) * 70.0f };
        glUniform3fv(viewpoint_location, 1, vViewpoint);

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
	GLuint			mvp_location;
	GLuint			viewpoint_location;

	sb6::object		object;
};

/** @} @} */

int main(int argc, char** argv)
{
	gsculling_app a;
	a.run();
	return 0;
}