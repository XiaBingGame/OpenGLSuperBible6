#include <iostream>
#include "OpenGLApp.h"
#include "shader.h"
#include "texture.h"
#include "model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifdef _DEBUG
#pragma comment(lib, "OpenGLAppDll_d.lib")
#else
#pragma comment(lib, "OpenGLAppDll.lib")
#endif

class ktxview_app : public OpenGLApp
{
public:
	void init()
	{
		static const char title[] = "OpenGL SuperBible - KTX Viewer";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
		info.windowWidth = 480;
		info.windowHeight = 480;
	}

	void startup(void)
	{
		// Generate a name for the texture
		glGenTextures(1, &texture);

		// Load texture from file
		//sb6::ktx::file::load("G:\\Book\\Program\\OpenGL\\OpenGL SuperBible\\src\\media\\textures\\Tree.ktx", texture);
		//loadKTX("../media/textures/Tree.ktx", texture);
		//loadKTX("../media/textures/grass_color.ktx", texture);
		loadKTX("../media/textures/terragen_color.ktx", texture);
		//texture = loadBMP_custom("../media/textures/blue_jaiqua.bmp");
		//loadKTX("../media/textures/grass_color.ktx", texture);
		//loadKTX("../media/textures/pattern1.ktx", texture);
		// Now bind it to the context using the GL_TEXTURE_2D binding point
		glBindTexture(GL_TEXTURE_2D, texture);
		GLFWwindow* w = getWindow();
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
		glUniform1f(exppos, 1.0);
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