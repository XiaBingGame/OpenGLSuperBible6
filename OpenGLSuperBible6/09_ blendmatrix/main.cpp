#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/** \addtogroup Chapter09*   @{*       \addtogroup blendmatrix* 演示各种混合函数的使用, 注意片段着色器产生了两个颜色输出.*       @{*/

class blendmatrix_app : public OpenGLApp
{
public:
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Blending Functions";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		program = LoadShadersVF("../media/glsl/09_blendmatrix.vs", "../media/glsl/09_blendmatrix.fs");
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

		glEnable(GL_CULL_FACE);
	}

	virtual void render(double currentTime)
	{
		int i, j;
		static const GLfloat orange[] = { 0.6f, 0.4f, 0.1f, 1.0f };
		static const GLfloat one = 1.0f;

		static const GLenum blend_func[] =
		{
			GL_ZERO,
			GL_ONE,
			GL_SRC_COLOR,
			GL_ONE_MINUS_SRC_COLOR,
			GL_DST_COLOR,
			GL_ONE_MINUS_DST_COLOR,
			GL_SRC_ALPHA,
			GL_ONE_MINUS_SRC_ALPHA,
			GL_DST_ALPHA,
			GL_ONE_MINUS_DST_ALPHA,
			GL_CONSTANT_COLOR,
			GL_ONE_MINUS_CONSTANT_COLOR,
			GL_CONSTANT_ALPHA,
			GL_ONE_MINUS_CONSTANT_ALPHA,
			GL_SRC_ALPHA_SATURATE,
			GL_SRC1_COLOR,
			GL_ONE_MINUS_SRC1_COLOR,
			GL_SRC1_ALPHA,
			GL_ONE_MINUS_SRC1_ALPHA
		};
		static const int num_blend_funcs = sizeof(blend_func) / sizeof(blend_func[0]);
		static const float x_scale = 20.0f / float(num_blend_funcs);
		static const float y_scale = 16.0f / float(num_blend_funcs);
		const float t = (float)currentTime;
		//float t = 0.0f;
		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glClearBufferfv(GL_COLOR, 0, orange);
		glClearBufferfv(GL_DEPTH, 0, &one);

		glUseProgram(program);

		glm::mat4 proj_matrix = glm::perspective(glm::radians(50.0f),
			(float)info.windowWidth / (float)info.windowHeight,
			0.1f,
			1000.0f);
		glUniformMatrix4fv(proj_location, 1, GL_FALSE, glm::value_ptr(proj_matrix));

		glEnable(GL_BLEND);
		glBlendColor(0.2f, 0.5f, 0.7f, 0.5f);
		
		for (j = 0; j < num_blend_funcs; j++)
		{
			for (i = 0; i < num_blend_funcs; i++)
			{
				glm::mat4 mv_matrix = 
					glm::translate(glm::mat4(1.0f), glm::vec3(9.5f - x_scale * float(i),
					7.5f - y_scale * float(j),
					-18.0f)) *
					glm::rotate(glm::mat4(1.0f), glm::radians(t * -45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
					glm::rotate(glm::mat4(1.0f), glm::radians(t * -21.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				glUniformMatrix4fv(mv_location, 1, GL_FALSE, glm::value_ptr(mv_matrix));
				glBlendFunc(blend_func[i], blend_func[j]);
				glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
			}
		}
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
};

/** @} @} */

int main(int argc, char** argv)
{
	blendmatrix_app a;
	a.run();
	return 0;
}