#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/** \addtogroup Chapter08*   @{*       \addtogroup tessellatedcube* 使用 TCS, TES 对一个立方体再划分.*	- 第二个 tcs 根据线段在屏幕上的大小进行子划分, 大小越大则子划分等级越高.*	- 屏幕的大小根据投影后的坐标距离得出.*       @{*/

//#define MANY_CUBES

class tessellatedcube_app : public OpenGLApp
{
public:
	tessellatedcube_app() : wireframe_mode(false) {}
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Tessellated Cube";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
#if 0
		program = LoadShadersVTF("../media/glsl/08_tessellatedcube.vs", 
			"../media/glsl/08_tessellatedcube.1.tcs", 
			"../media/glsl/08_tessellatedcube.tes",
			"../media/glsl/08_tessellatedcube.fs");
#else
		program = LoadShadersVTF("../media/glsl/08_tessellatedcube.vs", 
			"../media/glsl/08_tessellatedcube.2.tcs", 
			"../media/glsl/08_tessellatedcube.tes",
			"../media/glsl/08_tessellatedcube.fs");
#endif
		mv_location = glGetUniformLocation(program, "mv_matrix");
		proj_location = glGetUniformLocation(program, "proj_matrix");

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		static const GLushort vertex_indices[] =
		{
			0, 1, 2, 3,
			2, 3, 4, 5,
			4, 5, 6, 7,
			6, 7, 0, 1,
			0, 2, 6, 4,
			1, 7, 3, 5
		};

		static const GLfloat vertex_positions[] =
		{
			-0.25f, -0.25f, -0.25f,
			-0.25f,  0.25f, -0.25f,
			0.25f, -0.25f, -0.25f,
			0.25f,  0.25f, -0.25f,
			0.25f, -0.25f,  0.25f,
			0.25f,  0.25f,  0.25f,
			-0.25f, -0.25f,  0.25f,
			-0.25f,  0.25f,  0.25f,
		};

		glGenBuffers(1, &position_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
		glBufferData(GL_ARRAY_BUFFER,
			sizeof(vertex_positions),
			vertex_positions,
			GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &index_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			sizeof(vertex_indices),
			vertex_indices,
			GL_STATIC_DRAW);

		glEnable(GL_CULL_FACE);
		// glFrontFace(GL_CW);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

	}

	virtual void render(double currentTime)
	{
		int i;
		static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
		static const GLfloat one = 1.0f;

		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glClearBufferfv(GL_COLOR, 0, green);
		glClearBufferfv(GL_DEPTH, 0, &one);

		glUseProgram(program);

		glm::mat4 proj_matrix = glm::perspective(glm::radians(50.0f),
			(float)info.windowWidth / (float)info.windowHeight,
			0.1f,
			1000.0f);
		glUniformMatrix4fv(proj_location, 1, GL_FALSE, glm::value_ptr(proj_matrix));
		glPatchParameteri(GL_PATCH_VERTICES, 4);

		glPolygonMode(GL_FRONT_AND_BACK, wireframe_mode ? GL_LINE : GL_FILL);

#ifdef MANY_CUBES
		for (int i = 0; i < 100; i++)
		{
			float f = (float)i + (float)currentTime * 0.03f;
			glm::mat4 mv_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0)) *
				glm::translate(glm::mat4(1.0f), glm::vec3(sinf(2.1f * f) * 4.0f,
				cosf(1.7f * f) * 4.0f,
				sinf(4.3f * f) * cosf(3.5f * f) * 30.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians((float)currentTime * 3.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians((float)currentTime * 5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(mv_location, 1, GL_FALSE, glm::value_ptr(mv_matrix));
			glDrawElements(GL_PATCHES, 24, GL_UNSIGNED_SHORT, 0);
		}
#else
		currentTime = 9.85;
		float f = (float)currentTime * 0.3f;
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		
		glm::mat4 mv_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f)) *
			glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, // sinf(2.1f * f) * 0.5f,
			0.0f, // cosf(1.7f * f) * 0.5f,
			sinf(1.3f * f) * cosf(1.5f * f) * 15.0f)) *
			// vmath::rotate((float)currentTime * 45.0f, 0.0f, 1.0f, 0.0f) *
			glm::rotate(glm::mat4(1.0f), glm::radians((float)currentTime * 81.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(mv_location, 1, GL_FALSE, glm::value_ptr(mv_matrix));
		glDrawElements(GL_PATCHES, 24, GL_UNSIGNED_SHORT, 0);
#endif
	}

	virtual void shutdown()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteProgram(program);
	}

protected:
	GLuint          program;
	GLuint          vao;
	GLuint          position_buffer;
	GLuint          index_buffer;
	GLint           mv_location;
	GLint           proj_location;

	bool            wireframe_mode;
};

/** @} @} */

int main(int argc, char** argv)
{
	tessellatedcube_app a;
	a.run();
	return 0;
}