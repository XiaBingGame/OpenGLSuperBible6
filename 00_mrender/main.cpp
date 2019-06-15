/************************************************************************/
// Ä£ÐÍ¼ÓÔØ
/************************************************************************/

#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class mrender_app : public OpenGLApp
{
public:
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Single Triangle";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		load_shaders();
		object.load("../media/models/ladybug.sbm");

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glGenTextures(1, &tex_color);
		glGenTextures(1, &tex_normal);
		glActiveTexture(GL_TEXTURE0);
		tex_color =loadKTX("../media/textures/ladybug_co.ktx");
		glActiveTexture(GL_TEXTURE1);
		tex_normal =loadKTX("../media/textures/ladybug_nm.ktx");
	}

	virtual void render(double currentTime)
	{
		static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
		static const GLfloat one = 1.0f;
		float f = (float)currentTime;

		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glClearBufferfv(GL_COLOR, 0, green);
		glClearBufferfv(GL_DEPTH, 0, &one);

		glUseProgram(program);

		glm::mat4 proj_matrix = glm::perspective(glm::radians(50.0f),
			(float)info.windowWidth / (float)info.windowHeight,
			0.1f,
			1000.0f);
		glUniformMatrix4fv(proj_location, 1, GL_FALSE, glm::value_ptr(proj_matrix));

		glm::mat4 mv_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, -7.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians((float)currentTime * 5.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::mat4(1.0f);
		glUniformMatrix4fv(mv_location, 1, GL_FALSE, glm::value_ptr(mv_matrix));

		object.render();
	}

	virtual void shutdown()
	{
		object.free();
		glDeleteProgram(program);
	}

private:
	void load_shaders();

protected:
	GLuint          program;
	GLint           mv_location;
	GLint           proj_location;

	glm::mat4         mat_rotation;

	GLuint              tex_color;
	GLuint              tex_normal;
	sb6::object         object;
	//sb6::utils::arcball arcball;
	bool                mouseDown;
};

void mrender_app::load_shaders()
{
	if (program != 0)
		glDeleteProgram(program);

	program = LoadShadersVF("../media/glsl/00_mrender/render.vs", "../media/glsl/00_mrender/render.fs");

	mv_location = glGetUniformLocation(program, "mv_matrix");
	proj_location = glGetUniformLocation(program, "proj_matrix");
}

int main(int argc, char** argv)
{
	mrender_app a;
	a.run();
	return 0;
}