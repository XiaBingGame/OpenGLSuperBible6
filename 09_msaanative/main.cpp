#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/** \addtogroup Chapter09*   @{*       \addtogroup msaanative* 多重采样技术, 本例通过 info.samples 实现多重采样.*       @{*/

#define MANY_CUBES

class msaanative_app : public OpenGLApp
{
public:
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Native MSAA";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));

		info.samples = 8;
	}
protected:
	virtual void startup()
	{
		program = LoadShadersVF("../media/glsl/09_msaanative.vs", "../media/glsl/09_msaanative.fs");
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		mv_location = glGetUniformLocation(program, "mv_matrix");
		proj_location = glGetUniformLocation(program, "proj_matrix");

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
		static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        static const GLfloat one = 1.0f;

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);

        glUseProgram(program);

        glm::mat4 proj_matrix = glm::perspective(glm::radians(50.0f),
                                                     (float)info.windowWidth / (float)info.windowHeight,
                                                     0.1f,
                                                     1000.0f);
        glUniformMatrix4fv(proj_location, 1, GL_FALSE, glm::value_ptr(proj_matrix));

        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

#ifdef MANY_CUBES
        for (int i = 0; i < 24; i++)
        {
            float f = (float)i + (float)currentTime * 0.3f;
            glm::mat4 mv_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -20.0f)) *
                                    glm::rotate(glm::mat4(1.0f), glm::radians((float)currentTime * 45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
                                    glm::rotate(glm::mat4(1.0f), glm::radians((float)currentTime * 21.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
                                    glm::translate(glm::mat4(1.0f), glm::vec3(sinf(2.1f * f) * 2.0f,
                                                     cosf(1.7f * f) * 2.0f,
                                                     sinf(1.3f * f) * cosf(1.5f * f) * 2.0f));
            glUniformMatrix4fv(mv_location, 1, GL_FALSE, glm::value_ptr(mv_matrix));
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
        }
#else
        float f = (float)currentTime * 0.3f;
        currentTime = 3.15;
        glm::mat4 mv_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f)) *
                                /*vmath::translate(sinf(2.1f * f) * 0.5f,
                                                    cosf(1.7f * f) * 0.5f,
                                                    sinf(1.3f * f) * cosf(1.5f * f) * 2.0f) **/
                                glm::rotate(glm::mat4(1.0f), glm::radians((float)currentTime * 45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
                                glm::rotate(glm::mat4(1.0f), glm::radians((float)currentTime * 81.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(mv_location, 1, GL_FALSE, glm::value_ptr(mv_matrix));
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
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
};

/** @} @} */

int main(int argc, char** argv)
{
	msaanative_app a;
	a.run();
	return 0;
}