#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/** \addtogroup Chapter07*   @{*       \addtogroup grass* 通过实例化绘制草原.*	- 生成了五个纹理, 一共 1024x1024 个实例.*	- 通过实例绘制不同地方的草*       @{*/

class grass_app : public OpenGLApp
{
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Grass";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}

	virtual void startup()
	{
		static const GLfloat grass_blade[] = {
			-0.3f, 0.0f,
			0.3f, 0.0f,
			-0.2f, 1.0f,
			0.1f, 1.3f,
			-0.05f, 2.3f,
			0.0f, 3.3f
		};

		grass_program = LoadShadersVF("../media/glsl/07_grass.vs", "../media/glsl/07_grass.fs");
		uniforms.mvpMatrix = glGetUniformLocation(grass_program, "mvpMatrix");

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glGenVertexArrays(1, &grass_vao);
		glBindVertexArray(grass_vao);

		glGenBuffers(1, &grass_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, grass_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(grass_blade), grass_blade, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);

		glActiveTexture(GL_TEXTURE0);
		tex_grass_palette = generatePaletteTexture();
		//generatePaletteTexture();
		glActiveTexture(GL_TEXTURE1);
		tex_grass_length = loadKTX("../media/textures/grass_length.ktx");
		glActiveTexture(GL_TEXTURE2);
		tex_grass_orientation = loadKTX("../media/textures/grass_orientation.ktx");
		glActiveTexture(GL_TEXTURE3);
		tex_grass_color = loadKTX("../media/textures/grass_color.ktx");
		glActiveTexture(GL_TEXTURE4);
		tex_grass_bend = loadKTX("../media/textures/grass_bend.ktx");
	}

	virtual void render(double currentTime)
	{
		float t = (float)currentTime * 0.02f;
		float r = 550.0f;

		static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, black);
		static const float one = 1.0f;
		glClearBufferfv(GL_DEPTH, 0, &one);

		glm::mat4 mv_matrix = glm::lookAt(glm::vec3(sinf(t) * r, 25.0f, cosf(t) * r),
										glm::vec3(0.0f, -50.0f, 0.0f),
										glm::vec3(0.0f, 1.0f, 0.0f));

		//mv_matrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		
		glm::mat4 proj_matrix = glm::perspective(glm::radians(45.0f), (float)info.windowWidth/(float)info.windowHeight, 0.1f, 1000.0f);
		glm::mat4 mvp = proj_matrix * mv_matrix;

		glUseProgram(grass_program);
		glUniformMatrix4fv(uniforms.mvpMatrix, 1, GL_FALSE, &mvp[0][0]);
		
		//glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 6, 1024*1024);
	}

	virtual void shutdown()
	{
		glDeleteTextures(1, &tex_grass_palette);
		glDeleteTextures(1, &tex_grass_length);
		glDeleteTextures(1, &tex_grass_color);
		glDeleteTextures(1, &tex_grass_bend);
		glDeleteTextures(1, &tex_grass_orientation);
		glDeleteBuffers(1, &grass_buffer);
		glDeleteVertexArrays(1, &grass_vao);
		glDeleteProgram(grass_program);
	}

private:
	GLuint generatePaletteTexture()
	{
		float* data = new float[256*4];
		GLuint tex;
		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_1D, tex);
		glTexStorage1D(GL_TEXTURE_1D,
			8,               // 8 mipmap levels
			GL_RGBA32F,      // 32-bit floating-point RGBA data
			256);       // 256 x 256 texels

		// 枯黄色 DE B8 87
		// 绿色 00 FF 00
		float red = 0.0f;
		float green = 0.0f;
		float blue = 0.0f;
		float rstep = -red/256.0f;
		float gstep = (255.0f - green)/256.0f;
		float bstep = -blue/256.0f;
		int x, y;
		int width = 256;
		for (int i = 0; i < 256; i++)
		{
			float factor = 1.0;
			data[i*4 + 0] = factor*red/255.0f;
			data[i*4 + 1] = factor*green/255.0f;
			data[i*4 + 2] = factor*blue/255.0f;
			data[i*4 + 3] = 1.0f;
			red += rstep;
			green += gstep;
			blue += bstep;
		}

		// Assume the texture is already bound to the GL_TEXTURE_2D target
		glTexSubImage1D(GL_TEXTURE_1D,  // 2D texture
			0,              // Level 0
			0,          // Offset 0
			256,    // 256 x 256 texels, replace entire image
			GL_RGBA,        // Four channel data
			GL_FLOAT,       // Floating point data
			data);          // Pointer to data
		//glGenerateMipmap(GL_TEXTURE_1D);
		delete [] data;
		return tex;
	}

	void generate_texture2()
	{
		// Generate a name for the texture
		glGenTextures(1, &tex_grass_palette);
		// Now bind it to the context using the GL_TEXTURE_2D binding point
		glBindTexture(GL_TEXTURE_2D, tex_grass_palette);

		// Specify the amount of storage we want to use for the texture
		glTexStorage2D(GL_TEXTURE_2D,   // 2D texture
			8,               // 8 mipmap levels
			GL_RGBA32F,      // 32-bit floating-point RGBA data
			256, 256);       // 256 x 256 texels

		// Define some data to upload into the texture
		float * data = new float[256 * 256 * 4];

		// generate_texture() is a function that fills memory with image data
		generate_texture(data, 256, 256);

		// Assume the texture is already bound to the GL_TEXTURE_2D target
		glTexSubImage2D(GL_TEXTURE_2D,  // 2D texture
			0,              // Level 0
			0, 0,           // Offset 0, 0
			256, 256,       // 256 x 256 texels, replace entire image
			GL_RGBA,        // Four channel data
			GL_FLOAT,       // Floating point data
			data);          // Pointer to data

		// Free the memory we allocated before - \GL now has our data
		delete [] data;
	}

	void generate_texture(float * data, int width, int height)
	{
		int x, y;

		for (y = 0; y < height; y++)
		{
			for (x = 0; x < width; x++)
			{
				data[(y * width + x) * 4 + 0] = (float)(x) / 255.0f;
				data[(y * width + x) * 4 + 1] = (float)((x | y) & 0xFF) / 255.0f;
				data[(y * width + x) * 4 + 2] = (float)((x ^ y) & 0xFF) / 255.0f;
				data[(y * width + x) * 4 + 3] = 1.0f;
			}
		}
	}

protected:
	GLuint          grass_program;
	GLuint          grass_vao;

	GLuint			grass_buffer;
	GLuint			tex_grass_palette;
	GLuint			tex_grass_length;
	GLuint			tex_grass_orientation;
	GLuint			tex_grass_color;
	GLuint			tex_grass_bend;

	struct {
		GLuint mvpMatrix;
	} uniforms;
};

/** @} @} */

int main(int argc, char** argv)
{
	grass_app a;
	a.run();
	return 0;
}