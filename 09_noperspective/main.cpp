#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/** \addtogroup Chapter09*   @{*       \addtogroup noperspective* 本例对于纹理坐标通过 noperspective 前缀实现在屏幕空间中线性插值.* *       @{*/

class noperspective_app : public OpenGLApp
{
public:
	noperspective_app()
		: paused(false),
		  use_perspective(true)
	{}
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Perspective";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		char buffer[1024];
		program = LoadShadersVF("../media/glsl/09_noperspective.vs", "../media/glsl/09_noperspective.fs");
		uniforms.mvp = glGetUniformLocation(program, "mvp");
		uniforms.use_perspective = glGetUniformLocation(program, "use_perspective");
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		static const unsigned char checker_data[] =
		{
			0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
			0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
			0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
			0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
			0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
			0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
			0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
			0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
		};

		glGenTextures(1, &tex_checker);
		glBindTexture(GL_TEXTURE_2D, tex_checker);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, 8, 8);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 8, GL_RED, GL_UNSIGNED_BYTE, checker_data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	virtual void render(double currentTime)
	{
		static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		static const GLfloat one = 1.0f;
		static double last_time = 0.0;
		static double total_time = 0.0;

		if (!paused)
			total_time += (currentTime - last_time);
		last_time = currentTime;

		float t = (float)total_time * 14.3f;

		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glClearBufferfv(GL_COLOR, 0, black);
		glClearBufferfv(GL_DEPTH, 0, &one);

		glm::mat4 mv_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.5f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(t), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 proj_matrix = glm::perspective(glm::radians(60.0f),
			(float)info.windowWidth / (float)info.windowHeight,
			0.1f, 1000.0f);

		glUseProgram(program);

		glUniformMatrix4fv(uniforms.mvp, 1, GL_FALSE, glm::value_ptr(proj_matrix * mv_matrix));
		glUniform1i(uniforms.use_perspective, use_perspective);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	virtual void shutdown()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteProgram(program);
	}

	void onKey(int key, int scancode, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			switch(key)
			{
			case 'I':
				use_perspective = !use_perspective;
				break;
			case 'P':
				paused = !paused;
				break;
			default:
				break;
			}
		}
		OpenGLApp::onKey(key, scancode, action, mods);
	}

protected:
	GLuint          program;
	GLuint          vao;

	GLuint			tex_checker;
	bool				paused;
	bool				use_perspective;

	struct 
	{
		GLuint		mvp;
		GLuint		use_perspective;
	} uniforms;
};

/** @} @} */

int main(int argc, char** argv)
{
	noperspective_app a;
	a.run();
	return 0;
}