#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/** \addtogroup Chapter05*   @{*       \addtogroup tunnel* 通过移动纹理产生隧道中行进的错觉*       @{*/

class tunnel_app : public OpenGLApp
{
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Tunnel";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));

	}

	void startup()
	{
		char buffer[1024];

		render_prog = LoadShadersVF("../media/glsl/05_tunnel.vs", "../media/glsl/05_tunnel.fs");

		uniforms.mvp = glGetUniformLocation(render_prog, "mvp");
		uniforms.offset = glGetUniformLocation(render_prog, "offset");

		glGenVertexArrays(1, &render_vao);
		glBindVertexArray(render_vao);

		tex_wall = loadKTX("../media/textures/brick.ktx", 0);
		tex_ceiling = loadKTX("../media/textures/ceiling.ktx", 0);
		tex_floor = loadKTX("../media/textures/floor.ktx", 0);

		int i;
		GLuint textures[] = { tex_floor, tex_wall, tex_ceiling };

		for (i = 0; i < 3; i++)
		{
			glBindTexture(GL_TEXTURE_2D, textures[i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}

		glBindVertexArray(render_vao);
	}

	void render(double currentTime)
	{
		static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		float t = (float)currentTime;

		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glClearBufferfv(GL_COLOR, 0, black);

		glUseProgram(render_prog);
		
		glm::mat4 proj_matrix = glm::perspective(glm::radians(60.0f), (float)info.windowWidth / (float)info.windowHeight, 0.1f, 100.0f);
		glUniform1f(uniforms.offset, t * 0.003f);
		
		int i;
		GLuint textures[] = { tex_wall, tex_floor, tex_wall, tex_ceiling };
		for (i = 0; i < 4; i++)
		{
			glm::mat4 mv_matrix = glm::rotate(glm::mat4(1.0), glm::radians(90.0f * (float)i), glm::vec3(0.0f, 0.0f, 1.0f));
			mv_matrix = glm::translate(mv_matrix, glm::vec3(-0.5f, 0.0f, -10.0f));
			mv_matrix = glm::rotate(mv_matrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			mv_matrix = glm::scale(mv_matrix, glm::vec3(30.0f, 1.0f, 1.0f));
			glm::mat4 mvp = proj_matrix * mv_matrix;

			glUniformMatrix4fv(uniforms.mvp, 1, GL_FALSE, &mvp[0][0]);

			glBindTexture(GL_TEXTURE_2D, textures[i]);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
		
	}

protected:
	GLuint          render_prog;
	GLuint          render_vao;
	struct
	{
		GLint       mvp;
		GLint       offset;
	} uniforms;

	GLuint          tex_wall;
	GLuint          tex_ceiling;
	GLuint          tex_floor;
};

/** @} @} */

int main(int argc, char** argv)
{
	tunnel_app a;
	a.run();
	return 0;
}