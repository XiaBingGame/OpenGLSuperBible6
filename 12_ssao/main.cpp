#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Random number generator
static unsigned int seed = 0x13371337;

static inline float random_float()
{
	float res;
	unsigned int tmp;

	seed *= 16807;

	tmp = seed ^ (seed >> 4) ^ (seed << 15);

	*((unsigned int *) &res) = (tmp >> 9) | 0x3F800000;

	return (res - 1.0f);
}

class ssao_app : public OpenGLApp
{
public:
	ssao_app() :
		  render_program(0),
		  ssao_program(0),
		  paused(false),
		  ssao_level(1.0f),
		  ssao_radius(0.05f),
		  show_shading(true),
		  show_ao(true),
		  weight_by_angle(true),
		  randomize_points(true),
		  point_count(10) {}

	void init()
	{
		static const char title[] = "OpenGL SuperBible - Screen-Space Ambient Occlusion";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup();

	virtual void render(double currentTime);

	virtual void shutdown()
	{
		glDeleteProgram(render_program);
		glDeleteProgram(ssao_program);
	}

private:
	void load_shaders();

protected:
	GLuint      render_program;
	GLuint      ssao_program;
	bool        paused;

	GLuint      render_fbo;
	GLuint      fbo_textures[3];
	GLuint      quad_vao;
	GLuint      points_buffer;

	sb6::object object;
	sb6::object cube;

	struct
	{
		struct
		{
			GLint           mv_matrix;
			GLint           proj_matrix;
			GLint           shading_level;
		} render;
		struct
		{
			GLint           ssao_level;
			GLint           object_level;
			GLint           ssao_radius;
			GLint           weight_by_angle;
			GLint           randomize_points;
			GLint           point_count;
		} ssao;
	} uniforms;

	bool  show_shading;
	bool  show_ao;
	float ssao_level;
	float ssao_radius;
	bool  weight_by_angle;
	bool randomize_points;
	unsigned int point_count;

	struct SAMPLE_POINTS
	{
		mvec4     point[256];
		mvec4     random_vectors[256];
	};
};

void ssao_app::startup()
{
	load_shaders();

	glGenFramebuffers(1, &render_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);
	glGenTextures(3, fbo_textures);

	glBindTexture(GL_TEXTURE_2D, fbo_textures[0]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB16F, 2048, 2048);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, fbo_textures[1]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, 2048, 2048);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, fbo_textures[2]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, 2048, 2048);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbo_textures[0], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, fbo_textures[1], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, fbo_textures[2], 0);

	static const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

	glDrawBuffers(2, draw_buffers);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenVertexArrays(1, &quad_vao);
	glBindVertexArray(quad_vao);

	object.load("../media/models/dragon.sbm");
	cube.load("../media/models/cube.sbm");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	int i;
	SAMPLE_POINTS point_data;
	glm::vec4 t;
	float l;
	for (i = 0; i < 256; i++)
	{
		do
		{
			t[0] = random_float() * 2.0f - 1.0f;
			t[1] = random_float() * 2.0f - 1.0f;
			t[2] = random_float(); //  * 2.0f - 1.0f;
			t[3] = 0.0f; 
		} while (glm::length(t) > 1.0f);
		t = glm::normalize(t);
		vec4copy(point_data.point[i], t);
	}
	for (i = 0; i < 256; i++)
	{
		t[0] = random_float();
		t[1] = random_float();
		t[2] = random_float();
		t[3] = random_float();
		vec4copy(point_data.random_vectors[i], t);
	}

	glGenBuffers(1, &points_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, points_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SAMPLE_POINTS), &point_data, GL_STATIC_DRAW);
}

void ssao_app::load_shaders()
{
	if (render_program)
		glDeleteProgram(render_program);

	if (ssao_program)
		glDeleteProgram(ssao_program);

	render_program = LoadShadersVF("../media/glsl/12_ssao/render.vs", "../media/glsl/12_ssao/render.fs");
	ssao_program = LoadShadersVF("../media/glsl/12_ssao/ssao.vs", "../media/glsl/12_ssao/ssao.fs");

	uniforms.render.mv_matrix = glGetUniformLocation(render_program, "mv_matrix");
	uniforms.render.proj_matrix = glGetUniformLocation(render_program, "proj_matrix");
	uniforms.render.shading_level = glGetUniformLocation(render_program, "shading_level");

	uniforms.ssao.ssao_radius = glGetUniformLocation(ssao_program, "ssao_radius");
	uniforms.ssao.ssao_level = glGetUniformLocation(ssao_program, "ssao_level");
	uniforms.ssao.object_level = glGetUniformLocation(ssao_program, "object_level");
	uniforms.ssao.weight_by_angle = glGetUniformLocation(ssao_program, "weight_by_angle");
	uniforms.ssao.randomize_points = glGetUniformLocation(ssao_program, "randomize_points");
	uniforms.ssao.point_count = glGetUniformLocation(ssao_program, "point_count");
}

void ssao_app::render(double currentTime)
{
	static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	static const GLfloat one = 1.0f;
	static double last_time = currentTime;
	static double total_time = 0.0;
	static const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

	if (!paused)
		total_time += (currentTime - last_time);
	last_time = currentTime;

	float f = (float)total_time;

	glViewport(0, 0, info.windowWidth, info.windowHeight);

	glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);
	glEnable(GL_DEPTH_TEST);

	glClearBufferfv(GL_COLOR, 0, black);
	glClearBufferfv(GL_COLOR, 1, black);
	glClearBufferfv(GL_DEPTH, 0, &one);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, points_buffer);

	glUseProgram(render_program);

	const glm::mat4 lookat_matrix = glm::lookAt(glm::vec3(0.0f, 3.0f, 15.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 proj_matrix = glm::perspective(glm::radians(50.0f),
		(float)info.windowWidth / (float)info.windowHeight,
		0.1f,
		1000.0f);
	glUniformMatrix4fv(uniforms.render.proj_matrix, 1, GL_FALSE, glm::value_ptr(proj_matrix));

	glm::mat4 mv_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -5.0f, 0.0f)) *
		glm::rotate(glm::mat4(1.0f), glm::radians(f * 5.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::mat4(1.0f);
	glUniformMatrix4fv(uniforms.render.mv_matrix, 1, GL_FALSE, glm::value_ptr(lookat_matrix * mv_matrix));

	glUniform1f(uniforms.render.shading_level, show_shading ? (show_ao ? 0.7f : 1.0f) : 0.0f);

	object.render();

	mv_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.5f, 0.0f)) *
		glm::rotate(glm::mat4(1.0f), glm::radians(f * 5.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(4000.0f, 0.1f, 4000.0f)) *
		glm::mat4(1.0f);
	glUniformMatrix4fv(uniforms.render.mv_matrix, 1, GL_FALSE, glm::value_ptr(lookat_matrix * mv_matrix));

	cube.render();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glUseProgram(ssao_program);

	glUniform1f(uniforms.ssao.ssao_radius, ssao_radius * float(info.windowWidth) / 1000.0f);
	glUniform1f(uniforms.ssao.ssao_level, show_ao ? (show_shading ? 0.3f : 1.0f) : 0.0f);
	// glUniform1i(uniforms.ssao.weight_by_angle, weight_by_angle ? 1 : 0);
	glUniform1i(uniforms.ssao.randomize_points, randomize_points ? 1 : 0);
	glUniform1ui(uniforms.ssao.point_count, point_count);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fbo_textures[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fbo_textures[1]);

	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(quad_vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

int main(int argc, char** argv)
{
	ssao_app a;
	a.run();
	return 0;
}