#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/** \addtogroup Chapter08*   @{*       \addtogroup tesssubdivmodes** TCS, TES 子划分图元, 演示不同的子划分策略, 相等, 偶数, 奇数*       @{*/

class tesssubdivmodes_app : public OpenGLApp
{
public:
	tesssubdivmodes_app() : program_index(0) {}
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Tessellation Subdivision Modes";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		program[0] = LoadShadersVTF("../media/glsl/08_tesssubdivmodes/vertex.vs", "../media/glsl/08_tesssubdivmodes/triangle.tcs",
			"../media/glsl/08_tesssubdivmodes/equal.tes", "../media/glsl/08_tesssubdivmodes/fragment.fs");
		program[1] = LoadShadersVTF("../media/glsl/08_tesssubdivmodes/vertex.vs", "../media/glsl/08_tesssubdivmodes/triangle.tcs",
			"../media/glsl/08_tesssubdivmodes/fract_even.tes", "../media/glsl/08_tesssubdivmodes/fragment.fs");
		program[2] = LoadShadersVTF("../media/glsl/08_tesssubdivmodes/vertex.vs", "../media/glsl/08_tesssubdivmodes/triangle.tcs",
			"../media/glsl/08_tesssubdivmodes/fract_odd.tes", "../media/glsl/08_tesssubdivmodes/fragment.fs");
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

		for (int i = 0; i < 3; i++)
		{
			glDeleteProgram(program[i]);
		}
	}

protected:
	void onKey(int key, int scancode, int action, int mods)
	{
		if (key == 'M' && action == GLFW_PRESS)
		{
			program_index = (program_index+1) % 3;
			return;
		}
		OpenGLApp::onKey(key, scancode, action, mods);
	}

	GLuint          program[3];
	GLuint          vao;
	GLuint			program_index;
};

/** @} @} */

int main(int argc, char** argv)
{
	tesssubdivmodes_app a;
	a.run();
	return 0;
}