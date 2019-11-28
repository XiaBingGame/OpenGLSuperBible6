#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/** \addtogroup Chapter03*   @{*       \addtogroup tessellatedtri* ʹ�� Tessellate ��ɫ��.*	- glDrawArrays() �� GL_PATCHES ������������ tessellate �� patch.*	- ������ɫ����д�������������λ��, ʹ�� gl_VertexID ����.*	- tcs ��ʹ�� layout �����������Ϊ 3 �������, gl_InvocationID �ж��� patch ���ĸ�����, ���� gl_TessLevelInner �� gl_TessLevelOuter ��������, �������� gl_in �� gl_out*	- tes ��ʹ�� layout ����������, ��ͬ�ռ�, ˳ʱ�������. gl_TessCoord ��ʾ��ǰ�������������, gl_in �����ʾ����Է��ʵ���Χ����. ���.* *       @{*/

class tessellatedtri_app : public OpenGLApp
{
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Tessellated Triangle";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}

	virtual void startup() 
	{
		program = LoadShadersVTF("../media/glsl/03_tessellatedtri.vs", 
			"../media/glsl/03_tessellatedtri.cs", 
			"../media/glsl/03_tessellatedtri.es", 
			"../media/glsl/03_tessellatedtri.fs");
		
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	virtual void render(double currentTime)
	{
		static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, green);

		glUseProgram(program);
		glDrawArrays(GL_PATCHES, 0, 3);
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
	tessellatedtri_app a;
	a.run();
	return 0;
}