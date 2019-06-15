#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/** \addtogroup Chapter09*   @{*       \addtogroup gslayered* 本例主要演示如果多次绘制一个场景并将其绘制入*	- 本例还演示了将 uniform block 绑定至一个 buffer.*	- 使用 uniform buffer, GL_UNIFORM_BUFFER.*	- 这里多层颜色使用纹理数组实现, 其在几何着色器中使用 gl_Layer 选择纹理数组的层索引. 其包括颜色纹理和深度纹理, 格式分别为 GL_RGBA8 和 GL_DEPTH_COMPONENT32*	- glGenFramebuffers(), glBindFramebuffer() 生成和绑定 Frame Buffer.*	- glFramebufferTexture 设置buffer 的纹理目标, 最后一个参数 level 为 mipmap level.*	- glBindBufferBase 将一个 buffer 绑定至索引绑定点, 这里用于 GL_UNIFORM_BUFFER. 其所用的索引可以在着色器中片段着色器的location或者使用 GL_UNIFORM_BUFFER 设置.*	- 绑定一个 GL_FRAMEBUFFER 之后, 使用 glDrawBuffers() 设置绘制的挂接点. glBindFramebuffer() 绑定至0 可以取消绑定.
*	- glDrawBuffer() 及 GL_BACK 绘制至屏幕.
*	- 本例绘制 16 个正方形, 布满屏幕, 使用贴图.*	- 几何着色器中使用 invocations 前缀将一个图元绘制多遍.这里绘制了 16 遍.*	- 使用 glMapBufferRange 得到地址, 使用 glUnMapBuffer 取消映射*	- gl_InvocationID 的范围为[0, N-1], N 表示每个图元绘制的次数*	- gl_Layer 表示多层 framebuffer 挂接点时, 选择哪一层.*       @{*/

class gslayered_app : public OpenGLApp
{
public:
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Layered Rendering";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		load_shaders();
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		obj.load("../media/models/torus.sbm");

		// 使用 uniform buffer, GL_UNIFORM_BUFFER.
		glGenBuffers(1, &transform_ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, transform_ubo);
		glBufferData(GL_UNIFORM_BUFFER, 17 * 16 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
		// 使用纹理数组, 其包括颜色纹理和深度纹理, 格式分别为 GL_RGBA8 和 GL_DEPTH_COMPONENT32
		glGenTextures(1, &array_texture);
		glBindTexture(GL_TEXTURE_2D_ARRAY, array_texture);
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, 256, 256, 16);
		
		glGenTextures(1, &array_depth);
		glBindTexture(GL_TEXTURE_2D_ARRAY, array_depth);
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT32, 256, 256, 16);

		// glFramebufferTexture 设置buffer 的纹理目标, 最后一个参数 level 为 mipmap level.
		// glGenFramebuffers(), glBindFramebuffer() 生成和绑定 Frame Buffer.
		glGenFramebuffers(1, &layered_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, layered_fbo);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, array_texture, 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, array_depth, 0);
	}

	virtual void render(double t)
	{
		static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		static const GLfloat gray[] =  { 0.1f, 0.1f, 0.1f, 1.0f };
		static const GLfloat one = 1.0f;

		glm::mat4 mv_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians((float)t * 5.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians((float)t * 30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 proj_matrix = glm::perspective(glm::radians(50.0f), (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);
		glm::mat4 mvp = proj_matrix * mv_matrix;

		

		struct TRANSFORM_BUFFER
		{
			mmat4 proj_matrix;
			mmat4 mv_matrix[16];
		};

		unsigned int matsize = 16 * sizeof(float);

		// glBindBufferBase 将一个 buffer 绑定至索引绑定点, 这里用于 GL_UNIFORM_BUFFER. 其所用的索引可以在着色器中片段着色器的location或者使用 GL_UNIFORM_BUFFER 设置.
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, transform_ubo);

		// 使用 glMapBufferRange 得到地址.
		TRANSFORM_BUFFER * buffer = (TRANSFORM_BUFFER *)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(TRANSFORM_BUFFER), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

		memcpy(buffer->proj_matrix, glm::value_ptr(glm::perspective(glm::radians(50.0f), 1.0f, 0.1f, 1000.0f)), matsize); // proj_matrix;
		int i;

		for (i = 0; i < 16; i++)
		{
			float fi = (float)(i + 12) / 16.0f;

			memcpy(buffer->mv_matrix[i], glm::value_ptr(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians((float)t * 25.0f * fi), glm::vec3(0.0f, 0.0f, 1.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians((float)t * 30.0f * fi), glm::vec3(1.0f, 0.0f, 0.0f))), matsize);
		}
		// glUnmapBuffer 解除映射.
		glUnmapBuffer(GL_UNIFORM_BUFFER);

		static const GLenum ca0 = GL_COLOR_ATTACHMENT0;

		glBindFramebuffer(GL_FRAMEBUFFER, layered_fbo);
		// 绑定一个 GL_FRAMEBUFFER 之后, 使用 glDrawBuffers() 设置绘制的挂接点.
		glDrawBuffers(1, &ca0);
		glViewport(0, 0, 256, 256);
		glClearBufferfv(GL_COLOR, 0, black);
		glClearBufferfv(GL_DEPTH, 0, &one);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glUseProgram(program_gslayers);

		obj.render();

		// glBindFramebuffer() 绑定至0 可以取消绑定.
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// glDrawBuffer() 及 GL_BACK 绘制至屏幕.
		glDrawBuffer(GL_BACK);
		glUseProgram(program_showlayers);

		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glClearBufferfv(GL_COLOR, 0, gray);

		glBindTexture(GL_TEXTURE_2D_ARRAY, array_texture);
		glDisable(GL_DEPTH_TEST);

		// 绘制 16 个正方形, 布满屏幕, 使用贴图.
		glBindVertexArray(vao);
		glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 16);

		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	}

	virtual void shutdown()
	{
		obj.free();
		glDeleteProgram(program_showlayers);
		glDeleteProgram(program_gslayers);
		glDeleteVertexArrays(1, &vao);
	}

private:
	void load_shaders();

protected:
	GLuint      program_gslayers;
	GLuint      program_showlayers;
	GLuint      vao;
	int         mode;
	GLuint      transform_ubo;

	GLuint      layered_fbo;
	GLuint      array_texture;
	GLuint      array_depth;

	sb6::object obj;
};

/** @} @} */

void gslayered_app::load_shaders()
{
	if (program_showlayers)
		glDeleteProgram(program_showlayers);

	program_showlayers = LoadShadersVF("../media/glsl/09_gslayered/showlayers.vs", "../media/glsl/09_gslayered/showlayers.fs");

	if (program_gslayers)
		glDeleteProgram(program_gslayers);

	program_gslayers = LoadShadersVGF("../media/glsl/09_gslayered/gslayers.vs", "../media/glsl/09_gslayered/gslayers.gs", "../media/glsl/09_gslayered/gslayers.fs");
}

int main(int argc, char** argv)
{
	gslayered_app a;
	a.run();
	return 0;
}