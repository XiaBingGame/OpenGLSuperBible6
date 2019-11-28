#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/** \addtogroup Chapter08*   @{*       \addtogroup dispmap* ʹ�� TCS, TES, displacement mapping �����Ƶ���, ÿ����θ��ݵ���Ļ�ľ����ÿһ�߽����ʵ��� tessellate.**	- ���η�Ϊ 64x64, ÿһ����ݵ��۾��ľ�����в�ͬ�� tessellation*	- ������ɫ������ʵ��ID����λ�ú�����.*	- tcs ���������� varying ��Ϊ������ʽ*	- tcs ���ȼ����ĸ��ǵ��豸��Ԫ����. ����õ��� tessellation factor*	- tes ���� displacement map �����������εĸ߶�*	- ������ɫ����ȡ������ɫ, ������Ч��*       @{*/

class dispmap_app : public OpenGLApp
{
public:
	dispmap_app() : program(0), enable_displacement(true),
		wireframe(false), enable_fog(true), paused(false) {}

	void init()
	{
		static const char title[] = "OpenGL";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		load_shaders();	
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glPatchParameteri(GL_PATCH_VERTICES, 4);
		glEnable(GL_CULL_FACE);

		tex_displacement = loadKTX("../media/textures/terragen1.ktx");
		glActiveTexture(GL_TEXTURE1);
		tex_color = loadKTX("../media/textures/terragen_color.ktx");
	}

	virtual void render(double currentTime)
	{
		static const GLfloat black[] = { 0.85f, 0.95f, 1.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, black);
		static const GLfloat one = 1.0f;
		glClearBufferfv(GL_DEPTH, 0, &one);

		static double last_time = currentTime;
		static double total_time = 0.0;
		if(!paused)
			total_time += (currentTime - last_time);
		last_time = currentTime;

		float t = (float)total_time * 0.03f;
		float r = sinf(t * 5.37f) * 15.0f + 16.0f;
		float h = cosf(t * 4.79f) * 2.0f + 3.2f;

		glViewport(0, 0, info.windowWidth, info.windowHeight);

		glm::mat4 mv_matrix = glm::lookAt(glm::vec3(sinf(t) * r, h, cosf(t) * r), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 proj_matrix = glm::perspective(glm::radians(60.0f), (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);
		glm::mat4 mvp_matrix = proj_matrix * mv_matrix;

		glUseProgram(program);

		glUniformMatrix4fv(uniforms.mv_matrix, 1, GL_FALSE, glm::value_ptr(mv_matrix));
		glUniformMatrix4fv(uniforms.proj_matrix, 1, GL_FALSE, glm::value_ptr(proj_matrix));
		glUniformMatrix4fv(uniforms.mvp_matrix, 1, GL_FALSE, glm::value_ptr(proj_matrix*mv_matrix));
		glUniform1f(uniforms.dmap_depth, enable_displacement ? dmap_depth : 0.0f);
		glUniform1i(uniforms.enable_fog, enable_fog ? 1 : 0);

		if(wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glDrawArraysInstanced(GL_PATCHES, 0, 4, 64 * 64);
	}

	virtual void shutdown()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteProgram(program);
	}

protected:
	void onKey(int key, int scancode, int action, int mods);

private:
	void load_shaders();

protected:
	GLuint          program;
	GLuint          vao;

	GLuint			tex_displacement;
	GLuint			tex_color;
	float			dmap_depth;
	bool				enable_displacement;
	bool				wireframe;
	bool				enable_fog;
	bool				paused;

	struct 
	{
		GLuint		mvp_matrix;
		GLuint		mv_matrix;
		GLuint		proj_matrix;
		GLuint		dmap_depth;
		GLuint		enable_fog;
	} uniforms;
};

void dispmap_app::load_shaders()
{
	if(program)
		glDeleteProgram(program);
	program = LoadShadersVTF("../media/glsl/08_dispmap/dispmap.vs", "../media/glsl/08_dispmap/dispmap.tcs",
							"../media/glsl/08_dispmap/dispmap.tes", "../media/glsl/08_dispmap/dispmap.fs");

	uniforms.mv_matrix = glGetUniformLocation(program, "mv_matrix");
	uniforms.mvp_matrix = glGetUniformLocation(program, "mvp_matrix");
	uniforms.proj_matrix = glGetUniformLocation(program, "proj_matrix");
	uniforms.dmap_depth = glGetUniformLocation(program, "dmap_depth");
	uniforms.enable_fog = glGetUniformLocation(program, "enable_fog");

	dmap_depth = 6.0f;
}

void dispmap_app::onKey(int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		switch(key)
		{
		case GLFW_KEY_KP_ADD:
			dmap_depth += 0.1f;
			break;
		case GLFW_KEY_KP_SUBTRACT:
			dmap_depth -= 0.1f;
			break;
		case 'F':
			enable_fog = !enable_fog;
			break;
		case 'D':
			enable_displacement = !enable_displacement;
			break;
		case 'W':
			wireframe = !wireframe;
			break;
		case 'P':
			paused = !paused;
			break;
		case 'R':
			load_shaders();
			break;
		default:
			break;
		}
	}
	OpenGLApp::onKey(key, scancode, action, mods);
}

/** @} @} */

int main(int argc, char** argv)
{
	dispmap_app a;
	a.run();
	return 0;
}