#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/** \addtogroup Chapter09*   @{*       \addtogroup basicfbo* 本例实现将渲染内容写入 fbo.* * 	- glGenFramebuffers() --- 生成 framebuffer
* 	- glBindFramebuffer() --- 绑定和解绑 framebuffer
* 	- 纹理格式使用 GL_RGBA8, GL_DEPTH_COMPONENT32F
* 	- fbo 挂接点 GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT
* 	- glDrawBuffers() 设置 fbo 中片段着色器会写入的挂接点.
* 	- program1 用来写入 framebuffer.
**       @{*/

class basicfbo_app : public OpenGLApp
{
public:
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Basic Framebuffer Object";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		program1 = LoadShadersVF("../media/glsl/09_basicfbo/program.vs", "../media/glsl/09_basicfbo/program1.fs");
		program2 = LoadShadersVF("../media/glsl/09_basicfbo/program.vs", "../media/glsl/09_basicfbo/program2.fs");

		mv_location = glGetUniformLocation(program1, "mv_matrix");
		proj_location = glGetUniformLocation(program1, "proj_matrix");
		mv_location2 = glGetUniformLocation(program2, "mv_matrix");
		proj_location2 = glGetUniformLocation(program2, "proj_matrix");

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);


		static const GLfloat vertex_data[] =
		{
			// Position                 Tex Coord
			-0.25f, -0.25f,  0.25f,      0.0f, 1.0f,
			-0.25f, -0.25f, -0.25f,      0.0f, 0.0f,
			0.25f, -0.25f, -0.25f,      1.0f, 0.0f,

			0.25f, -0.25f, -0.25f,      1.0f, 0.0f,
			0.25f, -0.25f,  0.25f,      1.0f, 1.0f,
			-0.25f, -0.25f,  0.25f,      0.0f, 1.0f,

			0.25f, -0.25f, -0.25f,      0.0f, 0.0f,
			0.25f,  0.25f, -0.25f,      1.0f, 0.0f,
			0.25f, -0.25f,  0.25f,      0.0f, 1.0f,

			0.25f,  0.25f, -0.25f,      1.0f, 0.0f,
			0.25f,  0.25f,  0.25f,      1.0f, 1.0f,
			0.25f, -0.25f,  0.25f,      0.0f, 1.0f,

			0.25f,  0.25f, -0.25f,      1.0f, 0.0f,
			-0.25f,  0.25f, -0.25f,      0.0f, 0.0f,
			0.25f,  0.25f,  0.25f,      1.0f, 1.0f,

			-0.25f,  0.25f, -0.25f,      0.0f, 0.0f,
			-0.25f,  0.25f,  0.25f,      0.0f, 1.0f,
			0.25f,  0.25f,  0.25f,      1.0f, 1.0f,

			-0.25f,  0.25f, -0.25f,      1.0f, 0.0f,
			-0.25f, -0.25f, -0.25f,      0.0f, 0.0f,
			-0.25f,  0.25f,  0.25f,      1.0f, 1.0f,

			-0.25f, -0.25f, -0.25f,      0.0f, 0.0f,
			-0.25f, -0.25f,  0.25f,      0.0f, 1.0f,
			-0.25f,  0.25f,  0.25f,      1.0f, 1.0f,

			-0.25f,  0.25f, -0.25f,      0.0f, 1.0f,
			0.25f,  0.25f, -0.25f,      1.0f, 1.0f,
			0.25f, -0.25f, -0.25f,      1.0f, 0.0f,

			0.25f, -0.25f, -0.25f,      1.0f, 0.0f,
			-0.25f, -0.25f, -0.25f,      0.0f, 0.0f,
			-0.25f,  0.25f, -0.25f,      0.0f, 1.0f,

			-0.25f, -0.25f,  0.25f,      0.0f, 0.0f,
			0.25f, -0.25f,  0.25f,      1.0f, 0.0f,
			0.25f,  0.25f,  0.25f,      1.0f, 1.0f,

			0.25f,  0.25f,  0.25f,      1.0f, 1.0f,
			-0.25f,  0.25f,  0.25f,      0.0f, 1.0f,
			-0.25f, -0.25f,  0.25f,      0.0f, 0.0f,
		};

		glGenBuffers(1, &position_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
		glBufferData(GL_ARRAY_BUFFER,
			sizeof(vertex_data),
			vertex_data,
			GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), NULL);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

		glEnable(GL_CULL_FACE);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		// 生成绑定 fbo
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		// 生成绑定设置颜色纹理, 纹理格式为 GL_RGBA8, 大小为 512, 512
		glGenTextures(1, &color_texture);
		glBindTexture(GL_TEXTURE_2D, color_texture);
		glTexStorage2D(GL_TEXTURE_2D, 9, GL_RGBA8, 512, 512);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// 生成绑定设置深度纹理
		glGenTextures(1, &depth_texture);
		glBindTexture(GL_TEXTURE_2D, depth_texture);
		glTexStorage2D(GL_TEXTURE_2D, 9, GL_DEPTH_COMPONENT32F, 512, 512);

		// 将两个纹理分别绑定至 fbo 的颜色和深度挂接点.
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_texture, 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture, 0);

		static const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
		// glDrawBuffers() 设置 fbo 中片段着色器会写入的挂接点.
		glDrawBuffers(1, draw_buffers);
	}

	virtual void render(double currentTime)
	{
		static const GLfloat green[] = { 0.0f, 0.1f, 0.0f, 1.0f };
		static const GLfloat blue[] = { 0.0f, 0.0f, 0.3f, 1.0f };
		static const GLfloat one = 1.0f;

		glm::mat4 proj_matrix = glm::perspective(glm::radians(50.0f),
			(float)info.windowWidth / (float)info.windowHeight,
			0.1f,
			1000.0f);

		float f = (float)currentTime * 0.3f;
		glm::mat4 mv_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f)) *
			glm::translate(glm::mat4(1.0f), glm::vec3(sinf(2.1f * f) * 0.5f,
			cosf(1.7f * f) * 0.5f,
			sinf(1.3f * f) * cosf(1.5f * f) * 2.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians((float)currentTime * 45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians((float)currentTime * 81.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		// glBindFramebuffer() 绑定和解绑 framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glViewport(0, 0, 512, 512);
		glClearBufferfv(GL_COLOR, 0, green);
		glClearBufferfv(GL_DEPTH, 0, &one);

		// program1 用来写入 framebuffer.
		glUseProgram(program1);

		glUniformMatrix4fv(proj_location, 1, GL_FALSE, glm::value_ptr(proj_matrix));
		glUniformMatrix4fv(mv_location, 1, GL_FALSE, glm::value_ptr(mv_matrix));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glClearBufferfv(GL_COLOR, 0, blue);
		glClearBufferfv(GL_DEPTH, 0, &one);

		glBindTexture(GL_TEXTURE_2D, color_texture);

		glUseProgram(program2);

		glUniformMatrix4fv(proj_location2, 1, GL_FALSE, glm::value_ptr(proj_matrix));
		glUniformMatrix4fv(mv_location2, 1, GL_FALSE, glm::value_ptr(mv_matrix));

		glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	virtual void shutdown()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteProgram(program1);
		glDeleteProgram(program2);
	}

protected:
	GLuint          program1;
	GLuint          program2;
	GLuint          vao;

	GLuint          position_buffer;
	GLuint          index_buffer;
	GLuint          fbo;
	GLuint          color_texture;
	GLuint          depth_texture;
	GLint           mv_location;
	GLint           proj_location;
	GLuint          mv_location2;
	GLuint          proj_location2;
};

/** @} @} */

int main(int argc, char** argv)
{
	basicfbo_app a;
	a.run();
	return 0;
}