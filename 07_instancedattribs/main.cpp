#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/** \addtogroup Chapter07*   @{*       \addtogroup instancedattrib* 一个实例读取一次数据.*	- 多个数据使用同一个 buffer.*	- glVertexAttribDivisor() 间隔多少个 instance 读取一次数据.*	- glDrawArraysInstancedBaseInstance() 绘制多个实例.*       @{*/

class instancing_app : public OpenGLApp
{
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Instanced Attributes";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}

	virtual void startup()
	{
		static const GLfloat square_vertices[] = {
			-1.0f, -1.0f, 0.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f,
			1.0f, 1.0f, 0.0f, 1.0f,
			-1.0f, 1.0f, 0.0f, 1.0f
		};
		static const GLfloat instanced_color[] = {
			1.0f, 0.0f, 0.0f, 1.0f,
			0.0f, 1.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 0.0f, 1.0f
		};
		static const GLfloat instance_positions[] = {
			-2.0f, -2.0f, 0.0f, 0.0f,
			2.0f, -2.0f, 0.0f, 0.0f,
			2.0f, 2.0f, 0.0f, 0.0f,
			-2.0f, 2.0f, 0.0f, 0.0f
		};

		GLuint offset = 0;

		program = LoadShadersVF("../media/glsl/07_instancedattribs.vs", "../media/glsl/07_instancedattribs.fs");
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &square_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, square_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(square_vertices) + sizeof(instanced_color) + sizeof(instance_positions), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(square_vertices), square_vertices);
		offset += sizeof(square_vertices);
		glBufferSubData(GL_ARRAY_BUFFER,offset, sizeof(instanced_color), instanced_color);
		offset += sizeof(instanced_color);
		glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(instance_positions), instance_positions);
		offset += sizeof(instance_positions);

		// 多个数据使用同一个 buffer.
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)sizeof(square_vertices));
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(square_vertices) + sizeof(instanced_color)));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		glVertexAttribDivisor(1, 1);
		glVertexAttribDivisor(2, 1);
	}

	virtual void render(double currentTime)
	{
		static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, black);

		glUseProgram(program);
		glDrawArraysInstancedBaseInstance(GL_TRIANGLE_FAN, 0, 4, 4, 0);
	}

	virtual void shutdown()
	{
		glDeleteBuffers(1, &square_buffer);
		glDeleteVertexArrays(1, &vao);
		glDeleteProgram(program);
	}

protected:
	GLuint          program;
	GLuint          vao;

	GLuint			square_buffer;
};

int main(int argc, char** argv)
{
	instancing_app a;
	a.run();
	return 0;
}