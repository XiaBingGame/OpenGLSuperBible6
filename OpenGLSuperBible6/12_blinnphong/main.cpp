#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define MANY_OBJECTS

class blinnphong_app : public OpenGLApp
{
public:
	blinnphong_app() :
	  per_fragment_program(0) {}
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Blinn-Phong Shading";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		load_shaders();

		glGenBuffers(1, &uniforms_buffer);
		glBindBuffer(GL_UNIFORM_BUFFER, uniforms_buffer);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(uniforms_block), NULL, GL_DYNAMIC_DRAW);

		object.load("../media/models/sphere.sbm");

		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	}

	virtual void render(double currentTime)
	{
		static const GLfloat zeros[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		static const GLfloat gray[] = { 0.1f, 0.1f, 0.1f, 0.0f };
		static const GLfloat ones[] = { 1.0f };
		const float f = (float)currentTime;

		glUseProgram(per_fragment_program);
		glViewport(0, 0, info.windowWidth, info.windowHeight);

		glClearBufferfv(GL_COLOR, 0, gray);
		glClearBufferfv(GL_DEPTH, 0, ones);

		
		glm::vec3 view_position = glm::vec3(0.0f, 0.0f, 50.0f);
		glm::mat4 view_matrix = glm::lookAt(view_position,
												glm::vec3(0.0f, 0.0f, 0.0f),
												glm::vec3(0.0f, 1.0f, 0.0f));

		glm::vec3 light_position = glm::vec3(-20.0f, -20.0f, 0.0f);

		glm::mat4 light_proj_matrix = glm::frustum(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 200.0f);
		glm::mat4 light_view_matrix = glm::lookAt(light_position,
													  glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	#if defined(MANY_OBJECTS)
		int i, j;

		for (j = 0; j < 7; j++)
		{
			for (i = 0; i < 7; i++)
			{
				glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniforms_buffer);
				uniforms_block * block = (uniforms_block *)glMapBufferRange(GL_UNIFORM_BUFFER,
																			0,
																			sizeof(uniforms_block),
																			GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

				glm::mat4 model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3((float)i * 2.75f - 8.25f, 6.75f - (float)j * 2.25f, 0.0f));

				mat4copy(block->mv_matrix, view_matrix * model_matrix);
				mat4copy(block->view_matrix, view_matrix);
				mat4copy(block->proj_matrix, glm::perspective(glm::radians(50.0f),
														(float)info.windowWidth / (float)info.windowHeight,
														0.1f,
														1000.0f));

				glUnmapBuffer(GL_UNIFORM_BUFFER);

				glUniform1f(uniforms[0].specular_power, powf(2.0f, (float)j + 2.0f));
				glUniform3fv(uniforms[0].specular_albedo, 1, glm::value_ptr(glm::vec3((float)i / 9.0f + 1.0f / 9.0f)));

				object.render();
			}
		}
	#else
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniforms_buffer);
		uniforms_block * block = (uniforms_block *)glMapBufferRange(GL_UNIFORM_BUFFER,
																	0,
																	sizeof(uniforms_block),
																	GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

		glm::mat4 model_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(7.0f));

		mat4copy(block->mv_matrix, view_matrix * model_matrix);
		mat4copy(block->view_matrix, view_matrix);
		mat4copy(block->proj_matrix, glm::perspective(glm::radians(50.0f),
												(float)info.windowWidth / (float)info.windowHeight,
												0.1f,
												1000.0f));

		glUnmapBuffer(GL_UNIFORM_BUFFER);

		glUniform1f(uniforms[0].specular_power, 30.0f);
		glUniform3fv(uniforms[0].specular_albedo, 1, glm::value_ptr(glm::vec3(1.0f)));

		object.render();
	#endif
	}

	virtual void shutdown()
	{
		glDeleteProgram(per_fragment_program);
	}

	void onKey(int key, int scancode, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			switch(key)
			{
			case 'R': 
				load_shaders();
				break;
			default:
				break;
			}
		}
		OpenGLApp::onKey(key, scancode, action, mods);
	}

private:
	void load_shaders();

protected:
	GLuint          per_fragment_program;

	struct
	{
		GLuint      color;
		GLuint      normals;
	} textures;

	struct uniforms_block
	{
		mmat4     mv_matrix;
		mmat4     view_matrix;
		mmat4     proj_matrix;
	};

	GLuint          uniforms_buffer;

	struct
	{
		GLint           diffuse_albedo;
		GLint           specular_albedo;
		GLint           specular_power;
	} uniforms[2];

	sb6::object     object;
};

void blinnphong_app::load_shaders()
{
	if (per_fragment_program)
		glDeleteProgram(per_fragment_program);

	per_fragment_program = LoadShadersVF("../media/glsl/12_blinnphong/blinnphong.vs", "../media/glsl/12_blinnphong/blinnphong.fs");

	uniforms[0].diffuse_albedo = glGetUniformLocation(per_fragment_program, "diffuse_albedo");
	uniforms[0].specular_albedo = glGetUniformLocation(per_fragment_program, "specular_albedo");
	uniforms[0].specular_power = glGetUniformLocation(per_fragment_program, "specular_power");
}

int main(int argc, char** argv)
{
	blinnphong_app a;
	a.run();
	return 0;
}