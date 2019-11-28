#include <windows.h>
#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


enum
{
	MAX_DISPLAY_WIDTH       = 2048,
	MAX_DISPLAY_HEIGHT      = 2048,
	NUM_LIGHTS              = 64,
	NUM_INSTANCES           = (15 * 15)
};

class deferredshading_app : public OpenGLApp
{
public:
	deferredshading_app()
		: render_program_nm(0),
		  render_program(0),
		  light_program(0),
		  vis_program(0),
		  use_nm(true),
		  paused(false),
		  vis_mode(VIS_OFF)
	{}
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Deferred Shading";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		
		glGenFramebuffers(1, &gbuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, gbuffer);

		glGenTextures(3, gbuffer_tex);
		glBindTexture(GL_TEXTURE_2D, gbuffer_tex[0]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32UI, MAX_DISPLAY_WIDTH, MAX_DISPLAY_HEIGHT); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, gbuffer_tex[1]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, MAX_DISPLAY_WIDTH, MAX_DISPLAY_HEIGHT); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, gbuffer_tex[2]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, MAX_DISPLAY_WIDTH, MAX_DISPLAY_HEIGHT); 

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, gbuffer_tex[0], 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, gbuffer_tex[1], 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, gbuffer_tex[2], 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glGenVertexArrays(1, &fs_quad_vao);
		glBindVertexArray(fs_quad_vao);

		object.load("../media/models/ladybug.sbm");
		tex_nm = loadKTX("../media/textures/ladybug_nm.ktx");
		tex_diffuse = loadKTX("../media/textures/ladybug_co.ktx");

		load_shaders();

		glGenBuffers(1, &light_ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, light_ubo);
		glBufferData(GL_UNIFORM_BUFFER, NUM_LIGHTS * sizeof(light_t), NULL, GL_DYNAMIC_DRAW);

		glGenBuffers(1, &render_transform_ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, render_transform_ubo);
		glBufferData(GL_UNIFORM_BUFFER, (2 + NUM_INSTANCES) * sizeof(float) * 16, NULL, GL_DYNAMIC_DRAW);
	}

	virtual void render(double currentTime)
	{
		static const GLuint uint_zeros[] = { 0, 0, 0, 0 };
		static const GLfloat float_zeros[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		static const GLfloat float_ones[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		static const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		int i, j;
		static double last_time = currentTime;
		static double total_time = 0.0;

		if (!paused)
		{
			total_time += (currentTime - last_time);
		}
		else
		{
#ifdef _WIN32
			Sleep(10);
#endif
		}
		last_time = currentTime;

		float t = (float)total_time;

		glBindFramebuffer(GL_FRAMEBUFFER, gbuffer);
		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glDrawBuffers(2, draw_buffers);
		glClearBufferuiv(GL_COLOR, 0, uint_zeros);
		glClearBufferuiv(GL_COLOR, 1, uint_zeros);
		glClearBufferfv(GL_DEPTH, 0, float_ones);

		glBindBufferBase(GL_UNIFORM_BUFFER, 0, render_transform_ubo);
		float* matrices = reinterpret_cast<float*>(glMapBufferRange(GL_UNIFORM_BUFFER,
			0,
			(2 + NUM_INSTANCES) * sizeof(float)*16,
			GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

		mat4copy(&matrices[0], glm::perspective(glm::radians(50.0f),
			(float)info.windowWidth / (float)info.windowHeight,
			0.1f,
			1000.0f));
		float d = (sinf(t * 0.131f) + 2.0f) * 0.15f;
		glm::vec3 eye_pos = glm::vec3(d * 120.0f * sinf(t * 0.11f),
			5.5f,
			d * 120.0f * cosf(t * 0.01f));
		mat4copy(&matrices[16], glm::lookAt(eye_pos,
			glm::vec3(0.0f, -20.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f)));

		for (j = 0; j < 15; j++)
		{
			float j_f = (float)j;
			for (i = 0; i < 15; i++)
			{
				float i_f = (float)i;

				mat4copy(&matrices[(j * 15 + i + 2)*16], glm::translate(glm::mat4(1.0f), glm::vec3((i - 7.5f) * 7.0f, 0.0f, (j - 7.5f) * 11.0f)));
			}
		}

		glUnmapBuffer(GL_UNIFORM_BUFFER);

		glUseProgram(use_nm ? render_program_nm : render_program);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex_diffuse);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, tex_nm);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		object.render(NUM_INSTANCES);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glDrawBuffer(GL_BACK);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gbuffer_tex[0]);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gbuffer_tex[1]);

		if (vis_mode == VIS_OFF)
		{
			glUseProgram(light_program);
		}
		else
		{
			glUseProgram(vis_program);
			glUniform1i(loc_vis_mode, vis_mode);
		}

		glDisable(GL_DEPTH_TEST);

		glBindBufferBase(GL_UNIFORM_BUFFER, 0, light_ubo);
		light_t * lights = reinterpret_cast<light_t *>(glMapBufferRange(GL_UNIFORM_BUFFER,
			0,
			NUM_LIGHTS * sizeof(light_t),
			GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
		for (i = 0; i < NUM_LIGHTS; i++)
		{
			float i_f = ((float)i - 7.5f) * 0.1f + 0.3f;
			// t = 0.0f;
			memcpy(lights[i].position, glm::value_ptr(glm::vec3(100.0f * sinf(t * 1.1f + (5.0f * i_f)) * cosf(t * 2.3f + (9.0f * i_f)),
				15.0f,
				100.0f * sinf(t * 1.5f + (6.0f * i_f)) * cosf(t * 1.9f + (11.0f * i_f)))), sizeof(float)*3); // 300.0f * sinf(t * i_f * 0.7f) * cosf(t * i_f * 0.9f) - 600.0f);
			memcpy(lights[i].color, glm::value_ptr(glm::vec3(cosf(i_f * 14.0f) * 0.5f + 0.8f,
				sinf(i_f * 17.0f) * 0.5f + 0.8f,
				sinf(i_f * 13.0f) * cosf(i_f * 19.0f) * 0.5f + 0.8f)), sizeof(float)*3);
			// lights[i].color = glm::vec3(1.0);
			// glm::vec3(0.5f, 0.4f, 0.75f);
		}

		glUnmapBuffer(GL_UNIFORM_BUFFER);

		glBindVertexArray(fs_quad_vao);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void onKey(int key, int scancode, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			switch(key)
			{
			case 'R': load_shaders();
				break;
			case 'P': paused = !paused;
				break;
			case 'N': use_nm = !use_nm;
				break;
			case '1': vis_mode = VIS_OFF;
				break;
			case '2': vis_mode = VIS_NORMALS;
				break;
			case '3': vis_mode = VIS_WS_COORDS;
				break;
			case '4': vis_mode = VIS_DIFFUSE;
				break;
			case '5': vis_mode = VIS_META;
				break;
			default:
				break;
			}
		}
		OpenGLApp::onKey(key, scancode, action, mods);
	}

	virtual void shutdown()
	{
		glDeleteTextures(3, &gbuffer_tex[0]);
		glDeleteFramebuffers(1, &gbuffer);
		glDeleteProgram(render_program);
		glDeleteProgram(light_program);
	}

private:
	void load_shaders();

protected:
	GLuint      gbuffer;
	GLuint      gbuffer_tex[3];
	GLuint      fs_quad_vao;

	sb6::object object;

	GLuint      render_program;
	GLuint      render_program_nm;
	GLuint      render_transform_ubo;

	GLuint      light_program;
	GLuint      light_ubo;

	GLuint      vis_program;
	GLint       loc_vis_mode;

	GLuint      tex_diffuse;
	GLuint      tex_nm;

	bool        use_nm;
	bool        paused;

	enum
	{
		VIS_OFF,
		VIS_NORMALS,
		VIS_WS_COORDS,
		VIS_DIFFUSE,
		VIS_META
	} vis_mode;

#pragma pack (push, 1)
	struct light_t
	{
		mvec3         position;
		unsigned int        : 32;       // pad0
		mvec3         color;
		unsigned int        : 32;       // pad1
	};
#pragma pack (pop)
};

void deferredshading_app::load_shaders()
{
	if (render_program)
		glDeleteProgram(render_program);
	if (light_program)
		glDeleteProgram(light_program);
	if (render_program_nm)
		glDeleteProgram(render_program_nm);
	if (vis_program)
		glDeleteProgram(vis_program);

	render_program = LoadShadersVF("../media/glsl/12_deferredshading/render.vs", "../media/glsl/12_deferredshading/render.fs");
	render_program_nm = LoadShadersVF("../media/glsl/12_deferredshading/render-nm.vs", "../media/glsl/12_deferredshading/render-nm.fs");
	light_program = LoadShadersVF("../media/glsl/12_deferredshading/light.vs", "../media/glsl/12_deferredshading/light.fs");
	vis_program = LoadShadersVF("../media/glsl/12_deferredshading/light.vs", "../media/glsl/12_deferredshading/render-vis.fs");

	loc_vis_mode = glGetUniformLocation(vis_program, "vis_mode");
}

int main(int argc, char** argv)
{
	deferredshading_app a;
	a.run();
	return 0;
}