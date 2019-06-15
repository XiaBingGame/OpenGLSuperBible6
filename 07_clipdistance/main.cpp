#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/** \addtogroup Chapter07*   @{*       \addtogroup clipdistance* 实现自定义裁剪平面.**	- 通过 GL_CLIP_DISTANCE0 允许裁剪平面*	- 着色器中通过 gl_ClipDistance 设置裁剪距离*       @{*/

class clipdistance_app : public OpenGLApp
{
public:
	clipdistance_app()
		: program(0), paused(false) {}

	void init()
	{
		static const char title[] = "OpenGL SuperBible - Clip Distance";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		object.load("../media/models/dragon.sbm");
		load_shaders();
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

	virtual void render(double currentTime);

	virtual void shutdown()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteProgram(program);
	}

private:
	void load_shaders();

protected:
	GLuint          program;
	GLuint          vao;
	bool				paused;
	sb6::object		object;

	struct
	{
		GLuint proj_matrix;
		GLuint mv_matrix;
		GLuint clip_plane;
		GLuint clip_sphere;
	} uniforms;
};

void clipdistance_app::load_shaders()
{
	if(program)
		glDeleteProgram(program);

	program = LoadShadersVF("../media/glsl/07_clipdistance.vs", "../media/glsl/07_clipdistance.fs");

	uniforms.proj_matrix = glGetUniformLocation(program, "proj_matrix");
	uniforms.mv_matrix = glGetUniformLocation(program, "mv_matrix");
	uniforms.clip_plane = glGetUniformLocation(program, "clip_plane");
	uniforms.clip_sphere = glGetUniformLocation(program, "clip_sphere");
}

void clipdistance_app::render(double currentTime)
{
	static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	static const GLfloat one = 1.0f;
	glClearBufferfv(GL_COLOR, 0, black);
	glClearBufferfv(GL_DEPTH, 0, &one);

	static double last_time = currentTime;
	static double total_time = 0.0;
	
	if(!paused)
		total_time += (currentTime - last_time);
	last_time = currentTime;

	float f = (float)total_time;
	glm::mat4 proj_matrix = glm::perspective(glm::radians(50.0f), (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);
	glm::mat4 mv_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -15.0f)) *
						glm::rotate(glm::mat4(1.0f), glm::radians(f*0.34f), glm::vec3(0.0f, 1.0f, 0.0f)) *
						glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.0f, 0.0f));
	glm::mat4 plane_matrix = glm::rotate(glm::mat4(1.0f), glm::radians(f*6.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
							glm::rotate(glm::mat4(1.0f), glm::radians(f*7.3f), glm::vec3(0.0f, 1.0f, 0.0f));

	// 得到裁剪的平面方程
	glm::vec4 plane = plane_matrix[0];
	plane[3] = 0.0f;
	plane = glm::normalize(plane);

	glm::vec4 clip_sphere = glm::vec4(sinf(f*0.7f)*3.0f, cosf(f*1.9f)*3.0f, sinf(f*0.1f)*3.0f, cosf(f*1.7f)+2.5f);

	// 通过 GL_CLIP_DISTANCE0 允许裁剪平面
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CLIP_DISTANCE0);
	glEnable(GL_CLIP_DISTANCE1);

	glUseProgram(program);
	glUniformMatrix4fv(uniforms.proj_matrix, 1, false, &proj_matrix[0][0]);
	glUniformMatrix4fv(uniforms.mv_matrix, 1, false, &mv_matrix[0][0]);
	glUniform4fv(uniforms.clip_plane, 1, &plane[0]);
	glUniform4fv(uniforms.clip_sphere, 1, &clip_sphere[0]);

	object.render();
}

/** @} @} */

int main(int argc, char** argv)
{
	clipdistance_app a;
	a.run();
	return 0;
}