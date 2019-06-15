#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class cubemapenv_app : public OpenGLApp
{
public:
	cubemapenv_app() :
		  envmap_index(0),
		  render_prog(0),
		  skybox_prog(0)
	{

	}
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Cubic Environment Map";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{	
		envmaps[0] = loadKTX("../media/textures/envmaps/mountaincube.ktx");
		tex_envmap = envmaps[envmap_index];

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		object.load("../media/models/dragon.sbm");

		load_shaders();

		glGenVertexArrays(1, &skybox_vao);
		glBindVertexArray(skybox_vao);

		glDepthFunc(GL_LEQUAL);
	}

	virtual void render(double currentTime)
	{
		static const GLfloat gray[] = { 0.2f, 0.2f, 0.2f, 1.0f };
		static const GLfloat ones[] = { 1.0f };
		const float t = (float)currentTime * 0.1f;

		glm::mat4 proj_matrix = glm::perspective(glm::radians(60.0f), (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);
		glm::mat4 view_matrix = glm::lookAt(glm::vec3(15.0f * sinf(t), 0.0f, 15.0f * cosf(t)),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 mv_matrix = view_matrix *
			glm::rotate(glm::mat4(1.0f), glm::radians(t), glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(t * 130.1f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.0f, 0.0f));

		glClearBufferfv(GL_COLOR, 0, gray);
		glClearBufferfv(GL_DEPTH, 0, ones);
		glBindTexture(GL_TEXTURE_CUBE_MAP, tex_envmap);

		glViewport(0, 0, info.windowWidth, info.windowHeight);

		glUseProgram(skybox_prog);
		glBindVertexArray(skybox_vao);

		glUniformMatrix4fv(uniforms.skybox.view_matrix, 1, GL_FALSE, glm::value_ptr(view_matrix));

		glDisable(GL_DEPTH_TEST);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glUseProgram(render_prog);

		glUniformMatrix4fv(uniforms.render.mv_matrix, 1, GL_FALSE, glm::value_ptr(mv_matrix));
		glUniformMatrix4fv(uniforms.render.proj_matrix, 1, GL_FALSE, glm::value_ptr(proj_matrix));

		glEnable(GL_DEPTH_TEST);

		object.render();
	}

	virtual void shutdown()
	{
		glDeleteProgram(render_prog);
		glDeleteProgram(skybox_prog);
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
				//envmap_index = (envmap_index + 1) % 3;
				//tex_envmap = envmaps[envmap_index];
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
	GLuint          skybox_prog;

	GLuint          tex_envmap;
	GLuint          envmaps[3];
	int             envmap_index;

	struct
	{
		struct
		{
			GLint       mv_matrix;
			GLint       proj_matrix;
		} render;
		struct
		{
			GLint       view_matrix;
		} skybox;
	} uniforms;

	sb6::object     object;

	GLuint          skybox_vao;
};

void cubemapenv_app::load_shaders()
{
	if (render_prog)
		glDeleteProgram(render_prog);
	render_prog = LoadShadersVF("../media/glsl/12_cubemapenv/render.vs", "../media/glsl/12_cubemapenv/render.fs");
	uniforms.render.mv_matrix = glGetUniformLocation(render_prog, "mv_matrix");
	uniforms.render.proj_matrix = glGetUniformLocation(render_prog, "proj_matrix");

	if (skybox_prog)
		glDeleteProgram(skybox_prog);
	skybox_prog = LoadShadersVF("../media/glsl/12_cubemapenv/skybox.vs", "../media/glsl/12_cubemapenv/skybox.fs");
	uniforms.skybox.view_matrix = glGetUniformLocation(skybox_prog, "view_matrix");
}

int main(int argc, char** argv)
{
	cubemapenv_app a;
	a.run();
	return 0;
}