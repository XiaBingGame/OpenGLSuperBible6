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
* ʵ��Ƭ������
* ʹ��ԭ�Ӽ�����, ��ɫ����д���ͼ��(��������), �޴�Ĺ���洢��, ʵ��һ��ģ�͵����͸��ͼ.
* ʹ��һ��ͼ����ÿ�����ص�����, ����ʹ��������һ�������ڲ�ѯ��ͼÿ��λ�õ��ۻ���Ⱦ����.
*	- ͨ�� GL_UNIFORM_BUFFER ʹ�� uniform block buffer.
*	- ʹ�� GL_SHADER_STORAGE_BUFFER ����洢��, �ÿ���Ա���ɫ��д������.
* 	- ʹ�� GL_ATOMIC_COUNTER_BUFFER ԭ�Ӽ����� buffer.
* 	- 2ά����ʹ�õĸ�ʽ GL_R32UI
* 	- glMemoryBarrier() --- ȷ���κ�������ϵͳ�ķ��ʶ�Ӧ�����. ���������� GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT.
* 	- glBindBufferBase ��һ�� buffer object ������ɫ��ʹ��(ԭ�Ӽ�����, ����洢)
* 	- ʹ�� glBindImageTexture ��һ��ͼ�����ڲ���
* 	- ����ʹ���� 1024 x 1024 �Ĵ�С, ע�ⴰ�ڲ��ܳ����ô�С, ���޸ĳ� 2048
* 	 
* 	- ����ʹ�� clear_program ����������Ļ
* 	 	- ������ɫ������������Ļ
* 	 	- Ƭ����ɫ��
* 	 		- layout (binding = 0, r32ui) coherent uniform uimage2D head_pointer;
* 	 		- ʹ�� uimage2D ��һ�±�����������ɫ����д������.
* 	 		- coherent ����, Ĭ�ϱ�������ǰ���ÿ��޸�, �������ÿ�������д��ֵ.ʹ�ø�ǰ׺����ȷ�������Ե���ɫ��, �ñ�����Ҫִ�к��ʵ��ڴ�դ��
* 	 	- Ƭ����ɫ�������������дΪ0xFFFFFFFF
* 	 
* 	- ����ʹ�� append_program ����ģ��
* 	 	- ������ɫ�����͵�Ԫ�����Ƭ����ɫ��.
* 	 	- Ƭ����ɫ��
* 	 		- ԭ�Ӽ���������0, ƫ��Ϊ0
* 	 		- ʹ��ԭ�Ӽ���������һ�������˶��ٴ�Ƭ����ɫ��, ÿ����ֵ��Ϊ����.
* 	 		- list_item_block Ϊ uniform ��, ���� list_item ����, ����Ϊ shared buffer, ��������ɫ����д��.
* 	 		- ʹ�� imageAtomicExchange ��ͼ��һ��洢�µ�����ֵ, ����ԭ�ȵ�����ֵ.
* 	 		
* 	- ���ʹ�� resolve_program ����������Ļ
* 	 	- ������ɫ��ͬ���ǻ���������Ļ
* 	 	- Ƭ����ɫ��
* 	 		- ��ȡ��ǰ���ص�����ֵ
* 	 		- ���������ۼӵ�ǰ�����ֵ
* 	 		- ʹ������ۼӵ����ֵ��Ϊ��ɫ���
* 	 
* 	- ���еڶ�����ɫ���õ�������δ�õ�, ����ɫֵ.
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

		// ͨ�� GL_UNIFORM_BUFFER ʹ�� uniform block buffer.
		glGenBuffers(1, &uniforms_buffer);
		glBindBuffer(GL_UNIFORM_BUFFER, uniforms_buffer);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(uniforms_block), NULL, GL_DYNAMIC_DRAW);

		obj.load("../media/models/dragon.sbm");

		// ʹ�� GL_SHADER_STORAGE_BUFFER ����洢��, �ÿ���Ա���ɫ��д������.
		glGenBuffers(1, &fragment_buffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, fragment_buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 2048 * 2048 * 16, NULL, GL_DYNAMIC_COPY);

		// ʹ�� GL_ATOMIC_COUNTER_BUFFER ԭ�Ӽ����� buffer.
		glGenBuffers(1, &atomic_counter_buffer);
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomic_counter_buffer);
		glBufferData(GL_ATOMIC_COUNTER_BUFFER, 4, NULL, GL_DYNAMIC_COPY);

		// 2ά����ʹ�õĸ�ʽ GL_R32UI
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
		
		// glMemoryBarrier() --- ȷ���κ�������ϵͳ�ķ��ʶ�Ӧ�����. ���������� GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT.
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

		// ���Ȼ��� 4 �������ʾ������������.
		// ����ʹ�� clear_program ����������Ļ, imageStore �������.
		glUseProgram(clear_program);
		glBindVertexArray(dummy_vao);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		// ����ʹ�� append_program
		glUseProgram(append_program);
		
		glm::mat4 model_matrix = glm::scale(glm::mat4(1.0), glm::vec3(7.0, 7.0, 7.0));
		glm::vec3 view_position = glm::vec3(cosf(f * 0.35f) * 120.0f, cosf(f * 0.4f) * 30.0f, sinf(f * 0.35f) * 120.0f);
		glm::mat4 view_matrix = glm::lookAt(view_position, glm::vec3(0.0f, 30.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 mv_matrix = view_matrix * model_matrix;
		glm::mat4 proj_matrix = glm::perspective(glm::radians(50.0f), (float)info.windowWidth / (float)info.windowHeight,
			0.1f, 1000.0f);

		glm::mat4 projmvmat = proj_matrix * mv_matrix;

		glUniformMatrix4fv(uniforms.mvp, 1, GL_FALSE, &projmvmat[0][0]);
		
		// glBindBufferBase ��һ�� buffer object ������ɫ��ʹ��(ԭ�Ӽ�����, ����洢)
		static const unsigned int zero = 0;
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomic_counter_buffer);
		glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(zero), &zero);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, fragment_buffer);

		// ʹ�� glBindImageTexture ��һ��ͼ�����ڲ���
		glBindImageTexture(0, head_pointer_image, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

		// ��Ⱦ֮ǰ��֮��ͬ��, ��������
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

		obj.render();

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

		// ���ʹ�� resolve_program ����������Ļ.
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