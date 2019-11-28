#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/** \addtogroup Chapter03
*   @{
*       \addtogroup tessellatedgstri
* ʹ�� Tessellate �� Geometry ��ɫ��.
*	- glDrawArrays() �� GL_PATCHES ������������ tessellate �� patch.
*	- ������ɫ����д�������������λ��, ʹ�� gl_VertexID ����.
*	- tcs ��ʹ�� layout �����������Ϊ 3 �������, gl_InvocationID �ж��� patch ���ĸ�����, ���� gl_TessLevelInner �� gl_TessLevelOuter ��������, �������� gl_in �� gl_out
*	- tes ��ʹ�� layout ����������, ��ͬ�ռ�, ˳ʱ�������. gl_TessCoord ��ʾ��ǰ�������������, gl_in �����ʾ����Է��ʵ���Χ����. ���.
*	- gs ��ʹ�� layout ���� triangle ����, �����, �������Ϊ3. gl_in ����Ϊ���Է��ʵĶ���, gl_Position Ϊ���, EmitVertex() �������
*	- ʹ�ü�����ɫ���ı������������Ϊ�����. 
*
*       @{
*/

class tessellatedgstri_app : public OpenGLApp
{
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Tessellated Triangle";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}

	virtual void startup()
	{
		program = LoadShadersVTGF("../media/glsl/03_tessellatedgstri.vs", 
			"../media/glsl/03_tessellatedgstri.cs", 
			"../media/glsl/03_tessellatedgstri.es", 
			"../media/glsl/03_tessellatedgstri.gs", 
			"../media/glsl/03_tessellatedgstri.fs");

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	virtual void render(double currentTime)
	{
		static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, green);

		glPointSize(5.0f);
		glUseProgram(program);
		glDrawArrays(GL_PATCHES, 0, 3);
		glPointSize(1.0f);
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
	tessellatedgstri_app a;
	a.run();
	return 0;
}