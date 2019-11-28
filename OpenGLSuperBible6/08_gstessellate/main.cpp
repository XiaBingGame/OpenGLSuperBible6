#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/** \addtogroup Chapter08*   @{*       \addtogroup gstessellate**	- 数据和索引使用同一个缓存.*	- 在几何着色器中将一个三角形更改成多种形状.*       @{*/

class gstessellate_app : public OpenGLApp
{
public:
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Geometry Shader Tessellate";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		program = LoadShadersVGF("../media/glsl/08_gstessellate.vs", "../media/glsl/08_gstessellate.gs", "../media/glsl/08_gstessellate.fs");
		mv_location = glGetUniformLocation(program, "mvMatrix");
		mvp_location = glGetUniformLocation(program, "mvpMatrix");
		stretch_location = glGetUniformLocation(program, "stretch");

		static const GLfloat tetrahedron_verts[] =
		{
			0.000f,  0.000f,  1.000f,
			0.943f,  0.000f, -0.333f,
			-0.471f,  0.816f, -0.333f,
			-0.471f, -0.816f, -0.333f
		};

		static const GLushort tetrahedron_indices[] =
		{
			0, 1, 2,
			0, 2, 3,
			0, 3, 1,
			3, 2, 1
		};

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tetrahedron_verts) + sizeof(tetrahedron_indices), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(tetrahedron_indices), tetrahedron_indices);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tetrahedron_indices), sizeof(tetrahedron_verts), tetrahedron_verts);

		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)sizeof(tetrahedron_indices));
		glEnableVertexAttribArray(0);

		glEnable(GL_CULL_FACE);
		// glDisable(GL_CULL_FACE);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	}

	virtual void render(double currentTime)
	{
		static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		static const GLfloat one = 1.0f;
		float f = (float)currentTime;

		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glClearBufferfv(GL_COLOR, 0, black);
		glClearBufferfv(GL_DEPTH, 0, &one);

		glUseProgram(program);

		glm::mat4 proj_matrix = glm::perspective( glm::radians(50.0f),
			(float)info.windowWidth / (float)info.windowHeight,
			0.1f,
			1000.0f);
		glm::mat4 mv_matrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, -10.5f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians((float)currentTime * 71.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians((float)currentTime * 10.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(proj_matrix * mv_matrix));
		glUniformMatrix4fv(mv_location, 1, GL_FALSE, glm::value_ptr(mv_matrix));
		glUniform1f(stretch_location, sinf(f * 4.0f) * 0.75f + 1.0f);

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_SHORT, NULL);
	}

	virtual void shutdown()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteProgram(program);
		glDeleteBuffers(1, &buffer);
	}

protected:
	GLuint          program;
	GLuint          vao;

	GLuint			mv_location;
	GLuint			mvp_location;
	GLuint			stretch_location;
	GLuint			buffer;
};

/** @} @} */

int main(int argc, char** argv)
{
	gstessellate_app a;
	a.run();
	return 0;
}