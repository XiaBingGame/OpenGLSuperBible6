#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/** \addtogroup Chapter03*   @{*       \addtogroup tessellatedtri* 使用 Tessellate 着色器.*	- glDrawArrays() 及 GL_PATCHES 参数绘制用于 tessellate 的 patch.*	- 顶点着色器中写好了三个顶点的位置, 使用 gl_VertexID 分派.*	- tcs 中使用 layout 设置输出顶点为 3 个的输出, gl_InvocationID 判断是 patch 的哪个顶点, 设置 gl_TessLevelInner 和 gl_TessLevelOuter 两个因子, 输入和输出 gl_in 和 gl_out*	- tes 中使用 layout 设置三角形, 相同空间, 顺时针的输入. gl_TessCoord 表示当前顶点的重心坐标, gl_in 数组表示其可以访问的周围顶点. 输出.* *       @{*/

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