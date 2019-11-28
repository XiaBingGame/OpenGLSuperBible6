#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/** \addtogroup Chapter06*   @{*       \addtogroup subroutines* ��ʾ����ɫ���д�������ӳ���, ����ͨ�� uniform ѡ��ͬ���ӳ���.**	- glUniformSubroutinesuiv �����ӳ���*	- glGetSubroutineIndex ��ȡ�ӳ��������
*	- glGetSubroutineUniformLocation ��ȡ�ӳ����λ��*       @{*/

class subroutines_app : public OpenGLApp
{
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Shader subroutines";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}

	virtual void startup()
	{
		load_shaders();
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

	virtual void render(double currentTime)
	{
		int i = int(currentTime);
		glUseProgram(render_program);
		// glUniformSubroutinesuiv �����ӳ���
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subroutines[i & 1]);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
	}

	virtual void shutdown()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteProgram(render_program);
	}

	void load_shaders();

protected:
	void onKey(int key, int scancode, int action, int mods)
	{
		if (action)
		{
			switch (key)
			{
			case 'R':
				load_shaders();
				break;
			}
		}
		OpenGLApp::onKey(key, scancode, action, mods);
	}

	GLuint          vao;
	GLuint			render_program;
	GLuint			subroutines[2];

	struct
	{
		GLint subroutine1;
	} uniforms;
};

void subroutines_app::load_shaders()
{
	if (render_program)
		glDeleteProgram(render_program);

	// glGetSubroutineIndex ��ȡ�ӳ��������
	// glGetSubroutineUniformLocation ��ȡ�ӳ����λ��
	render_program = LoadShadersVF("../media/glsl/06_subroutines.vs", "../media/glsl/06_subroutines.fs");
	subroutines[0] = glGetSubroutineIndex(render_program, GL_FRAGMENT_SHADER, "myFunction1");
	subroutines[1] = glGetSubroutineIndex(render_program, GL_FRAGMENT_SHADER, "myFunction2");
	uniforms.subroutine1 = glGetSubroutineUniformLocation(render_program, GL_FRAGMENT_SHADER, "mySubroutineUniform");
}

int main(int argc, char** argv)
{
	subroutines_app a;
	a.run();
	return 0;
}