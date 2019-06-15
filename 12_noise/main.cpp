#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//  程序似乎有点问题, 找不到一致变量 time

class noise_app : public OpenGLApp
{
public:
	noise_app() :
	  prog_noise(0),
		  paused(0)
	  {

	  }
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Noise";
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

		static double last_time = currentTime;
		static double total_time = 0.0;

		if (!paused)
			total_time += (currentTime - last_time);
		last_time = currentTime;

		float t = (float)total_time;

		glClearBufferfv(GL_COLOR, 0, black);

		glUseProgram(prog_noise);
		glUniform1f(loc_time, t * 0.0002);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
	}

	virtual void shutdown()
	{
		glDeleteProgram(prog_noise);
		glDeleteVertexArrays(1, &vao);
	}
	void onKey(int key, int scancode, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			switch(key)
			{
			case 'P': paused = !paused;
				break;
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
	void load_shaders()
	{

		if (prog_noise)
			glDeleteProgram(prog_noise);

		prog_noise = LoadShadersVF("../media/glsl/12_noise/noise.vs", "../media/glsl/12_noise/noise.fs");
		loc_time = glGetUniformLocation(prog_noise, "time");
	}

protected:
	GLuint      prog_noise;
	GLuint      vao;
	GLuint         loc_time;
	bool        paused;
};

int main(int argc, char** argv)
{
	noise_app a;
	a.run();
	return 0;
}