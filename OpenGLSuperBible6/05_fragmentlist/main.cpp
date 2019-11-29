#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

/** \addtogroup Chapter05
*   @{
*       \addtogroup fragmentlist
* 实现片段链表
* 使用原子计数器, 着色器可写入的图像(保存索引), 巨大的共享存储块, 实现一个模型的深度透视图.
* 使用一张图保存每个像素的索引, 而后使用索引在一个链表内查询该图每个位置的累积渲染内容.
*	- 通过 GL_UNIFORM_BUFFER 使用 uniform block buffer.
*	- 使用 GL_SHADER_STORAGE_BUFFER 共享存储块, 该块可以被着色器写入内容.
* 	- 使用 GL_ATOMIC_COUNTER_BUFFER 原子计数器 buffer.
* 	- 2维纹理使用的格式 GL_R32UI
* 	- glMemoryBarrier() --- 确保任何在其子系统的访问都应先完成. 这里设置了 GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT.
* 	- glBindBufferBase 将一个 buffer object 绑定至着色器使用(原子计数器, 共享存储)
* 	- 使用 glBindImageTexture 绑定一个图像用于操作
* 	- 本例使用了 1024 x 1024 的大小, 注意窗口不能超过该大小, 后修改成 2048
* 	 
* 	- 首先使用 clear_program 绘制整个屏幕
* 	 	- 顶点着色器绘制整个屏幕
* 	 	- 片段着色器
* 	 		- layout (binding = 0, r32ui) coherent uniform uimage2D head_pointer;
* 	 		- 使用 uimage2D 的一致变量可以在着色器中写入纹理.
* 	 		- coherent 变量, 默认变量仅当前调用可修改, 其他调用看不到其写的值.使用该前缀可以确保依赖性的着色器, 该变量需要执行合适的内存栅栏
* 	 	- 片段着色器将纹理的内容写为0xFFFFFFFF
* 	 
* 	- 而后使用 append_program 绘制模型
* 	 	- 顶点着色器发送单元坐标给片段着色器.
* 	 	- 片段着色器
* 	 		- 原子计数器绑定至0, 偏移为0
* 	 		- 使用原子计数器保存一共调用了多少次片段着色器, 每次其值作为索引.
* 	 		- list_item_block 为 uniform 块, 内有 list_item 数组, 其作为 shared buffer, 可以在着色器中写入.
* 	 		- 使用 imageAtomicExchange 在图像一点存储新的索引值, 返回原先的索引值.
* 	 		
* 	- 最后使用 resolve_program 绘制整个屏幕
* 	 	- 顶点着色器同样是绘制整个屏幕
* 	 	- 片段着色器
* 	 		- 获取当前像素的索引值
* 	 		- 根据正反累加当前的深度值
* 	 		- 使用最后累加的深度值作为颜色结果
* 	 
* 	- 其有第二个着色器得到的数据未用到, 如颜色值.
*
*       @{
*/
class fragmentlist_app : public OpenGLApp
{
public:
	fragmentlist_app()
		: clear_program(0),
		append_program(0),
		resolve_program(0)
	{
	}

	void init()
	{
		static const char title[] = "OpenGL SuperBible - Fragment List";

		OpenGLApp::init();

		memcpy(info.title, title, sizeof(title));
	}

	void startup()
	{
		load_shaders();

		// 通过 GL_UNIFORM_BUFFER 使用 uniform block buffer.
		glGenBuffers(1, &uniforms_buffer);
		glBindBuffer(GL_UNIFORM_BUFFER, uniforms_buffer);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(uniforms_block), NULL, GL_DYNAMIC_DRAW);

		obj.load("../media/models/dragon.sbm");

		// 使用 GL_SHADER_STORAGE_BUFFER 共享存储块, 该块可以被着色器写入内容.
		glGenBuffers(1, &fragment_buffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, fragment_buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 2048 * 2048 * 16, NULL, GL_DYNAMIC_COPY);

