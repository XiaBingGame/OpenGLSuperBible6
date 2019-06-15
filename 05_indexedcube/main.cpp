#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/** \addtogroup Chapter05*   @{*       \addtogroup * 通过 buffer 绘制模型.*	- 使用 GL_ARRAY_BUFFER 和 GL_ELEMENT_ARRAY_BUFFER 绘制模型*	- glDrawElements() 用于索引绘制.*       @{*/

#define MANY_CUBES

class singlepoint_app : public OpenGLApp
{
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Indexed Cube";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
	
	virtual void startup()
	{
		program = LoadShadersVF("../media/glsl/05_indexedcube.vs", "../media/glsl/05_indexedcube.fs");
		mv_location = glGetUniformLocation(program, "mv_matrix");
		proj_location = glGetUniformLocation(program, "proj_matrix");

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		static const GLushort vertex_indices[] =
		{
			0, 1, 2,
			2, 1, 3,
			2, 3, 4,
			4, 3, 5,
			4, 5, 6,
			6, 5, 7,
			6, 7, 0,
			0, 7, 1,
			6, 0, 2,
			2, 4, 6,
			7, 5, 3,
			7, 3, 1
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
		
		// 使用 GL_ARRAY_BUFFER 和 GL_ELEMENT_ARRAY_BUFFER 绘制模型
		glGenBuffers(1, &position_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_positions), vertex_positions, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &index_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertex_indices), vertex_indices, GL_STATIC_DRAW);

		glEnable(GL_CULL_FACE);
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
		glm::mat4 proj_matrix = glm::perspective(glm::radians(50.0f), (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);
		glUniformMatrix4fv(proj_location, 1, GL_FALSE, &proj_matrix[0][0]);

		// glDrawElements() 用于索引绘制.
#ifdef MANY_CUBES
		for (i = 0; i < 24; i++)
		{
			float f = (float)i + (float)currentTime * 0.3f;
			glm::mat4 mv_matrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, -20.0f));
			mv_matrix = glm::rotate(mv_matrix, glm::radians((float)currentTime * 45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			mv_matrix = glm::rotate(mv_matrix, glm::radians((float)currentTime * 21.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			mv_matrix = glm::translate(mv_matrix, glm::vec3(sinf(2.1f * f) * 2.0f, cosf(1.7f * f) * 2.0f, sinf(1.3f * f) * cosf(1.5f * f) * 2.0f));
			glUniformMatrix4fv(mv_location, 1, GL_FALSE, &mv_matrix[0][0]);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
		}
#else
		float f = (float)currentTime * 0.3f;
		glm::mat4 mv_matrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, -4.0f));
		mv_matrix = glm::translate(mv_matrix, glm::vec3(sinf(2.1f * f) * 0.5f, cosf(1.7f * f) * 0.5f, sinf(1.3f * f) * cosf(1.5f * f) * 2.0f));
		mv_matrix = glm::rotate(mv_matrix, glm::radians((float)currentTime * 45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		mv_matrix = glm::rotate(mv_matrix, glm::radians((float)currentTime * 81.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(mv_location, 1, GL_FALSE, &mv_matrix[0][0]);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
#endif
	}

	virtual void shutdown()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteProgram(program);
		glDeleteBuffers(1, &position_buffer);
	}

private:
	GLuint          program;
	GLuint          vao;
	GLuint          position_buffer;
	GLuint          index_buffer;
	GLint           mv_location;
	GLint           proj_location;
};
/** @} @} */

int main(int argc, char** argv)
{
	singlepoint_app a;
	a.run();
	return 0;
}