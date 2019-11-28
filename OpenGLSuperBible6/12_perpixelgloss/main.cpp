#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class perpixelgloss_app : public OpenGLApp
{
public:
	perpixelgloss_app() : render_prog(0) {}
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Per-Pixel Gloss";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		glActiveTexture(GL_TEXTURE0);
		tex_envmap = loadKTX("../media/textures/envmaps/mountains3d.ktx");
		
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glActiveTexture(GL_TEXTURE1);
		tex_glossmap = loadKTX("../media/textures/pattern1.ktx");

		object.load("../media/models/torus_nrms_tc.sbm");

		load_shaders();

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	}

	virtual void render(double currentTime)
	{
		static const GLfloat gray[] = { 0.1f, 0.1f, 0.1f, 1.0f };
		static const GLfloat ones[] = { 1.0f };

		glClearBufferfv(GL_COLOR, 0, gray);
		glClearBufferfv(GL_DEPTH, 0, ones);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, tex_envmap);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, tex_glossmap);

		glViewport(0, 0, info.windowWidth, info.windowHeight);

		glUseProgram(render_prog);

		glm::mat4 proj_matrix = glm::perspective(glm::radians(60.0f), (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);
		glm::mat4 mv_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians((float)currentTime * 13.75f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians((float)currentTime * 7.75f), glm::vec3(0.0f, 0.0f, 1.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians((float)currentTime * 15.3f), glm::vec3(1.0f, 0.0f, 0.0f));

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
	GLuint          tex_glossmap;

	struct
	{
		GLint       mv_matrix;
		GLint       proj_matrix;
	} uniforms;

	sb6::object     object;
};

void perpixelgloss_app::load_shaders()
{
	if (render_prog)
		glDeleteProgram(render_prog);

	render_prog = LoadShadersVF("../media/glsl/12_perpixelgloss/perpixelgloss.vs", "../media/glsl/12_perpixelgloss/perpixelgloss.fs");
	uniforms.mv_matrix = glGetUniformLocation(render_prog, "mv_matrix");
	uniforms.proj_matrix = glGetUniformLocation(render_prog, "proj_matrix");
}

int main(int argc, char** argv)
{
	perpixelgloss_app a;
	a.run();
	return 0;
}