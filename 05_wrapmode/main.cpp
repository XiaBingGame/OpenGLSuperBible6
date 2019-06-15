// 围绕模式

#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/** \addtogroup Chapter05*   @{*       \addtogroup wrapmode* 演示了不同的 wrap 模式.*       @{*/

class wrapmode_app : public OpenGLApp
{
public:
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Texture Wrap Modes";

		OpenGLApp::init();

		memcpy(info.title, title, sizeof(title));
	}

	void startup(void)
	{
		// Generate a name for the texture
		glGenTextures(1, &texture);

		// Load texture from file
		loadKTX("../media/textures/rightarrows.ktx", texture);

		// Now bind it to the context using the GL_TEXTURE_2D binding point
		glBindTexture(GL_TEXTURE_2D, texture);

		program = LoadShadersVF("../media/glsl/05_wrapmode.vs", "../media/glsl/05_wrapmode.fs");
		offsetloc = glGetUniformLocation(program, "offset");

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

	void shutdown(void)
	{
		glDeleteProgram(program);
		glDeleteVertexArrays(1, &vao);
		glDeleteTextures(1, &texture);
	}

	void render(double t)
	{
		static const GLfloat green[] = { 0.0f, 0.1f, 0.0f, 1.0f };
		static const GLfloat yellow[] = { 0.4f, 0.4f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, green);

		static const GLenum wrapmodes[] = { GL_CLAMP_TO_EDGE, GL_REPEAT, GL_CLAMP_TO_BORDER, GL_MIRRORED_REPEAT };
		static const float offsets[] = { -0.5f, -0.5f,
			0.5f, -0.5f,
			-0.5f,  0.5f,
			0.5f,  0.5f };

		glUseProgram(program);
		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, yellow);

		for (int i = 0; i < 4; i++)
		{
			glUniform2fv(offsetloc, 1, &offsets[i * 2]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapmodes[i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapmodes[i]);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
	}

private:
	GLuint      texture;
	GLuint      program;
	GLuint      vao;
	GLuint		offsetloc;
};

/** @} @} */

int main(int argc, char** argv)
{
	wrapmode_app a;
	a.run();
	return 0;
}