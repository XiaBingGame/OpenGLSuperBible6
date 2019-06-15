#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"

#include <glm/gtc/matrix_transform.hpp>

/** \addtogroup Chapter05*   @{*       \addtogroup simpletexcoords* 纹理的使用例子, 设置纹理参数, *       @{*/

class simpletexcoords_app : public OpenGLApp
{
public:
	simpletexcoords_app()
		: render_prog(0),
		tex_index(0)
	{
	}

protected:
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Texture Coordinates";

		OpenGLApp::init();

		memcpy(info.title, title, sizeof(title));
	}

	virtual void startup()
	{
#define B 0x00, 0x00, 0x00, 0x00
#define W 0xFF, 0xFF, 0xFF, 0xFF
		// 32 x 16 x 16
		static const GLubyte tex_data[] =
		{
			B, W, B, W, B, W, B, W, B, W, B, W, B, W, B, W,
			W, B, W, B, W, B, W, B, W, B, W, B, W, B, W, B,
			B, W, B, W, B, W, B, W, B, W, B, W, B, W, B, W,
			W, B, W, B, W, B, W, B, W, B, W, B, W, B, W, B,
			B, W, B, W, B, W, B, W, B, W, B, W, B, W, B, W,
			W, B, W, B, W, B, W, B, W, B, W, B, W, B, W, B,
			B, W, B, W, B, W, B, W, B, W, B, W, B, W, B, W,
			W, B, W, B, W, B, W, B, W, B, W, B, W, B, W, B,
			B, W, B, W, B, W, B, W, B, W, B, W, B, W, B, W,
			W, B, W, B, W, B, W, B, W, B, W, B, W, B, W, B,
			B, W, B, W, B, W, B, W, B, W, B, W, B, W, B, W,
			W, B, W, B, W, B, W, B, W, B, W, B, W, B, W, B,
			B, W, B, W, B, W, B, W, B, W, B, W, B, W, B, W,
			W, B, W, B, W, B, W, B, W, B, W, B, W, B, W, B,
			B, W, B, W, B, W, B, W, B, W, B, W, B, W, B, W,
			W, B, W, B, W, B, W, B, W, B, W, B, W, B, W, B,
		};
#undef B
#undef W

		glGenTextures(1, &tex_object[0]);
		glBindTexture(GL_TEXTURE_2D, tex_object[0]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, 16, 16);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, tex_data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		tex_object[1] = loadKTX("../media/textures/pattern1.ktx", 0);

		object.load("../media/models/torus_nrms_tc.sbm");
		//object.load("../media/models/dragon.sbm");

		load_shaders();


		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	}

	virtual void render(double currentTime)
	{
		static const GLfloat gray[] = { 0.2f, 0.2f, 0.2f, 1.0f };
		static const GLfloat ones[] = { 1.0f };

		glClearBufferfv(GL_COLOR, 0, gray);
		glClearBufferfv(GL_DEPTH, 0, ones);

		glViewport(0, 0, info.windowWidth, info.windowHeight);

		glBindTexture(GL_TEXTURE_2D, tex_object[tex_index]);

		glUseProgram(render_prog);

		glm::mat4 proj_matrix = glm::perspective(glm::radians(60.0f), (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);
		glm::mat4 mv_matrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, -3.0));
		mv_matrix = glm::rotate(mv_matrix, glm::radians((float)currentTime * 19.3f), glm::vec3(0.0f, 1.0f, 0.0f));
		mv_matrix = glm::rotate(mv_matrix, glm::radians((float)currentTime * 21.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		glUniformMatrix4fv(uniforms.mv_matrix, 1, GL_FALSE, &mv_matrix[0][0]);
		glUniformMatrix4fv(uniforms.proj_matrix, 1, GL_FALSE, &proj_matrix[0][0]);

		object.render();
	}

	virtual void shutdown()
	{
		glDeleteProgram(render_prog);
		glDeleteTextures(2, tex_object);
	}

	virtual void onKey(int key, int scancode, int action, int mods)
	{
		if (action)
		{
			switch (key)
			{
			case 'R': load_shaders();
				break;
			case 'T':
				tex_index++;
				if (tex_index > 1)
					tex_index = 0;
				break;
			}
		}
		OpenGLApp::onKey(key, scancode, action, mods);
	}

protected:
	GLuint          render_prog;

	GLuint          tex_object[2];
	GLuint          tex_index;

	struct
	{
		GLint       mv_matrix;
		GLint       proj_matrix;
	} uniforms;

	sb6::object     object;

private:
	void load_shaders()
	{
		glDeleteProgram(render_prog);
		render_prog = LoadShadersVF("../media/glsl/05_simpletexcoords.vs", "../media/glsl/05_simpletexcoords.fs");
		uniforms.mv_matrix = glGetUniformLocation(render_prog, "mv_matrix");
		uniforms.proj_matrix = glGetUniformLocation(render_prog, "proj_matrix");
	}
};


class singletri_app : public OpenGLApp
{
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Single Triangle";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}

	virtual void startup()
	{
		program = LoadShadersVF("../media/glsl/02_singletri.vs", "../media/glsl/02_singletri.fs");
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

	virtual void render(double currentTime)
	{
		static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, green);

		glUseProgram(program);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	virtual void shutdown()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteProgram(program);
	}

protected:
	GLuint          program;
	GLuint          vao;
};

/** @} @} */

int main(int argc, char** argv)
{
	simpletexcoords_app a;
	a.run();

	return 0;
}