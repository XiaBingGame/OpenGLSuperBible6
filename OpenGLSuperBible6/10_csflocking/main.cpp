#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

/** \addtogroup Chapter10*   @{*       \addtogroup csflocking* 使用计算着色器和双缓存技术绘制大量的鸟群.**	- 本例创建了 GL_SHADER_STORAGE_BUFFER, 用于着色器中buffer前缀的块. 本例创建了两个 buffer, 一个用于输入, 一个用于输出.*	- 对于两个不同的 buffer, 其设置不同的VAO, 因此切换 VAO 就可以实现双缓存类似的更新和绘制.*	- glDispatchCompute() 设置工作组的数量, 着色器使用 local_size_x 的前缀设置内部本地的项数量*	- glVertexAttribDivisor() 设置了顶点着色器如何读取每个实例的属性.*	- 在计算着色器中的每一次运行中, 便利了所有组的所有项计算出中心位置以及加速度.*       @{*/

enum
{
	WORKGROUP_SIZE  = 256,
	NUM_WORKGROUPS  = 64,
	FLOCK_SIZE      = (NUM_WORKGROUPS * WORKGROUP_SIZE)
};

class csflocking_app : public OpenGLApp
{
public:
	csflocking_app() 
		: frame_index(0),
		  flock_update_program(0),
		  flock_render_program(0) {}
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Compute Shader Flocking";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		// This is position and normal data for a paper airplane
		static const glm::vec3 geometry[] =
		{
			// Positions
			glm::vec3(-5.0f, 1.0f, 0.0f),
			glm::vec3(-1.0f, 1.5f, 0.0f),
			glm::vec3(-1.0f, 1.5f, 7.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 10.0f),
			glm::vec3(1.0f, 1.5f, 0.0f),
			glm::vec3(1.0f, 1.5f, 7.0f),
			glm::vec3(5.0f, 1.0f, 0.0f),

			// Normals
			glm::vec3(0.0f),
			glm::vec3(0.0f),
			glm::vec3(0.107f, -0.859f, 0.00f),
			glm::vec3(0.832f, 0.554f, 0.00f),
			glm::vec3(-0.59f, -0.395f, 0.00f),
			glm::vec3(-0.832f, 0.554f, 0.00f),
			glm::vec3(0.295f, -0.196f, 0.00f),
			glm::vec3(0.124f, 0.992f, 0.00f),
		};

		load_shaders();

		glGenBuffers(2, flock_buffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, flock_buffer[0]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, FLOCK_SIZE * sizeof(flock_member), NULL, GL_DYNAMIC_COPY);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, flock_buffer[1]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, FLOCK_SIZE * sizeof(flock_member), NULL, GL_DYNAMIC_COPY);

		int i;

		glGenBuffers(1, &geometry_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, geometry_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(geometry), geometry, GL_STATIC_DRAW);

		glGenVertexArrays(2, flock_render_vao);

		for (i = 0; i < 2; i++)
		{
			glBindVertexArray(flock_render_vao[i]);
			glBindBuffer(GL_ARRAY_BUFFER, geometry_buffer);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)(8 * sizeof(glm::vec3)));

			glBindBuffer(GL_ARRAY_BUFFER, flock_buffer[i]);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(flock_member), NULL);
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(flock_member), (void *)sizeof(glm::vec4));
			glVertexAttribDivisor(2, 1);
			glVertexAttribDivisor(3, 1);

			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glEnableVertexAttribArray(3);
		}

		glBindBuffer(GL_ARRAY_BUFFER, flock_buffer[0]);
		flock_member * ptr = reinterpret_cast<flock_member *>(glMapBufferRange(GL_ARRAY_BUFFER, 0, FLOCK_SIZE * sizeof(flock_member), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

		for (i = 0; i < FLOCK_SIZE; i++)
		{
			ptr[i].position = (glm::linearRand(glm::vec3(-1), glm::vec3(1))) * 300.0f;
			ptr[i].velocity = (glm::linearRand(glm::vec3(-1), glm::vec3(1)));
		}

		glUnmapBuffer(GL_ARRAY_BUFFER);
		
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	}

	virtual void render(double currentTime)
	{
		float t = (float)currentTime;
		static const float black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		static const float one = 1.0f;

		glUseProgram(flock_update_program);

		glm::vec3 goal = glm::vec3(sinf(t * 0.34f),
			cosf(t * 0.29f),
			sinf(t * 0.12f) * cosf(t * 0.5f));

		goal = goal * glm::vec3(35.0f, 25.0f, 60.0f);

		glUniform3fv(uniforms.update.goal, 1, glm::value_ptr(goal));

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, flock_buffer[frame_index]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, flock_buffer[frame_index ^ 1]);

		glDispatchCompute(NUM_WORKGROUPS, 1, 1);

		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glClearBufferfv(GL_COLOR, 0, black);
		glClearBufferfv(GL_DEPTH, 0, &one);

		glUseProgram(flock_render_program);

		glm::mat4 mv_matrix = glm::lookAt(glm::vec3(0.0f, 0.0f, -400.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 proj_matrix = glm::perspective(glm::radians(60.0f),
			(float)info.windowWidth / (float)info.windowHeight,
			0.1f,
			3000.0f);
		glm::mat4 mvp = proj_matrix * mv_matrix;

		glUniformMatrix4fv(uniforms.render.mvp, 1, GL_FALSE, glm::value_ptr(mvp));

		glBindVertexArray(flock_render_vao[frame_index]);

		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 8, FLOCK_SIZE);

		frame_index ^= 1;
	}

	virtual void shutdown()
	{
		glDeleteVertexArrays(2, flock_render_vao);
		glDeleteProgram(flock_update_program);
		glDeleteProgram(flock_render_program);
		glDeleteBuffers(2, flock_buffer);
		glDeleteBuffers(1, &geometry_buffer);
	}

	void onKey(int key, int scancode, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			switch(key)
			{
			case 'R': 
				load_shaders();
				break;
			default:
				break;
			}
		}
		OpenGLApp::onKey(key, scancode, action, mods);
	}

private:
	void load_shaders();

protected:
	GLuint      flock_update_program;
	GLuint      flock_render_program;

	GLuint      flock_buffer[2];

	GLuint      flock_render_vao[2];
	GLuint      geometry_buffer;

	struct flock_member
	{
		glm::vec3 position;
		unsigned int : 32;
		glm::vec3 velocity;
		unsigned int : 32;
	};

	struct
	{
		struct
		{
			GLint       goal;
		} update;
		struct
		{
			GLuint      mvp;
		} render;
	} uniforms;

	GLuint      frame_index;
};

void csflocking_app::load_shaders()
{
	if (flock_update_program)
		glDeleteProgram(flock_update_program);

	if (flock_render_program)
		glDeleteProgram(flock_render_program);

	flock_update_program = LoadShadersCS("../media/glsl/10_csflocking/flocking.cs");
	flock_render_program = LoadShadersVF("../media/glsl/10_csflocking/render.vs", "../media/glsl/10_csflocking/render.fs");

	uniforms.update.goal = glGetUniformLocation(flock_update_program, "goal");
	uniforms.render.mvp = glGetUniformLocation(flock_render_program, "mvp");
}

/** @} @} */

int main(int argc, char** argv)
{
	csflocking_app a;
	a.run();
	return 0;
}