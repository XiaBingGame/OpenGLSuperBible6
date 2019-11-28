#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/** \addtogroup Chapter09*   @{*       \addtogroup depthclamp* 允许 GL_DEPTH_CLAMP 来关闭近截平面和远截平面的裁剪.**       @{*/

class depthclamp_app : public OpenGLApp
{
public:
	depthclamp_app() : enable_clamped(true) {}
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Depth Clamping";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		program = LoadShadersVF("../media/glsl/09_depthclamp.vs", "../media/glsl/09_depthclamp.fs");
		mv_location = glGetUniformLocation(program, "mv_matrix");
		proj_location = glGetUniformLocation(program, "proj_matrix");

		object.load("../media/models/bunny_1k.sbm");

		glEnable(GL_CULL_FACE);
		//glCullFace(GL_FRONT);

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
			1.8f,
			1000.0f);
		glUniformMatrix4fv(proj_location, 1, GL_FALSE, glm::value_ptr(proj_matrix));

		if(enable_clamped)
			glEnable(GL_DEPTH_CLAMP);
		else
			glDisable(GL_DEPTH_CLAMP);

		glm::mat4 mv_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(f * 45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(f * 81.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(mv_location, 1, GL_FALSE, glm::value_ptr(mv_matrix));

		//glUniform1f(explode_factor_location, sinf((float)currentTime * 8.0f) * cosf((float)currentTime * 6.0f) * 0.7f + 0.1f);

		object.render();
	}

	void onKey(int key, int scancode, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			switch(key)
			{
			case 'E':
				enable_clamped = !enable_clamped;
				break;
			default:
				break;
			}
		}
		OpenGLApp::onKey(key, scancode, action, mods);
	}

	virtual void shutdown()
	{
		object.free();
		glDeleteProgram(program);
	}

protected:
	GLuint          program;
	GLint           mv_location;
	GLint           proj_location;
	GLint           explode_factor_location;

	GLboolean		enable_clamped;

	sb6::object     object;
};

/** @} @} */

int main(int argc, char** argv)
{
	depthclamp_app a;
	a.run();
	return 0;
}