		// 使用 GL_ATOMIC_COUNTER_BUFFER 原子计数器 buffer.
		glGenBuffers(1, &atomic_counter_buffer);
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomic_counter_buffer);
		glBufferData(GL_ATOMIC_COUNTER_BUFFER, 4, NULL, GL_DYNAMIC_COPY);

		// 2维纹理使用的格式 GL_R32UI
		glGenTextures(1, &head_pointer_image);
		glBindTexture(GL_TEXTURE_2D, head_pointer_image);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, 2048, 2048);

		glGenVertexArrays(1, &dummy_vao);
		glBindVertexArray(dummy_vao);
	}

	void render(double currentTime)
	{
		static const GLfloat zeros[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		static const GLfloat gray[] = { 0.1f, 0.1f, 0.1f, 0.0f };
		static const GLfloat ones[] = { 1.0f };
		const float f = (float)currentTime;

		glViewport(0, 0, info.windowWidth, info.windowHeight);
		
		// glMemoryBarrier() --- 确保任何在其子系统的访问都应先完成. 这里设置了 GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT.
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

		// 首先绘制 4 个顶点表示的两个三角形.
		// 首先使用 clear_program 绘制整个屏幕, imageStore 填充内容.
		glUseProgram(clear_program);
		glBindVertexArray(dummy_vao);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		// 而后使用 append_program
		glUseProgram(append_program);
		
		glm::mat4 model_matrix = glm::scale(glm::mat4(1.0), glm::vec3(7.0, 7.0, 7.0));
		glm::vec3 view_position = glm::vec3(cosf(f * 0.35f) * 120.0f, cosf(f * 0.4f) * 30.0f, sinf(f * 0.35f) * 120.0f);
		glm::mat4 view_matrix = glm::lookAt(view_position, glm::vec3(0.0f, 30.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 mv_matrix = view_matrix * model_matrix;
		glm::mat4 proj_matrix = glm::perspective(glm::radians(50.0f), (float)info.windowWidth / (float)info.windowHeight,
			0.1f, 1000.0f);

		glm::mat4 projmvmat = proj_matrix * mv_matrix;

		glUniformMatrix4fv(uniforms.mvp, 1, GL_FALSE, &projmvmat[0][0]);
		
		// glBindBufferBase 将一个 buffer object 绑定至着色器使用(原子计数器, 共享存储)
		static const unsigned int zero = 0;
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomic_counter_buffer);
		glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(zero), &zero);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, fragment_buffer);

		// 使用 glBindImageTexture 绑定一个图像用于操作
		glBindImageTexture(0, head_pointer_image, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

		// 渲染之前和之后同步, 绘制物体
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

		obj.render();

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

		// 最后使用 resolve_program 绘制整个屏幕.
		glUseProgram(resolve_program);

		glBindVertexArray(dummy_vao);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

private:
	void load_shaders()
	{
		if (clear_program)
			glDeleteProgram(clear_program);
		clear_program = LoadShadersVF("../media/glsl/05_fragmentlist/clear.vs", "../media/glsl/05_fragmentlist/clear.fs");
		if (append_program)
			glDeleteProgram(append_program);
		append_program = LoadShadersVF("../media/glsl/05_fragmentlist/append.vs", "../media/glsl/05_fragmentlist/append.fs");
		uniforms.mvp = glGetUniformLocation(append_program, "mvp");
		if (resolve_program)
			glDeleteProgram(resolve_program);
		resolve_program = LoadShadersVF("../media/glsl/05_fragmentlist/resolve.vs", "../media/glsl/05_fragmentlist/resolve.fs");
	}

	GLuint          clear_program;
	GLuint          append_program;
	GLuint          resolve_program;

	struct
	{
		GLuint      color;
		GLuint      normals;
	} textures;

	struct uniforms_block
	{
		glm::mat4     mv_matrix;
		glm::mat4     view_matrix;
		glm::mat4     proj_matrix;
	};

	GLuint          uniforms_buffer;

	struct
	{
		GLint           mvp;
	} uniforms;
	sb6::object			obj;
	GLuint          fragment_buffer;
	GLuint          head_pointer_image;
	GLuint          atomic_counter_buffer;
	GLuint          dummy_vao;
};

/** @} @} */

int main(int argc, char** argv)
{
	fragmentlist_app a;
	a.run();
	return 0;
}