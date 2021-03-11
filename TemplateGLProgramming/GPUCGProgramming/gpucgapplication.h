#ifndef __GPU_CG_APPLICATION_H__
#define __GPU_CG_APPLICATION_H__

#include "OpenGLApp.h"

class template_app : public OpenGLApp
{
public:
	void init()
	{
		static const char title[] = "OpenGL Application";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
		info.windowWidth = 800;
		info.windowHeight = 600;
	}

	void startup(void)
	{
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

	}

	void shutdown(void)
	{
		glDeleteVertexArrays(1, &vao);
	}


	void render(double t)
	{
		static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, black);

	}
private:
	GLuint      vao;
};

#endif
