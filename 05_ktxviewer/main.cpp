// 渲染一张纹理

#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/** \addtogroup Chapter05*   @{*       \addtogroup ktxviewer* 纹理的使用, 两个着色器函数 texelFetch 和 texture*       @{*/

class ktxview_app : public OpenGLApp
{
public:
	void init()
	{
		static const char title[] = "OpenGL SuperBible - KTX Viewer";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}

	void startup(void)
	{
		// Generate a name for the texture
		glGenTextures(1, &texture);

		// Load texture from file
		//sb6::ktx::file::load("G:\\Book\\Program\\OpenGL\\OpenGL SuperBible\\src\\media\\textures\\Tree.ktx", texture);
		//loadKTX("../media/textures/Tree.ktx", texture);
		//loadKTX("../media/textures/grass_color.ktx", texture);
		//loadKTX("../media/textures/terragen_color.ktx", texture);
		//loadKTX("../media/textures/grass_color.ktx", texture);
		//loadKTX("../media/textures/pattern1.ktx", texture);
		// Now bind it to the context using the GL_TEXTURE_2D binding point

		texture = loadBMP_custom("../media/textures/001.bmp");
		glBindTexture(GL_TEXTURE_2D, texture);

		program = LoadShadersVF("../media/glsl/05_ktxview.vs", "../media/glsl/05_ktxview.fs");
		exppos = glGetUniformLocation(program, "exposure");
		viewport = glGetUniformLocation(program, "viewport");
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

	void shutdown(void)
	{
		glDeleteTextures(1, &texture);
		glDeleteProgram(program);
		glDeleteVertexArrays(1, &vao);
	}

	void render(double t)
	{
		static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, green);

		glUseProgram(program);
		glViewport(0, 0, info.windowWidth, info.windowHeight);
		//glUniform1f(exppos, (float)(sin(t) * 16.0 + 16.0));
		float v[2] = {(float)info.windowWidth, (float)info.windowHeight};
		glUniform1f(exppos, t);
		glUniform2fv(viewport, 1, v);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
private:
	GLuint      texture;
	GLuint      program;
	GLuint      vao;
	GLuint		exppos;
	GLuint		viewport;
};

/** @} @} */

int main(int argc, char** argv)
{
	ktxview_app a;
	a.run();
	return 0;
}