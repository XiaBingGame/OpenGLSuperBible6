// 几何着色器中, gl_InvocationID 的范围为[0, N-1], N 表示每个图元绘制的次数, 这对于 layered rendering 和输出到 multiple streams 都很有用.
#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/** \addtogroup Chapter08*   @{*       \addtogroup multiviewport**	- 使用 glViewportIndexedf() 设置不同视口的大小.*	- glBindBufferBase() 和 GL_UNIFORM_BUFFER 将一个 buffer 绑定至 UNIFORM_BUFFER 绑定点.*	- 几何着色器中使用 gl_InvocationID 表示不同的视口. 本例不同的视口选择了不同的矩阵.*       @{*/

class multiviewport_app : public OpenGLApp
{
public:
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Multiple Viewport";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		program = LoadShadersVGF("../media/glsl/08_multiviewport.vs", "../media/glsl/08_multiviewport.gs", "../media/glsl/08_multiviewport.fs");
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

		glGenBuffers(1, &uniform_buffer);
		glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
		glBufferData(GL_UNIFORM_BUFFER, 4 * 16 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

		glEnable(GL_CULL_FACE);
		// glFrontFace(GL_CW);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	}

	virtual void render(double currentTime)
	{
		int i;
		static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		static const GLfloat one = 1.0f;

		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glClearBufferfv(GL_COLOR, 0, black);
		glClearBufferfv(GL_DEPTH, 0, &one);
		
		// Each rectangle will be 7/16 of the screen
		float viewport_width = (float)(7 * info.windowWidth) / 16.0f;
		float viewport_height = (float)(7 * info.windowHeight) / 16.0f;

		// Four rectangles - lower left first...
		glViewportIndexedf(0, 0, 0, viewport_width, viewport_height);

		// Lower right...
		glViewportIndexedf(1,
			info.windowWidth - viewport_width, 0,
			viewport_width, viewport_height);

		// Upper left...
		glViewportIndexedf(2,
			0, info.windowHeight - viewport_height,
			viewport_width, viewport_height);

		// Upper right...
		glViewportIndexedf(3,
			info.windowWidth - viewport_width,
			info.windowHeight - viewport_height,
			viewport_width, viewport_height);
		
		float f = (float)currentTime * 0.3f;

		glm::mat4 proj_matrix = glm::perspective(glm::radians(50.0f),
			(float)info.windowWidth / (float)info.windowHeight,
			0.1f,
			1000.0f);
		glm::mat4 mv_matrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, -4.0f));
		mv_matrix = glm::translate(mv_matrix, glm::vec3(sinf(2.1f * f) * 0.5f, cosf(1.7f * f) * 0.5f, sinf(1.3f * f) * cosf(1.5f * f) * 2.0f));
		mv_matrix = glm::rotate(mv_matrix, glm::radians((float)currentTime * 45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		mv_matrix = glm::rotate(mv_matrix, glm::radians((float)currentTime * 81.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		
		// glBindBufferBase() 和 GL_UNIFORM_BUFFER 将一个 buffer 绑定至 UNIFORM_BUFFER 绑定点.
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniform_buffer);
		float * mv_ma = (float *)glMapBufferRange(GL_UNIFORM_BUFFER,
			0,
			4 * 16 * sizeof(float),
			GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

		glm::mat4 mv_matrix_array[4];

		for (i = 0; i < 4; i++)
		{
			mv_matrix_array[i] = proj_matrix * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians((float)currentTime * 45.0f * (float)(i + 1)), glm::vec3(0.0f, 1.0f, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians((float)currentTime * 81.0f * (float)(i + 1)), glm::vec3(1.0f, 0.0f, 0.0f));

			for (int j = 0; j < 4; j++)
			{
				glm::vec4 v = mv_matrix_array[i][j];
				for (int k = 0; k < 4; k++)
				{
					mv_ma[4*j+k] = v[k];
				}
			}

			mv_ma += 16;
		}
		glUnmapBuffer(GL_UNIFORM_BUFFER);
		
		glUseProgram(program);
		glUniformMatrix4fv(proj_location, 1, GL_FALSE, &proj_matrix[0][0]);
		
		glUniformMatrix4fv(mv_location, 1, GL_FALSE, &mv_matrix[0][0]);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
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
	GLuint          uniform_buffer;
	GLint           mv_location;
	GLint           proj_location;
};

/** @} @} */

int main(int argc, char** argv)
{
	multiviewport_app a;
	a.run();
	return 0;
}