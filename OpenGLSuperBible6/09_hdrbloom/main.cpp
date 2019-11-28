#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/** \addtogroup Chapter09*   @{*       \addtogroup hdrbloom* * 主要演示场景的后期处理, 输出至纹理后, 对纹理进行后期处理.**	- 第一个着色器如常绘制了场景, 其内容输出至两个颜色纹理中. 这其中充分利用了 ubo 的功能. 第一个纹理是原本内容, 第二个纹理是亮度内容.*	- 过滤的时候, 第一层过滤对亮度纹理每个像素其左右13个位置进行权衡求和. 第二层过滤对前一次过滤的结果在进行一次13个位置权重求和.*	- 最后一个着色器程序使用场景纹理和过滤后的纹理进行叠加产生晕眩效果.*	- GL_DEPTH_COMPONENT32F 格式的纹理.
*	- 每个 FBO 设置之后, 使用 glDrawBuffers() 设置绘制的 buffer.
*	- GL_R32F 格式的纹理.
*	- glBufferData() 设置 buffer 的数据.
*	- 本例还是使用了 uniform block 的 buffer 技术.
*	- glClearBufferfv() 第二个参数可以设置哪个颜色挂接点.*       @{*/

enum
{
	MAX_SCENE_WIDTH     = 2048,
	MAX_SCENE_HEIGHT    = 2048,
	SPHERE_COUNT        = 32,
};

class hdrbloom_app : public OpenGLApp
{
public:
	hdrbloom_app() 
		: exposure(1.0f),
		  program_render(0),
		  program_filter(0),
		  program_resolve(0),
		  mode(0),
		  paused(false),
		  bloom_factor(1.0f),
		  show_bloom(true),
		  show_scene(true),
		  bloom_thresh_min(0.8f),
		  bloom_thresh_max(1.2f),
		  show_prefilter(false)
	  {
	  }

	void init()
	{
		static const char title[] = "OpenGL SuperBible - HDR Bloom";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		int i;
		static const GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		load_shaders();

		static const GLfloat exposureLUT[20]   = { 11.0f, 6.0f, 3.2f, 2.8f, 2.2f, 1.90f, 1.80f, 1.80f, 1.70f, 1.70f,  1.60f, 1.60f, 1.50f, 1.50f, 1.40f, 1.40f, 1.30f, 1.20f, 1.10f, 1.00f };

		glGenFramebuffers(1, &render_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);

		// 生成用于 FBO 输出的纹理.
		glGenTextures(1, &tex_scene);
		glBindTexture(GL_TEXTURE_2D, tex_scene);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, MAX_SCENE_WIDTH, MAX_SCENE_HEIGHT);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex_scene, 0);
		glGenTextures(1, &tex_brightpass);
		glBindTexture(GL_TEXTURE_2D, tex_brightpass);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, MAX_SCENE_WIDTH, MAX_SCENE_HEIGHT);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, tex_brightpass, 0);
		glGenTextures(1, &tex_depth);
		glBindTexture(GL_TEXTURE_2D, tex_depth);
		// GL_DEPTH_COMPONENT32F 格式的纹理.
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, MAX_SCENE_WIDTH, MAX_SCENE_HEIGHT);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex_depth, 0);
		// 每个 FBO 设置之后, 使用 glDrawBuffers() 设置绘制的 buffer.
		glDrawBuffers(2, buffers);

		glGenFramebuffers(2, &filter_fbo[0]);
		glGenTextures(2, &tex_filter[0]);
		for (i = 0; i < 2; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, filter_fbo[i]);
			glBindTexture(GL_TEXTURE_2D, tex_filter[i]);
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, i ? MAX_SCENE_WIDTH : MAX_SCENE_HEIGHT, i ? MAX_SCENE_HEIGHT : MAX_SCENE_WIDTH);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex_filter[i], 0);
			glDrawBuffers(1, buffers);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// GL_R32F 格式的纹理.
		glGenTextures(1, &tex_lut);
		glBindTexture(GL_TEXTURE_1D, tex_lut);
		glTexStorage1D(GL_TEXTURE_1D, 1, GL_R32F, 20);
		glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 20, GL_RED, GL_FLOAT, exposureLUT);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

		object.load("../media/models/sphere.sbm");

		// glBufferData() 设置 buffer 的数据.
		glGenBuffers(1, &ubo_transform);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo_transform);
		glBufferData(GL_UNIFORM_BUFFER, (2 + SPHERE_COUNT) * sizeof(mmat4), NULL, GL_DYNAMIC_DRAW);

		struct material
		{
			glm::vec3     diffuse_color;
			unsigned int    : 32;           // pad
			glm::vec3     specular_color;
			float           specular_power;
			glm::vec3     ambient_color;
			unsigned int    : 32;           // pad
		};
		// 本例还是使用了 uniform block 的 buffer 技术.
		glGenBuffers(1, &ubo_material);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo_material);
		glBufferData(GL_UNIFORM_BUFFER, SPHERE_COUNT * sizeof(material), NULL, GL_STATIC_DRAW);

		material * m = (material *)glMapBufferRange(GL_UNIFORM_BUFFER, 0, SPHERE_COUNT * sizeof(material), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
		float ambient = 0.002f;
		for (i = 0; i < SPHERE_COUNT; i++)
		{
			float fi = 3.14159267f * (float)i / 8.0f;
			m[i].diffuse_color  = glm::vec3(sinf(fi) * 0.5f + 0.5f, sinf(fi + 1.345f) * 0.5f + 0.5f, sinf(fi + 2.567f) * 0.5f + 0.5f);
			m[i].specular_color = glm::vec3(2.8f, 2.8f, 2.9f);
			m[i].specular_power = 30.0f;
			m[i].ambient_color  = glm::vec3(ambient * 0.025f);
			ambient *= 1.5f;
		}
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}

	virtual void render(double currentTime)
	{
		static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		static const GLfloat one = 1.0f;
		int i;
		static double last_time = 0.0;
		static double total_time = 0.0;

		if (!paused)
			total_time += (currentTime - last_time);
		last_time = currentTime;
		float t = (float)total_time;

		glViewport(0, 0, info.windowWidth, info.windowHeight);

		// glClearBufferfv() 第二个参数可以设置哪个颜色挂接点.
		// 首先内容写入两个纹理中.
		glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);
		glClearBufferfv(GL_COLOR, 0, black);
		glClearBufferfv(GL_COLOR, 1, black);
		glClearBufferfv(GL_DEPTH, 0, &one);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glUseProgram(program_render);

		glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_transform);
		struct transforms_t
		{
			mmat4 mat_proj;
			mmat4 mat_view;
			mmat4 mat_model[SPHERE_COUNT];
		} * transforms = (transforms_t *)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(transforms_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
		memcpy(transforms->mat_proj, glm::value_ptr(glm::perspective(glm::radians(50.0f), (float)info.windowWidth / (float)info.windowHeight, 1.0f, 1000.0f)), 16 * sizeof(float));
		memcpy(transforms->mat_view, glm::value_ptr(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -20.0f))), 16 * sizeof(float));
		for (i = 0; i < SPHERE_COUNT; i++)
		{
			float fi = 3.141592f * (float)i / 16.0f;
			// float r = cosf(fi * 0.25f) * 0.4f + 1.0f;
			float r = (i & 2) ? 0.6f : 1.5f;
			memcpy(transforms->mat_model[i], 
				glm::value_ptr(glm::translate(glm::mat4(1.0f), glm::vec3(cosf(t + fi) * 5.0f * r, sinf(t + fi * 4.0f) * 4.0f, sinf(t + fi) * 5.0f * r))), 16 * sizeof(float));
		}
		glUnmapBuffer(GL_UNIFORM_BUFFER);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo_material);

		glUniform1f(uniforms.scene.bloom_thresh_min, bloom_thresh_min);
		glUniform1f(uniforms.scene.bloom_thresh_max, bloom_thresh_max);

		object.render(SPHERE_COUNT);

		glDisable(GL_DEPTH_TEST);

		glUseProgram(program_filter);

		glBindVertexArray(vao);

		glBindFramebuffer(GL_FRAMEBUFFER, filter_fbo[0]);
		glBindTexture(GL_TEXTURE_2D, tex_brightpass);
		glViewport(0, 0, info.windowHeight, info.windowWidth);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glBindFramebuffer(GL_FRAMEBUFFER, filter_fbo[1]);
		glBindTexture(GL_TEXTURE_2D, tex_filter[0]);
		glViewport(0, 0, info.windowWidth, info.windowHeight);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glUseProgram(program_resolve);

		glUniform1f(uniforms.resolve.exposure, exposure);
		if (show_prefilter)
		{
			glUniform1f(uniforms.resolve.bloom_factor, 0.0f);
			glUniform1f(uniforms.resolve.scene_factor, 1.0f);
		}
		else
		{
			glUniform1f(uniforms.resolve.bloom_factor, show_bloom ? bloom_factor : 0.0f);
			glUniform1f(uniforms.resolve.scene_factor, show_scene ? 1.0f : 0.0f);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, tex_filter[1]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, show_prefilter ? tex_brightpass : tex_scene);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	virtual void shutdown()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteProgram(program_render);
		glDeleteProgram(program_filter);
		glDeleteProgram(program_resolve);

		glDeleteTextures(1, &tex_src);
		glDeleteTextures(1, &tex_lut);
	}

	void load_shaders()
	{
		if (program_render)
			glDeleteProgram(program_render);
		program_render = LoadShadersVF("../media/glsl/09_hdrbloom/hdrbloom-scene.vs", "../media/glsl/09_hdrbloom/hdrbloom-scene.fs");
		uniforms.scene.bloom_thresh_min = glGetUniformLocation(program_render, "bloom_thresh_min");
		uniforms.scene.bloom_thresh_max = glGetUniformLocation(program_render, "bloom_thresh_max");

		if (program_filter)
			glDeleteProgram(program_filter);
		program_filter = LoadShadersVF("../media/glsl/09_hdrbloom/hdrbloom-filter.vs", "../media/glsl/09_hdrbloom/hdrbloom-filter.fs");

		if (program_resolve)
			glDeleteProgram(program_resolve);
		program_resolve = LoadShadersVF("../media/glsl/09_hdrbloom/hdrbloom-resolve.vs", "../media/glsl/09_hdrbloom/hdrbloom-resolve.fs");
		uniforms.resolve.exposure = glGetUniformLocation(program_resolve, "exposure");
		uniforms.resolve.bloom_factor = glGetUniformLocation(program_resolve, "bloom_factor");
		uniforms.resolve.scene_factor = glGetUniformLocation(program_resolve, "scene_factor");
	}

	void onKey(int key, int scancode, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			switch(key)
			{
			case '1':
			case '2':
			case '3':
				mode = key - '1';
				break;
			case 'B':
				show_bloom = !show_bloom;
				break;
			case 'V':
				show_scene = !show_scene;
				break;
			case 'A':
				bloom_factor += 0.1f;
				break;
			case 'Z':
				bloom_factor -= 0.1f;
				break;
			case 'S':
				bloom_thresh_min += 0.1f;
				break;
			case 'X':
				bloom_thresh_min -= 0.1f;
				break;
			case 'D':
				bloom_thresh_max += 0.1f;
				break;
			case 'C':
				bloom_thresh_max -= 0.1f;
				break;
			case 'R':
				load_shaders();
				break;
			case 'N':
				show_prefilter = !show_prefilter;
				break;
			case 'M':
				mode = (mode + 1) % 3;
				break;
			case 'P':
				paused = !paused;
				break;
			case GLFW_KEY_UP:
				exposure *= 1.1f;
				break;
			case GLFW_KEY_DOWN:
				exposure /= 1.1f;
				break;
			default:
				break;
			}
		}
		OpenGLApp::onKey(key, scancode, action, mods);
	}

protected:
	GLuint      tex_src;
	GLuint      tex_lut;

	GLuint      render_fbo;
	GLuint      filter_fbo[2];

	GLuint      tex_scene;
	GLuint      tex_brightpass;
	GLuint      tex_depth;
	GLuint      tex_filter[2];

	GLuint      program_render;
	GLuint      program_filter;
	GLuint      program_resolve;
	GLuint      vao;
	float       exposure;
	int         mode;
	bool        paused;
	float       bloom_factor;
	bool        show_bloom;
	bool        show_scene;
	bool        show_prefilter;
	float       bloom_thresh_min;
	float       bloom_thresh_max;

	struct
	{
		struct
		{
			int bloom_thresh_min;
			int bloom_thresh_max;
		} scene;
		struct
		{
			int exposure;
			int bloom_factor;
			int scene_factor;
		} resolve;
	} uniforms;

	GLuint      ubo_transform;
	GLuint      ubo_material;

	sb6::object object;
};

/** @} @} */

int main(int argc, char** argv)
{
	hdrbloom_app a;
	a.run();
	return 0;
}