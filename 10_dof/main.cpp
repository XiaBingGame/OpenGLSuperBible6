#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define FBO_SIZE                2048
#define FRUSTUM_DEPTH           1000

class dof_app : public OpenGLApp
{
public:
	dof_app() :
		  display_program(0),
		  filter_program(0),
		  view_program(0),
		  paused(false),
		  focal_distance(40.0f),
		  focal_depth(50.0f)
	  {}
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Depth of Field";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup();

	virtual void render(double currentTime);

	virtual void shutdown()
	{
		glDeleteVertexArrays(1, &quad_vao);
		glDeleteProgram(view_program);
		glDeleteProgram(filter_program);
		glDeleteProgram(display_program);
		glDeleteBuffers(1, &depth_fbo);
		glDeleteTextures(1, &depth_tex);
		glDeleteTextures(1, &color_tex);
		glDeleteTextures(1, &temp_tex);
	}

	void onKey(int key, int scancode, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			switch(key)
			{
			case 'Q':
				focal_distance *= 1.1f;
				break;
			case'A':
				focal_distance /= 1.1f;
				break;
			case 'W':
				focal_depth *= 1.1f;
				break;
			case 'S':
				focal_depth /= 1.1f;
				break;
			case 'R':
				load_shaders();
				break;
			case 'P':
				paused = !paused;
				break;
			default:
				break;
			}
		}
		OpenGLApp::onKey(key, scancode, action, mods);
	}

private:
	void load_shaders();
	void render_scene(double currenttime);

protected:
	GLuint          view_program;
	GLuint          filter_program;
	GLuint          display_program;

	struct
	{
		struct
		{
			GLint   focal_distance;
			GLint   focal_depth;
		} dof;
		struct
		{
			GLint   mv_matrix;
			GLint   proj_matrix;
			GLint   full_shading;
			GLint   diffuse_albedo;
		} view;
	} uniforms;

	GLuint          depth_fbo;
	GLuint          depth_tex;
	GLuint          color_tex;
	GLuint          temp_tex;

	enum { OBJECT_COUNT = 5 };
	struct
	{
		sb6::object     obj;
		glm::mat4     model_matrix;
		glm::vec4     diffuse_albedo;
	} objects[OBJECT_COUNT];

	glm::mat4     camera_view_matrix;
	glm::mat4     camera_proj_matrix;

	GLuint          quad_vao;

	bool paused;

	float          focal_distance;
	float          focal_depth;
};

