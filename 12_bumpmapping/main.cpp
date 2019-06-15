#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class bumpmapping_app : public OpenGLApp
{
public:
	bumpmapping_app() : program(0), paused(false) {}
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Bump Mapping";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		load_shaders();

		glActiveTexture(GL_TEXTURE0);
		loadKTX("../media/textures/ladybug_co.ktx");
		textures.color = loadKTX("../media/textures/ladybug_co.ktx");
		glActiveTexture(GL_TEXTURE1);
		textures.normals = loadKTX("../media/textures/ladybug_nm.ktx");

		object.load("../media/models/ladybug.sbm");
	}

	virtual void render(double currentTime)
	{
		static const GLfloat zeros[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		static const GLfloat gray[] = { 0.1f, 0.1f, 0.1f, 0.0f };
		static const GLfloat ones[] = { 1.0f };
		static double last_time = 0.0;
		static double total_time = 0.0;

		if (!paused)
			total_time += (currentTime - last_time);
		last_time = currentTime;

		const float f = (float)total_time;

		glClearBufferfv(GL_COLOR, 0, gray);
		glClearBufferfv(GL_DEPTH, 0, ones);

		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glEnable(GL_DEPTH_TEST);

		glUseProgram(program);

		glm::mat4 proj_matrix = glm::perspective(glm::radians(50.0f),
			(float)info.windowWidth / (float)info.windowHeight,
			0.1f,
			1000.0f);
		glUniformMatrix4fv(uniforms.proj_matrix, 1, GL_FALSE, glm::value_ptr(proj_matrix));

		glm::mat4 mv_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.2f, -5.5f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(14.5f), glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(-20.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			//vmath::rotate(t * 14.5f, 0.0f, 1.0f, 0.0f) *
			//vmath::rotate(0.0f, 1.0f, 0.0f, 0.0f) *
			glm::mat4(1.0f);
		glUniformMatrix4fv(uniforms.mv_matrix, 1, GL_FALSE, glm::value_ptr(mv_matrix));

		glUniform3fv(uniforms.light_pos, 1, glm::value_ptr(glm::vec3(40.0f * sinf(f), 30.0f + 20.0f * cosf(f), 40.0f)));

		object.render();
	}

	virtual void shutdown()
	{
		glDeleteProgram(program);
	}

private:
	void load_shaders();

protected:
	GLuint          program;

	struct
	{
		GLuint      color;
		GLuint      normals;
	} textures;

	struct
	{
		GLint       mv_matrix;
		GLint       proj_matrix;
		GLint       light_pos;
	} uniforms;

	sb6::object     object;
	bool            paused;
};

void bumpmapping_app::load_shaders()
{
	if (program)
		glDeleteProgram(program);

	program = LoadShadersVF("../media/glsl/12_bumpmapping/bumpmapping.vs", "../media/glsl/12_bumpmapping/bumpmapping.fs");

	uniforms.mv_matrix = glGetUniformLocation(program, "mv_matrix");
	uniforms.proj_matrix = glGetUniformLocation(program, "proj_matrix");
	uniforms.light_pos = glGetUniformLocation(program, "light_pos");
}

int main(int argc, char** argv)
{
	bumpmapping_app a;
	a.run();
	return 0;
}