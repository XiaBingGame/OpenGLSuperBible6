#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/** \addtogroup Chapter08*   @{*       \addtogroup gsquads* 默认两个三角形绘制一个四边形时, 其中一个三角形影响不了另外一个三角形的着色, 所以产生了不一致的颜色过渡.**	- 使用 lines_adjacency 一次性向几何着色器发送四个顶点实现绘制四边形, 几何着色器则在其内绘制两个三角形实现正方形.*	- 几何着色器中, 每个顶点都发送了其重心坐标以及对应的四个角的颜色, 从而实现整个四边形的着色平滑移动, 不会产生三角形的不平滑过度.*       @{*/

class gsquads_app : public OpenGLApp
{
public:
	gsquads_app() :
		  program_linesadjacency(0),
		  program_fans(0),
		  mode(0),
		  paused(0),
		  vid_offset(0)
	  {

	  }

	void init()
	{
		static const char title[] = "OpenGL SuperBible - Quad Rendering";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		load_shaders();
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

	virtual void render(double currentTime)
	{
		static const GLfloat black[] = { 0.0f, 0.25f, 0.0f, 1.0f };
		glViewport(0, 0, info.windowWidth, info.windowHeight);

		static double last_time = 0.0;
		static double total_time = 0.0;

		if (!paused)
			total_time += (currentTime - last_time);
		last_time = currentTime;

		float t = (float)total_time;

		glClearBufferfv(GL_COLOR, 0, black);

		glm::mat4 mv_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians((float)t * 5.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians((float)t * 30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 proj_matrix = glm::perspective(glm::radians(50.0f), (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);
		glm::mat4 mvp = proj_matrix * mv_matrix;

		switch (mode)
		{
		case 0:
			glUseProgram(program_fans);
			glUniformMatrix4fv(mvp_loc_fans, 1, GL_FALSE, glm::value_ptr(mvp));
			glUniform1i(vid_offset_loc_fans, vid_offset);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			break;
		case 1:
			glUseProgram(program_linesadjacency);
			glUniformMatrix4fv(mvp_loc_linesadj, 1, GL_FALSE, glm::value_ptr(mvp));
			glUniform1i(vid_offset_loc_linesadj, vid_offset);
			glDrawArrays(GL_LINES_ADJACENCY, 0, 4);
			break;
		}
	}

	virtual void shutdown()
	{
		glDeleteProgram(program_linesadjacency);
		glDeleteProgram(program_fans);
		glDeleteVertexArrays(1, &vao);
	}

protected:
	void onKey(int key, int scancode, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			switch (key)
			{
			case '1':
			case '2':
				mode = key - '1';
				break;
			case GLFW_KEY_UP:
				vid_offset++;
				break;
			case GLFW_KEY_DOWN:
				vid_offset--;
				break;
			case 'P': paused = !paused;
				break;
			case 'R':
				load_shaders();
				break;
			case 'M':
				mode = (mode + 1) % 2;
				break;
			}
		}
		OpenGLApp::onKey(key, scancode, action, mods);
	}

private:
	void load_shaders();

protected:
	GLuint      program_fans;
	GLuint      program_linesadjacency;
	GLuint      vao;
	int         mode;
	int         mvp_loc_fans;
	int         mvp_loc_linesadj;
	int         vid_offset_loc_fans;
	int         vid_offset_loc_linesadj;
	int         vid_offset;
	bool        paused;
};

void gsquads_app::load_shaders()
{
	if (program_fans)
		glDeleteProgram(program_fans);

	program_fans = LoadShadersVF("../media/glsl/08_gsquads/quadsasfans.vs", "../media/glsl/08_gsquads/quadsasfans.fs");
	mvp_loc_fans = glGetUniformLocation(program_fans, "mvp");
	vid_offset_loc_fans = glGetUniformLocation(program_fans, "vid_offset");

	if (program_linesadjacency)
		glDeleteProgram(program_linesadjacency);

	program_linesadjacency = LoadShadersVGF("../media/glsl/08_gsquads/quadsaslinesadj.vs", "../media/glsl/08_gsquads/quadsaslinesadj.gs", "../media/glsl/08_gsquads/quadsaslinesadj.fs");
	mvp_loc_linesadj = glGetUniformLocation(program_linesadjacency, "mvp");
	vid_offset_loc_linesadj = glGetUniformLocation(program_fans, "vid_offset");
}

/** @} @} */

int main(int argc, char** argv)
{
	gsquads_app a;
	a.run();
	return 0;
}