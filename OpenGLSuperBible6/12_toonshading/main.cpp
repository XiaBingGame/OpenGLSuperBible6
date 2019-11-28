#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class toonshading_app : public OpenGLApp
{
public:
	toonshading_app() : render_prog(0) {}
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Toon Shading";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		static const GLubyte toon_tex_data[] =
		{
			0x44, 0x00, 0x00, 0x00,
			0x88, 0x00, 0x00, 0x00,
			0xCC, 0x00, 0x00, 0x00,
			0xFF, 0x00, 0x00, 0x00
		};

		glGenTextures(1, &tex_toon);
		glBindTexture(GL_TEXTURE_1D, tex_toon);
		glTexStorage1D(GL_TEXTURE_1D, 1, GL_RGB8, sizeof(toon_tex_data) / 4);
		glTexSubImage1D(GL_TEXTURE_1D, 0,
			0, sizeof(toon_tex_data) / 4,
			GL_RGBA, GL_UNSIGNED_BYTE,
			toon_tex_data);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

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

		glBindTexture(GL_TEXTURE_1D, tex_toon);

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
		glDeleteTextures(1, &tex_toon);
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

	void load_shaders();

protected:
	GLuint          render_prog;

	GLuint          tex_toon;

	struct
	{
		GLint       mv_matrix;
		GLint       proj_matrix;
	} uniforms;

	sb6::object     object;
};

void toonshading_app::load_shaders()
{
	if (render_prog)
		glDeleteProgram(render_prog);

	render_prog = LoadShadersVF("../media/glsl/12_toonshading/toonshading.vs", "../media/glsl/12_toonshading/toonshading.fs");
	uniforms.mv_matrix = glGetUniformLocation(render_prog, "mv_matrix");
	uniforms.proj_matrix = glGetUniformLocation(render_prog, "proj_matrix");
}

int main(int argc, char** argv)
{
	toonshading_app a;
	a.run();
	return 0;
}