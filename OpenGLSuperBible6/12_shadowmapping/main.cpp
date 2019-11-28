#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define DEPTH_TEXTURE_SIZE      4096
#define FRUSTUM_DEPTH           1000

class shadowmapping_app : public OpenGLApp
{
public:
	shadowmapping_app() : 
		  light_program(0),
		  view_program(0),
		  show_light_depth_program(0),
		  mode(RENDER_FULL),
		  paused(false)
	  {}

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

		int i;

		static const char * const object_names[] =
		{
			"../media/models/dragon.sbm",
			"../media/models/sphere.sbm",
			"../media/models/cube.sbm",
			"../media/models/torus.sbm"
		};

		for (i = 0; i < OBJECT_COUNT; i++)
		{
			objects[i].obj.load(object_names[i]);
		}

		glGenFramebuffers(1, &depth_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);

		glGenTextures(1, &depth_tex);
		glBindTexture(GL_TEXTURE_2D, depth_tex);
		glTexStorage2D(GL_TEXTURE_2D, 11, GL_DEPTH_COMPONENT32F, DEPTH_TEXTURE_SIZE, DEPTH_TEXTURE_SIZE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_tex, 0);

		glGenTextures(1, &depth_debug_tex);
		glBindTexture(GL_TEXTURE_2D, depth_debug_tex);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, DEPTH_TEXTURE_SIZE, DEPTH_TEXTURE_SIZE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, depth_debug_tex, 0);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glEnable(GL_DEPTH_TEST);

		glGenVertexArrays(1, &quad_vao);
		glBindVertexArray(quad_vao);
	}

	virtual void render(double currentTime)
	{
		static const GLfloat zeros[] = { 0.0f, 0.0f, 0.0f, 0.0f };

		static double last_time = currentTime;
		static double total_time = 0.0;

		if (!paused)
			total_time += (currentTime - last_time);
		last_time = currentTime;

		const float f = (float)total_time + 30.0f;

		glm::vec3 light_position = glm::vec3(20.0f, 20.0f, 20.0f);
		glm::vec3 view_position = glm::vec3(0.0f, 0.0f, 40.0f);

		light_proj_matrix = glm::frustum(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 200.0f);
		light_view_matrix = glm::lookAt(light_position,
			glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		camera_proj_matrix = glm::perspective(glm::radians(50.0f),
			(float)info.windowWidth / (float)info.windowHeight,
			1.0f,
			200.0f);

		camera_view_matrix = glm::lookAt(view_position,
			glm::vec3(0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));

		objects[0].model_matrix = glm::rotate(glm::mat4(1.0f), glm::radians(f * 14.5f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.0f, 0.0f));

		objects[1].model_matrix = glm::rotate(glm::mat4(1.0f), glm::radians(f * 3.7f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::translate(glm::mat4(1.0f), glm::vec3(sinf(f * 0.37f) * 12.0f, cosf(f * 0.37f) * 12.0f, 0.0f)) *
			glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));

		objects[2].model_matrix = glm::rotate(glm::mat4(1.0f), glm::radians(f * 6.45f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::translate(glm::mat4(1.0f), glm::vec3(sinf(f * 0.25f) * 10.0f, cosf(f * 0.25f) * 10.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(f * 99.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
			glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));

		objects[3].model_matrix = glm::rotate(glm::mat4(1.0f), glm::radians(f * 5.25f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::translate(glm::mat4(1.0f), glm::vec3(sinf(f * 0.51f) * 14.0f, cosf(f * 0.51f) * 14.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(f * 120.3f), glm::vec3(0.707106f, 0.0f, 0.707106f)) *
			glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));

		glEnable(GL_DEPTH_TEST);
		render_scene(total_time, true);

		if (mode == RENDER_DEPTH)
		{
			glDisable(GL_DEPTH_TEST);
			glBindVertexArray(quad_vao);
			glUseProgram(show_light_depth_program);
			glBindTexture(GL_TEXTURE_2D, depth_debug_tex);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
		else
		{
			render_scene(total_time, false);
		}
	}

	virtual void shutdown()
	{
		glDeleteProgram(light_program);
		glDeleteProgram(view_program);
		glDeleteProgram(show_light_depth_program);
	}

	void onKey(int key, int scancode, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			switch(key)
			{
			case '1':
				mode = RENDER_FULL;
				break;
			case '2':
				mode = RENDER_LIGHT;
				break;
			case '3':
				mode = RENDER_DEPTH;
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
	void render_scene(double currentTime, bool from_light);

protected:
	GLuint          light_program;
	GLuint          view_program;
	GLint           show_light_depth_program;

	struct
	{
		struct
		{
			GLint   mvp;
		} light;
		struct
		{
			GLint   mv_matrix;
			GLint   proj_matrix;
			GLint   shadow_matrix;
			GLint   full_shading;
		} view;
	} uniforms;

	GLuint          depth_fbo;
	GLuint          depth_tex;
	GLuint          depth_debug_tex;

	enum { OBJECT_COUNT = 4 };
	struct
	{
		sb6::object     obj;
		glm::mat4     model_matrix;
	} objects[OBJECT_COUNT];

	glm::mat4     light_view_matrix;
	glm::mat4     light_proj_matrix;

	glm::mat4     camera_view_matrix;
	glm::mat4     camera_proj_matrix;

	GLuint          quad_vao;

	enum
	{
		RENDER_FULL,
		RENDER_LIGHT,
		RENDER_DEPTH
	} mode;

	bool paused;
};

void shadowmapping_app::load_shaders()
{
	if (light_program)
		glDeleteProgram(light_program);

	if (view_program)
		glDeleteProgram(view_program);

	if (show_light_depth_program)
		glDeleteProgram(show_light_depth_program);

	light_program = LoadShadersVF("../media/glsl/12_shadowmapping/shadowmapping-light.vs", "../media/glsl/12_shadowmapping/shadowmapping-light.fs");
	view_program = LoadShadersVF("../media/glsl/12_shadowmapping/shadowmapping-camera.vs", "../media/glsl/12_shadowmapping/shadowmapping-camera.fs");
	show_light_depth_program = LoadShadersVF("../media/glsl/12_shadowmapping/shadowmapping-light-view.vs", "../media/glsl/12_shadowmapping/shadowmapping-light-view.fs");

	uniforms.light.mvp = glGetUniformLocation(light_program, "mvp");
	uniforms.view.proj_matrix = glGetUniformLocation(view_program, "proj_matrix");
	uniforms.view.mv_matrix = glGetUniformLocation(view_program, "mv_matrix");
	uniforms.view.shadow_matrix = glGetUniformLocation(view_program, "shadow_matrix");
	uniforms.view.full_shading = glGetUniformLocation(view_program, "full_shading");
}

void shadowmapping_app::render_scene(double currentTime, bool from_light)
{
	static const GLfloat ones[] = { 1.0f };
	static const GLfloat zero[] = { 0.0f };
	static const GLfloat gray[] = { 0.1f, 0.1f, 0.1f, 0.0f };
	static const glm::mat4 scale_bias_matrix = glm::mat4(glm::vec4(0.5f, 0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, 0.5f, 0.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, 0.5f, 0.0f),
		glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
	glm::mat4 light_vp_matrix = light_proj_matrix * light_view_matrix;
	glm::mat4 shadow_sbpv_matrix = scale_bias_matrix * light_proj_matrix * light_view_matrix;

	if (from_light)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
		glViewport(0, 0, DEPTH_TEXTURE_SIZE, DEPTH_TEXTURE_SIZE);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(4.0f, 4.0f);
		glUseProgram(light_program);
		static const GLenum buffs[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, buffs);
		glClearBufferfv(GL_COLOR, 0, zero);
	}
	else
	{
		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glClearBufferfv(GL_COLOR, 0, gray);
		glUseProgram(view_program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depth_tex);
		glUniformMatrix4fv(uniforms.view.proj_matrix, 1, GL_FALSE, glm::value_ptr(camera_proj_matrix));
		glDrawBuffer(GL_BACK);
	}

	glClearBufferfv(GL_DEPTH, 0, ones);

	int i;
	for (i = 0; i < 4; i++)
	{
		glm::mat4& model_matrix = objects[i].model_matrix;
		if (from_light)
		{
			glUniformMatrix4fv(uniforms.light.mvp, 1, GL_FALSE, glm::value_ptr(light_vp_matrix * objects[i].model_matrix));
		}
		else
		{
			glm::mat4 shadow_matrix = shadow_sbpv_matrix * model_matrix;
			glUniformMatrix4fv(uniforms.view.shadow_matrix, 1, GL_FALSE, glm::value_ptr(shadow_matrix));
			glUniformMatrix4fv(uniforms.view.mv_matrix, 1, GL_FALSE, glm::value_ptr(camera_view_matrix * objects[i].model_matrix));
			glUniform1i(uniforms.view.full_shading, mode == RENDER_FULL ? 1 : 0);
		}
		objects[i].obj.render();
	}

	if (from_light)
	{
		glDisable(GL_POLYGON_OFFSET_FILL);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

int main(int argc, char** argv)
{
	shadowmapping_app a;
	a.run();
	return 0;
}