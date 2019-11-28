#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class envmapsphere_app : public OpenGLApp
{
public:
	envmapsphere_app() : envmap_index(0) {}
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Spherical Environment Map";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		envmaps[0] = loadKTX("../media/textures/envmaps/spheremap1.ktx");
		envmaps[1] = loadKTX("../media/textures/envmaps/spheremap2.ktx");
		envmaps[2] = loadKTX("../media/textures/envmaps/spheremap3.ktx");
		
		tex_envmap = envmaps[envmap_index];

		object.load("../media/models/dragon.sbm");
		load_shaders();
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	}

	virtual void render(double currentTime)
	{
		static const GLfloat gray[] = { 0.2f, 0.2f, 0.2f, 1.0f };
		static const GLfloat ones[] = { 1.0f };

		glClearBufferfv(GL_COLOR, 0, gray);
		glClearBufferfv(GL_DEPTH, 0, ones);
		glBindTexture(GL_TEXTURE_2D, tex_envmap);

		glViewport(0, 0, info.windowWidth, info.windowHeight);

		glUseProgram(render_prog);

		glm::mat4 proj_matrix = glm::perspective(glm::radians(60.0f), (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);
		glm::mat4 mv_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -15.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians((float)currentTime), glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians((float)currentTime * 1.1f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.0f, 0.0f));

		glUniformMatrix4fv(uniforms.mv_matrix, 1, GL_FALSE, glm::value_ptr(mv_matrix));
		glUniformMatrix4fv(uniforms.proj_matrix, 1, GL_FALSE, glm::value_ptr(proj_matrix));

		object.render();
	}

	virtual void shutdown()
	{
		glDeleteProgram(render_prog);
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
			case 'E':
				envmap_index = (envmap_index + 1) % 3;
				tex_envmap = envmaps[envmap_index];
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
	GLuint          render_prog;

	GLuint          tex_envmap;
	GLuint          envmaps[3];
	int             envmap_index;

	struct
	{
		GLint       mv_matrix;
		GLint       proj_matrix;
	} uniforms;

	sb6::object     object;
};

void envmapsphere_app::load_shaders()
{
	if (render_prog)
		glDeleteProgram(render_prog);

	render_prog = LoadShadersVF("../media/glsl/12_envmapsphere/render.vs", "../media/glsl/12_envmapsphere/render.fs");
	uniforms.mv_matrix = glGetUniformLocation(render_prog, "mv_matrix");
	uniforms.proj_matrix = glGetUniformLocation(render_prog, "proj_matrix");
}

int main(int argc, char** argv)
{
	envmapsphere_app a;
	a.run();
	return 0;
}