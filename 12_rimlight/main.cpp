#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class rimlight_app : public OpenGLApp
{
public:
	rimlight_app() :
		  program(0),
		  paused(false),
		  rim_color(0.3f, 0.3f, 0.3f),
		  rim_power(2.5f),
		  rim_enable(true)
	  {
		  mat_rotation = glm::mat4(1.0f);
	  }
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Rim Lighting";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		load_shaders();

		object.load("../media/models/dragon.sbm");

		glEnable(GL_CULL_FACE);
		//glCullFace(GL_FRONT);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	}

	virtual void render(double currentTime)
	{
		static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
		static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		static const GLfloat one = 1.0f;
		static double last_time = 0.0;
		static double total_time = 0.0;

		if (!paused)
			total_time += (currentTime - last_time);
		last_time = currentTime;

		float f = (float)total_time;

		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glClearBufferfv(GL_COLOR, 0, black);
		glClearBufferfv(GL_DEPTH, 0, &one);

		glUseProgram(program);

		glm::mat4 proj_matrix = glm::perspective(glm::radians(50.0f),
			(float)info.windowWidth / (float)info.windowHeight,
			0.1f,
			1000.0f);
		glUniformMatrix4fv(uniforms.proj_matrix, 1, GL_FALSE, glm::value_ptr(proj_matrix));

		glm::mat4 mv_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -5.0f, -20.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(f * 5.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::mat4(1.0f);
		glUniformMatrix4fv(uniforms.mv_matrix, 1, GL_FALSE, glm::value_ptr(mv_matrix));

		glUniform3fv(uniforms.rim_color, 1, rim_enable ? glm::value_ptr(rim_color) : glm::value_ptr(glm::vec3(0.0f)));
		glUniform1f(uniforms.rim_power, rim_power);

		object.render();
	}

	virtual void shutdown()
	{
		glDeleteProgram(program);
	}

	void onKey(int key, int scancode, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			switch(key)
			{
			case 'Q':
				rim_color[0] += 0.1f;
				break;
			case 'W':
				rim_color[1] += 0.1f;
				break;
			case 'E':
				rim_color[2] += 0.1f;
				break;
			case 'R':
				rim_power *= 1.5f;
				break;
			case 'A':
				rim_color[0] -= 0.1f;
				break;
			case 'S':
				rim_color[1] -= 0.1f;
				break;
			case 'D':
				rim_color[2] -= 0.1f;
				break;
			case 'F':
				rim_power /= 1.5f;
				break;
			case 'Z':
				rim_enable = !rim_enable;
				break;

			case 'P':
				paused = !paused;
				break;
			case 'L':
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
	GLuint          program;
	struct
	{
		GLint           mv_matrix;
		GLint           proj_matrix;
		GLint           rim_color;
		GLint           rim_power;
	} uniforms;

	glm::mat4         mat_rotation;

	sb6::object         object;
	bool                paused;
	glm::vec3         rim_color;
	float               rim_power;
	bool                rim_enable;
};

void rimlight_app::load_shaders()
{
	if (program != 0)
		glDeleteProgram(program);

	program = LoadShadersVF("../media/glsl/12_rimlight/render.vs", "../media/glsl/12_rimlight/render.fs");

	uniforms.mv_matrix = glGetUniformLocation(program, "mv_matrix");
	uniforms.proj_matrix = glGetUniformLocation(program, "proj_matrix");
	uniforms.rim_color = glGetUniformLocation(program, "rim_color");
	uniforms.rim_power = glGetUniformLocation(program, "rim_power");
}

int main(int argc, char** argv)
{
	rimlight_app a;
	a.run();
	return 0;
}