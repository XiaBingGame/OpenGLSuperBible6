#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/** \addtogroup Chapter09*   @{*       \addtogroup hdrexposure* 在着色器中实现一张纹理图的曝光.**	- 着色器的核心代码: c.xyz = vec3(1.0) - exp(-c.xyz * exposure);*       @{*/

class hdrexposure_app : public OpenGLApp
{
public:
	hdrexposure_app() :
	  exposure(1.0f) {}

	void init()
	{
		static const char title[] = "OpenGL SuperBible - HDR Exposure";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		// Generate a name for the texture
		glGenTextures(1, &texture);

		// Load texture from file
		loadKTX("../media/textures/treelights_2k.ktx", texture);

		// Now bind it to the context using the GL_TEXTURE_2D binding point
		glBindTexture(GL_TEXTURE_2D, texture);

		program = LoadShadersVF("../media/glsl/09_hdrexposure.vs", "../media/glsl/09_hdrexposure.fs");
		exposurelocation = glGetUniformLocation(program, "exposure");
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

	virtual void render(double currentTime)
	{
		static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, green);

		glUseProgram(program);
		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glUniform1f(exposurelocation, exposure);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	virtual void shutdown()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteProgram(program);
		glDeleteTextures(1, &texture);
	}

	void onKey(int key, int scancode, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			switch(key)
			{
			case GLFW_KEY_UP:
				exposure *= 1.1f;
				break;
			case GLFW_KEY_DOWN:
				exposure /= 1.1f;
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

	GLuint			texture;
	float			exposure;
	GLuint			exposurelocation;
};

/** @} @} */

int main(int argc, char** argv)
{
	hdrexposure_app a;
	a.run();
	return 0;
}