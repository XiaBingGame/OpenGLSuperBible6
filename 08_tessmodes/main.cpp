#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/** \addtogroup Chapter08*   @{*       \addtogroup tessmodes* 演示不同的 tess 模式, 有四边形, 三角形, 三角形转换成点, 线条.*       @{*/

class tessmodes_app : public OpenGLApp
{
public:
	tessmodes_app() : program_index(0) {}
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Tessellation Modes";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		program[0] = LoadShadersVTF("../media/glsl/08_tessmodes/vertex.vs", "../media/glsl/08_tessmodes/quad.tcs",
			"../media/glsl/08_tessmodes/quad.tes", "../media/glsl/08_tessmodes/fragment.fs");
		program[1] = LoadShadersVTF("../media/glsl/08_tessmodes/vertex.vs", "../media/glsl/08_tessmodes/triangle.tcs",
			"../media/glsl/08_tessmodes/triangle.tes", "../media/glsl/08_tessmodes/fragment.fs");
		program[2] = LoadShadersVTF("../media/glsl/08_tessmodes/vertex.vs", "../media/glsl/08_tessmodes/triangle.tcs",
			"../media/glsl/08_tessmodes/trianglesaspoints.tes", "../media/glsl/08_tessmodes/fragment.fs");
		program[3] = LoadShadersVTF("../media/glsl/08_tessmodes/vertex.vs", "../media/glsl/08_tessmodes/isoline.tcs",
			"../media/glsl/08_tessmodes/isoline.tes", "../media/glsl/08_tessmodes/fragment.fs");

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glPatchParameteri(GL_PATCH_VERTICES, 4);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	virtual void render(double currentTime)
	{
		static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, green);

		glUseProgram(program[program_index]);
		glDrawArrays(GL_PATCHES, 0, 4);
	}

	virtual void shutdown()
	{
		glDeleteVertexArrays(1, &vao);
		for (int i = 0; i < 4; i++)
		{
			glDeleteProgram(program[i]);
		}
		
	}

	void onKey(int key, int scancode, int action, int mods)
	{
		switch(key)
		{
		case 'M':
			if(action)
				program_index = (program_index+1) % 4;
			break;
		default:
			break;
		}
		OpenGLApp::onKey(key, scancode, action, mods);
	}

protected:
	GLuint          program[4];
	GLuint			program_index;
	GLuint          vao;
};

/** @} @} */

int main(int argc, char** argv)
{
	tessmodes_app a;
	a.run();
	return 0;
}