#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/** \addtogroup Chapter07*   @{*       \addtogroup multidrawindirect* 多重间接绘制.*	- 本例设置了间接绘制的命令结构*	- 本例保存了 50000 个简洁命令的缓存, 其类型为 GL_DRAW_INDIRECT_BUFFER*	- glMapBufferRange() 使用 GL_MAP_INVALIDATE_BUFFER_BIT 标识表示buffer内之前的内容可丢弃.*	- 还设置了 index array buffer, 绘制完一个实例就换一个索引.*	- glMultiDrawArraysIndirect() 间接绘制.*       @{*/

/** 间接绘制的命令结构体 */
struct DrawArraysIndirectCommand
{
	GLuint count;
	GLuint primCount;
	GLuint first;
	GLuint baseInstance;
};

class multidrawindirect_app : public OpenGLApp
{
public:
	multidrawindirect_app():program(0),
		mode(MODE_MULTIDRAW), paused(false), vsync(false)
	{}
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Asteroids";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}

	virtual void startup();

	virtual void render(double currentTime);

	virtual void shutdown()
	{
		object.free();
		//glDeleteVertexArrays(1, &vao);
		glDeleteProgram(program);
	}

	void load_shaders();

protected:
	GLuint          program;
	//GLuint          vao;
	sb6::object		object;

	GLuint			indirect_draw_buffer;
	GLuint			draw_index_buffer;

	struct {
		GLuint		time;
		GLuint		view_matrix;
		GLuint		proj_matrix;
		GLuint		viewproj_matrix;
	} uniforms;

	enum 
	{
		NUM_DRAWS = 50000
	};

	enum MODE
	{
		MODE_FIRST,
		MODE_MULTIDRAW = 0,
		MODE_SEPARATE_DRAWS,
		MODE_MAX = MODE_SEPARATE_DRAWS
	};
	MODE mode;
	bool paused;
	bool vsync;
};

void multidrawindirect_app::load_shaders()
{
	if(program)
		glDeleteProgram(program);
	program = LoadShadersVF("../media/glsl/07_multidrawindirect.vs", "../media/glsl/07_multidrawindirect.fs");

	uniforms.time = glGetUniformLocation(program, "time");
	uniforms.view_matrix = glGetUniformLocation(program, "view_matrix");
	uniforms.proj_matrix = glGetUniformLocation(program, "proj_matrix");
	uniforms.viewproj_matrix = glGetUniformLocation(program, "viewproj_matrix");
}

void multidrawindirect_app::startup()
{
	program = 0;
	load_shaders();
	object.load("../media/models/asteroids.sbm");

	// 本例保存了 50000 个简洁命令的缓存, 其类型为 GL_DRAW_INDIRECT_BUFFER
	glGenBuffers(1, &indirect_draw_buffer);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_draw_buffer);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, 
		NUM_DRAWS * sizeof(DrawArraysIndirectCommand),
		NULL,
		GL_STATIC_DRAW);

	// glMapBufferRange() 使用 GL_MAP_INVALIDATE_BUFFER_BIT 标识表示buffer内之前的内容可丢弃.
	DrawArraysIndirectCommand* cmd = (DrawArraysIndirectCommand*)
		glMapBufferRange(GL_DRAW_INDIRECT_BUFFER,
		0,
		NUM_DRAWS*sizeof(DrawArraysIndirectCommand),
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	for (int i = 0; i < NUM_DRAWS; i++)
	{
		object.get_sub_object_info(i%object.get_sub_object_count(),
			cmd[i].first,
			cmd[i].count);
		cmd[i].primCount = 1;
		cmd[i].baseInstance = i;
	}

	glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);

	glBindVertexArray(object.get_vao());
	//glGenVertexArrays(1, &vao);
	//glBindVertexArray(vao);

	glGenBuffers(1, &draw_index_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, draw_index_buffer);
	glBufferData(GL_ARRAY_BUFFER,
		NUM_DRAWS * sizeof(GLuint),
		NULL,
		GL_STATIC_DRAW);

	GLuint* draw_index = (GLuint*)
		glMapBufferRange(GL_ARRAY_BUFFER,
						0,
						NUM_DRAWS*sizeof(GLuint),
						GL_MAP_WRITE_BIT |
						GL_MAP_INVALIDATE_BUFFER_BIT);
	for (int i = 0; i < NUM_DRAWS; i++)
	{
		draw_index[i] = i;
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);

	glVertexAttribIPointer(10, 1, GL_UNSIGNED_INT, 0, NULL);
	glVertexAttribDivisor(10, 1);
	glEnableVertexAttribArray(10);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_CULL_FACE);
}

void multidrawindirect_app::render(double currentTime)
{
	int j;
	static const GLfloat one = 1.0f;
	static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glClearBufferfv(GL_DEPTH, 0, &one);
	glClearBufferfv(GL_COLOR, 0, black);

	static double last_time = currentTime;
	static double totoal_time = 0.0;

	if(!paused)
		totoal_time += (currentTime - last_time);
	last_time = currentTime;

	float t = float(totoal_time);
	int i = int(totoal_time * 3.0f);

	const glm::mat4 view_matrix = glm::lookAt(glm::vec3(100.0f * cosf(t * 0.023f), 100.0f * cosf(t * 0.023f), 300.0f * sinf(t * 0.037f) - 600.0f),
		glm::vec3(0.0f, 0.0f, 260.0f),
		glm::normalize(glm::vec3(0.1f - cosf(t * 0.1f) * 0.3f, 1.0f, 0.0f)));
	const glm::mat4 proj_matrix = glm::perspective(glm::radians(50.0f),
		(float)info.windowWidth / (float)info.windowHeight,
		1.0f,
		2000.0f);
	glm::mat4 viewproj_matrix = proj_matrix * view_matrix;

	glUseProgram(program);

	glUniform1f(uniforms.time, t);
	glUniformMatrix4fv(uniforms.view_matrix, 1, GL_FALSE, &view_matrix[0][0]);
	glUniformMatrix4fv(uniforms.proj_matrix, 1, GL_FALSE, &proj_matrix[0][0]);
	glUniformMatrix4fv(uniforms.viewproj_matrix, 1, GL_FALSE, &viewproj_matrix[0][0]);

	glBindVertexArray(object.get_vao());

	if(mode == MODE_MULTIDRAW)
	{
		glMultiDrawArraysIndirect(GL_TRIANGLES, NULL, NUM_DRAWS, 0);
	}
	else if(mode == MODE_SEPARATE_DRAWS)
	{
		for (j = 0; j < NUM_DRAWS; j++)
		{
			GLuint first, count;
			object.get_sub_object_info(j%object.get_sub_object_count(), first, count);
			glDrawArraysInstancedBaseInstance(GL_TRIANGLES,
											  first, 
											  count, 
											  1, 
											  j);
		}
	}
}
/** @} @} */
int main(int argc, char** argv)
{
	multidrawindirect_app a;
	a.run();
	return 0;
}