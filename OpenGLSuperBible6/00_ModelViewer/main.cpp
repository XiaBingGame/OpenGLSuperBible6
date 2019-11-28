/************************************************************************/
// 可以实现代码运行的过程中修改着色器. 
// 这里模型加载用了自己的 VAO
/************************************************************************/
#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"

#include <glm/gtc/matrix_transform.hpp>


class modelviewer_app : public OpenGLApp
{
public:
	modelviewer_app()
		: render_prog(0)
	{
	}

protected:
	void init()
	{
		static const char title[] = "OpenGL SuperBible - ModelViewer";

		OpenGLApp::init();

		memcpy(info.title, title, sizeof(title));
	}

	virtual void startup()
	{
		//object.load("../media/models/dragon.sbm");
		object.load("../media/models/asteroids.sbm");

		load_shaders();

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	virtual void render(double currentTime)
	{
		static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		static const GLfloat ones[] = { 1.0f };
		glm::vec3 lightdirection = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));

		float distance = 10.0f;

		glClearBufferfv(GL_COLOR, 0, black);
		glClearBufferfv(GL_DEPTH, 0, ones);

		glViewport(0, 0, info.windowWidth, info.windowHeight);

		glUseProgram(render_prog);

		glm::mat4 proj_matrix = glm::perspective(glm::radians(60.0f), (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);
		glm::mat4 view_matrix = glm::lookAt(glm::vec3(0.0, distance, distance), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 model_matrix = glm::mat4(1.0f);
		model_matrix = glm::rotate(model_matrix, glm::radians((float)currentTime * 19.3f), glm::vec3(0.0f, 1.0f, 0.0f));
		//model_matrix = glm::rotate(model_matrix, glm::radians((float)currentTime * 21.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 mv_matrix = view_matrix * model_matrix;
		glm::mat4 mvproj_matrix = proj_matrix * view_matrix * model_matrix;

		glUniform3fv(uniforms.light_direction, 1, &lightdirection[0]);
		glUniformMatrix4fv(uniforms.mv_matrix, 1, GL_FALSE, &mv_matrix[0][0]);
		glUniformMatrix4fv(uniforms.mvproj_matrix, 1, GL_FALSE, &mvproj_matrix[0][0]);

		object.render();
		//object.render_sub_object(0, 1, 0);
		//object.render_sub_object(0, 1, 10);
	}

	virtual void shutdown()
	{
		glDeleteProgram(render_prog);
	}

	virtual void onKey(int key, int scancode, int action, int mods)
	{
		if (action)
		{
			switch (key)
			{
			case 'R': load_shaders();
				break;
			default:
				break;
			}
		}
		OpenGLApp::onKey(key, scancode, action, mods);
	}

protected:
	GLuint          render_prog;

	struct
	{
		GLuint       mv_matrix;
		GLuint		mvproj_matrix;
		GLuint		light_direction;
	} uniforms;

	sb6::object     object;

private:
	void load_shaders()
	{
		glDeleteProgram(render_prog);
		render_prog = LoadShadersVF("../media/glsl/00_modelviewer.vs", "../media/glsl/00_modelviewer.fs");
		uniforms.mv_matrix = glGetUniformLocation(render_prog, "mv_matrix");
		uniforms.mvproj_matrix = glGetUniformLocation(render_prog, "mvproj_matrix");
		uniforms.light_direction = glGetUniformLocation(render_prog, "light_direction");
	}
};
int main(int argc, char** argv)
{
	modelviewer_app a;
	a.run();
	return 0;
}