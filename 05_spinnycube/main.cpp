#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/** \addtogroup Chapter05*   @{*       \addtogroup spinnycube* 使用无索引的顶点 buffer 绘制模型*       @{*/

#define MANY_CUBES

class spinnycube_app : public OpenGLApp
{
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Spinny Cube";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}

	virtual void startup()
	{
		OpenGLApp::startup();
		program = LoadShadersVF("../media/glsl/05_spinnycube.vs", "../media/glsl/05_spinnycube.fs");

		mv_location = glGetUniformLocation(program, "mv_matrix");
		proj_location = glGetUniformLocation(program, "proj_matrix");

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		static const GLfloat vertex_positions[] =
		{
			-0.25f,  0.25f, -0.25f,
			-0.25f, -0.25f, -0.25f,
			0.25f, -0.25f, -0.25f,

			0.25f, -0.25f, -0.25f,
			0.25f,  0.25f, -0.25f,
			-0.25f,  0.25f, -0.25f,

			0.25f, -0.25f, -0.25f,
			0.25f, -0.25f,  0.25f,
			0.25f,  0.25f, -0.25f,

			0.25f, -0.25f,  0.25f,
			0.25f,  0.25f,  0.25f,
			0.25f,  0.25f, -0.25f,

			0.25f, -0.25f,  0.25f,
			-0.25f, -0.25f,  0.25f,
			0.25f,  0.25f,  0.25f,

			-0.25f, -0.25f,  0.25f,
			-0.25f,  0.25f,  0.25f,
			0.25f,  0.25f,  0.25f,

			-0.25f, -0.25f,  0.25f,
			-0.25f, -0.25f, -0.25f,
			-0.25f,  0.25f,  0.25f,

			-0.25f, -0.25f, -0.25f,
			-0.25f,  0.25f, -0.25f,
			-0.25f,  0.25f,  0.25f,

			-0.25f, -0.25f,  0.25f,
			0.25f, -0.25f,  0.25f,
			0.25f, -0.25f, -0.25f,

			0.25f, -0.25f, -0.25f,
			-0.25f, -0.25f, -0.25f,
			-0.25f, -0.25f,  0.25f,

			-0.25f,  0.25f, -0.25f,
			0.25f,  0.25f, -0.25f,
			0.25f,  0.25f,  0.25f,

			0.25f,  0.25f,  0.25f,
			-0.25f,  0.25f,  0.25f,
			-0.25f,  0.25f, -0.25f
		};

		

		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER,
			sizeof(vertex_positions),
			vertex_positions,
			GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);

		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CW);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	}

	virtual void render(double currentTime)
	{
		static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
		static const GLfloat one = 1.0f;

		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glClearBufferfv(GL_COLOR, 0, green);
		glClearBufferfv(GL_DEPTH, 0, &one);


		glUseProgram(program);
		glUniformMatrix4fv(proj_location, 1, GL_FALSE, &proj_matrix[0][0]);

		
#ifdef MANY_CUBES
		int i;
		for (i = 0; i < 24; i++)
		{
			float f = (float)i + (float)currentTime*0.3f;
			glm::mat4 mv_matrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, -6.0f)) *
				glm::rotate(glm::mat4(1.0), glm::radians((float)currentTime * 45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
				glm::rotate(glm::mat4(1.0), glm::radians((float)currentTime * 21.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
				glm::translate(glm::mat4(1.0), glm::vec3(sin(2.1f * f) * 2.0f, 
				cos(1.7f * f) * 2.0f,
				sin(1.3f * f) * cos(1.5f * f) * 3.0f));
			glUniformMatrix4fv(mv_location, 1, GL_FALSE, &mv_matrix[0][0]);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
#else
		float f = (float)currentTime*0.3f;
		glm::mat4 mv_matrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, -4.0f)) *
			glm::translate(glm::mat4(1.0), glm::vec3(sin(2.1f * f) * 0.5f,
			cos(1.7f * f) * 0.5f,
			sin(1.3f * f) * cos(1.5f * f) * 2.0f)) * 
			glm::rotate(glm::mat4(1.0), glm::radians((float)currentTime * 45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0), glm::radians((float)currentTime * 81.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(mv_location, 1, GL_FALSE, &mv_matrix[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, 36);
#endif		
	}

	virtual void shutdown()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteProgram(program);
	}

	void onResize(int w, int h)
	{
		OpenGLApp::onResize(w, h);

		aspect = (float)w/(float)h;
		// 用SB6自带的来重写这个
		proj_matrix = glm::perspective(glm::radians(50.0f), aspect, 0.1f, 100.0f);
	}

protected:
	GLuint          program;
	GLuint          vao;
	GLuint			buffer;
	GLuint			mv_location;
	GLuint			proj_location;

	float			aspect;
	glm::mat4		proj_matrix;
};

/** @} @} */

int main(int argc, char** argv)
{
	spinnycube_app a;
	a.run();
	return 0;
}