void dof_app::startup()
{
	load_shaders();

	int i;

	static const char * const object_names[] =
	{
		"../media/models/dragon.sbm",
		"../media/models/sphere.sbm",
		"../media/models/cube.sbm",
		"../media/models/cube.sbm",
		"../media/models/cube.sbm",
	};

	static const glm::vec4 object_colors[] =
	{
		glm::vec4(1.0f, 0.7f, 0.8f, 1.0f),
		glm::vec4(0.7f, 0.8f, 1.0f, 1.0f),
		glm::vec4(0.3f, 0.9f, 0.4f, 1.0f),
		glm::vec4(0.6f, 0.4f, 0.9f, 1.0f),
		glm::vec4(0.8f, 0.2f, 0.1f, 1.0f),
	};

	for (i = 0; i < OBJECT_COUNT; i++)
	{
		objects[i].obj.load(object_names[i]);
		objects[i].diffuse_albedo = object_colors[i];
	}

	glGenFramebuffers(1, &depth_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);

	glGenTextures(1, &depth_tex);
	glBindTexture(GL_TEXTURE_2D, depth_tex);
	glTexStorage2D(GL_TEXTURE_2D, 11, GL_DEPTH_COMPONENT32F, FBO_SIZE, FBO_SIZE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &color_tex);
	glBindTexture(GL_TEXTURE_2D, color_tex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, FBO_SIZE, FBO_SIZE);

	glGenTextures(1, &temp_tex);
	glBindTexture(GL_TEXTURE_2D, temp_tex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, FBO_SIZE, FBO_SIZE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_tex, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_tex, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glEnable(GL_DEPTH_TEST);

	glGenVertexArrays(1, &quad_vao);
	glBindVertexArray(quad_vao);
}

void dof_app::load_shaders()
{
	if (view_program)
		glDeleteProgram(view_program);

	view_program = LoadShadersVF("../media/glsl/10_dof/render.vs", "../media/glsl/10_dof/render.fs");
	uniforms.view.proj_matrix = glGetUniformLocation(view_program, "proj_matrix");
	uniforms.view.mv_matrix = glGetUniformLocation(view_program, "mv_matrix");
	uniforms.view.full_shading = glGetUniformLocation(view_program, "full_shading");
	uniforms.view.diffuse_albedo = glGetUniformLocation(view_program, "diffuse_albedo");

	if (display_program)
		glDeleteProgram(display_program);

	display_program = LoadShadersVF("../media/glsl/10_dof/display.vs", "../media/glsl/10_dof/display.fs");
	uniforms.dof.focal_distance = glGetUniformLocation(display_program, "focal_distance");
	uniforms.dof.focal_depth = glGetUniformLocation(display_program, "focal_depth");

	if (filter_program)
		glDeleteProgram(filter_program);

	filter_program = LoadShadersCS("../media/glsl/10_dof/gensat.cs");
}

void dof_app::render(double currentTime)
{
	static const GLfloat zeros[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	static double last_time = 0.0;
	static double total_time = 0.0;

	if (!paused)
		total_time += (currentTime - last_time);
	last_time = currentTime;

	const float f = (float)total_time + 30.0f;

	glm::vec3 view_position = glm::vec3(0.0f, 0.0f, 40.0f);

	camera_proj_matrix = glm::perspective(glm::radians(50.0f),
		(float)info.windowWidth / (float)info.windowHeight,
		2.0f,
		300.0f);

	camera_view_matrix = glm::lookAt(view_position,
		glm::vec3(0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	objects[0].model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 20.0f)) *
		glm::rotate(glm::mat4(1.0f), glm::radians(f * 14.5f), glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::rotate(glm::mat4(1.0f), glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.0f, 0.0f));

	objects[1].model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, 0.0f, 0.0f)) *
		glm::rotate(glm::mat4(1.0f), glm::radians(f * 14.5f), glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::rotate(glm::mat4(1.0f), glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.0f, 0.0f));

	objects[2].model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(-15.0f, 0.0f, -20.0f)) *
		glm::rotate(glm::mat4(1.0f), glm::radians(f * 14.5f), glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::rotate(glm::mat4(1.0f), glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.0f, 0.0f));

	objects[3].model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(-25.0f, 0.0f, -40.0f)) *
		glm::rotate(glm::mat4(1.0f), glm::radians(f * 14.5f), glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::rotate(glm::mat4(1.0f), glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.0f, 0.0f));

	objects[4].model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(-35.0f, 0.0f, -60.0f)) *
		glm::rotate(glm::mat4(1.0f), glm::radians(f * 14.5f), glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::rotate(glm::mat4(1.0f), glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.0f, 0.0f));

	glEnable(GL_DEPTH_TEST);
	render_scene(total_time);

	glUseProgram(filter_program);

	glBindImageTexture(0, color_tex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(1, temp_tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	glDispatchCompute(info.windowHeight, 1, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	glBindImageTexture(0, temp_tex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(1, color_tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	glDispatchCompute(info.windowWidth, 1, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	glBindTexture(GL_TEXTURE_2D, color_tex);
	glDisable(GL_DEPTH_TEST);
	glUseProgram(display_program);
	glUniform1f(uniforms.dof.focal_distance, focal_distance);
		glUniform1f(uniforms.dof.focal_depth, focal_depth);
	glBindVertexArray(quad_vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void dof_app::render_scene(double currenttime)
{
	static const GLfloat ones[] = { 1.0f };
	static const GLfloat zero[] = { 0.0f };
	static const GLfloat gray[] = { 0.1f, 0.1f, 0.1f, 0.0f };
	static const GLenum attachments[] = { GL_COLOR_ATTACHMENT0 };
	static const glm::mat4 scale_bias_matrix = glm::mat4(glm::vec4(0.5f, 0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, 0.5f, 0.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, 0.5f, 0.0f),
		glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));

	glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);

	glDrawBuffers(1, attachments);
	glViewport(0, 0, info.windowWidth, info.windowHeight);
	glClearBufferfv(GL_COLOR, 0, gray);
	glClearBufferfv(GL_DEPTH, 0, ones);
	glUseProgram(view_program);
	glUniformMatrix4fv(uniforms.view.proj_matrix, 1, GL_FALSE, glm::value_ptr(camera_proj_matrix));

	glClearBufferfv(GL_DEPTH, 0, ones);

	int i;
	for (i = 0; i < OBJECT_COUNT; i++)
	{
		glm::mat4& model_matrix = objects[i].model_matrix;
		glUniformMatrix4fv(uniforms.view.mv_matrix, 1, GL_FALSE, glm::value_ptr(camera_view_matrix * objects[i].model_matrix));
		glUniform3fv(uniforms.view.diffuse_albedo, 1, glm::value_ptr(objects[i].diffuse_albedo));
		objects[0].obj.render();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main(int argc, char** argv)
{
	dof_app a;
	a.run();
	return 0;
}