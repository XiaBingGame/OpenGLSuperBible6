#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/** \addtogroup Chapter09*   @{*       \addtogroup hdrtonemap* 类似于例子 hdrexposure, 只是 adaptive 程序通过算法使得一张图不过份亮也不过分暗, 其算法是每个纹素读取上下左右共25个纹素通过算法得到合适的纹素亮度.*       @{*/

class hdrtonemap_app : public OpenGLApp
{
public:
	hdrtonemap_app() :
		  exposure(1.0f),
		  program_adaptive(0),
		  program_exposure(0),
		  program_naive(0),
		  mode(0)  {}

	void init()
	{
		static const char title[] = "OpenGL SuperBible - HDR Tone Mapping";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		tex_src = loadKTX("../media/textures/treelights_2k.ktx");
		glBindTexture(GL_TEXTURE_2D, tex_src);

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		load_shaders();

		static const GLfloat exposureLUT[20]   = { 11.0f, 6.0f, 3.2f, 2.8f, 2.2f, 1.90f, 1.80f, 1.80f, 1.70f, 1.70f,  1.60f, 1.60f, 1.50f, 1.50f, 1.40f, 1.40f, 1.30f, 1.20f, 1.10f, 1.00f };

		glGenTextures(1, &tex_lut);
		glBindTexture(GL_TEXTURE_1D, tex_lut);
		glTexStorage1D(GL_TEXTURE_1D, 1, GL_R32F, 20);
		glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 20, GL_RED, GL_FLOAT, exposureLUT);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	}

	virtual void render(double currentTime)
	{
		static const GLfloat black[] = { 0.0f, 0.25f, 0.0f, 1.0f };
		glViewport(0, 0, info.windowWidth, info.windowHeight);

		glClearBufferfv(GL_COLOR, 0, black);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_1D, tex_lut);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex_src);

		// glUseProgram(mode ? program_adaptive : program_naive);
		switch (mode)
		{
		case 0:
			glUseProgram(program_naive);
			break;
		case 1:
			glUseProgram(program_exposure);
			glUniform1f(uniforms.exposure.exposure, exposure);
			break;
		case 2:
			glUseProgram(program_adaptive);
			break;
		}
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	virtual void shutdown()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteProgram(program_naive);
		glDeleteProgram(program_exposure);
		glDeleteProgram(program_adaptive);
		glDeleteTextures(1, &tex_src);
		glDeleteTextures(1, &tex_lut);
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
			case 'R':
				load_shaders();
				break;
			case 'M':
				mode = (mode + 1) % 3;
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

private:
	void load_shaders();

protected:
	GLuint      tex_src;
	GLuint      tex_lut;

	GLuint      program_naive;
	GLuint      program_exposure;
	GLuint      program_adaptive;
	GLuint      vao;
	float       exposure;
	int         mode;

	struct
	{
		struct
		{
			int exposure;
		} exposure;
	} uniforms;
};

void hdrtonemap_app::load_shaders()
{
	if (program_naive)
		glDeleteProgram(program_naive);
	program_naive = LoadShadersVF("../media/glsl/09_hdrtonemap/tonemap.vs", "../media/glsl/09_hdrtonemap/tonemap_naive.fs");

	if (program_exposure)
		glDeleteProgram(program_exposure);
	program_exposure = LoadShadersVF("../media/glsl/09_hdrtonemap/tonemap.vs", "../media/glsl/09_hdrtonemap/tonemap_exposure.fs");

	if (program_adaptive)
		glDeleteProgram(program_adaptive);
	program_adaptive = LoadShadersVF("../media/glsl/09_hdrtonemap/tonemap.vs", "../media/glsl/09_hdrtonemap/tonemap_adaptive.fs");

	uniforms.exposure.exposure = glGetUniformLocation(program_exposure, "exposure");
}

/** @} @} */

int main(int argc, char** argv)
{
	hdrtonemap_app a;
	a.run();
	return 0;
}