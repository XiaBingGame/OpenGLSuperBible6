#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define NUM_PASSES 200
#define NUM_CUBES 256

class multimaterial_app : public OpenGLApp
{
public:
	multimaterial_app() :
		  mode(BIG_UBO_WITH_UNIFORM),
		  draw_triangles(true),
		  paused(false),
		  simple_uniform_program(0),
		  ubo_plus_base_instance_program(0),
		  ubo_plus_uniform_program(0)
	  {

	  }

	struct transform_t
	{
		mmat4 mv_matrix;
		mmat4 proj_matrix;
	};

	transform_t transforms[NUM_CUBES];

	void init()
	{
		static const char title[] = "OpenGL SuperBible";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		load_shaders();

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		static const GLfloat vertex_positions[] =
		{
			-0.25f,  0.25f, -0.25f,
			-0.25f, -0.25f, -0.25f,
			0.25f, -0.25f, -0.25f,

			0.25f, -0.25f, -0.25f,
			0.25f,  0.25f, -0.25f,
			-0.25f,  0.25f, -0.25f,

			0.25f, -0.25f, -0.25f,
			0.25f, -0.25f,  0.25f,
			0.25f,  0.25f, -0.25f,

			0.25f, -0.25f,  0.25f,
			0.25f,  0.25f,  0.25f,
			0.25f,  0.25f, -0.25f,

			0.25f, -0.25f,  0.25f,
			-0.25f, -0.25f,  0.25f,
			0.25f,  0.25f,  0.25f,

			-0.25f, -0.25f,  0.25f,
			-0.25f,  0.25f,  0.25f,
			0.25f,  0.25f,  0.25f,

			-0.25f, -0.25f,  0.25f,
			-0.25f, -0.25f, -0.25f,
			-0.25f,  0.25f,  0.25f,

			-0.25f, -0.25f, -0.25f,
			-0.25f,  0.25f, -0.25f,
			-0.25f,  0.25f,  0.25f,

			-0.25f, -0.25f,  0.25f,
			0.25f, -0.25f,  0.25f,
			0.25f, -0.25f, -0.25f,

			0.25f, -0.25f, -0.25f,
			-0.25f, -0.25f, -0.25f,
			-0.25f, -0.25f,  0.25f,

			-0.25f,  0.25f, -0.25f,
			0.25f,  0.25f, -0.25f,
			0.25f,  0.25f,  0.25f,

			0.25f,  0.25f,  0.25f,
			-0.25f,  0.25f,  0.25f,
			-0.25f,  0.25f, -0.25f
		};

		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER,
			sizeof(vertex_positions),
			vertex_positions,
			GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);

		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CW);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glGenBuffers(1, &transform_ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, transform_ubo);
		glBufferData(GL_UNIFORM_BUFFER, 256 * sizeof(transform_t), NULL, GL_DYNAMIC_DRAW);

		struct draw_indirect_cmd_t
		{
			unsigned int count;
			unsigned int primCount;
			unsigned int first;
			unsigned int baseInstance;
		};

		glGenBuffers(1, &indirect_buffer);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_buffer);
		glBufferData(GL_DRAW_INDIRECT_BUFFER, NUM_CUBES * sizeof(draw_indirect_cmd_t), NULL, GL_STATIC_DRAW);
		draw_indirect_cmd_t * ind = (draw_indirect_cmd_t *)glMapBufferRange(GL_DRAW_INDIRECT_BUFFER, 0, NUM_CUBES * sizeof(draw_indirect_cmd_t), GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_WRITE_BIT);
		int i;
		for (i = 0; i < NUM_CUBES; i++)
		{
			ind[i].count = 36;
			ind[i].primCount = 1;
			ind[i].first = 0;
			ind[i].baseInstance = i;
		}
		glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
	}

	virtual void render(double currentTime)
	{
		static double last_time = 0.0;
		static double total_time = 0.0;
		static unsigned int q = 0;

		if (!paused)
			total_time += (currentTime - last_time);
		last_time = currentTime;

		switch (mode)
		{
		case SIMPLE_UNIFORM:
			render_simple_uniform(total_time);
			break;
		case BIG_UBO_WITH_UNIFORM:
			render_big_ubo_plus_uniform(total_time);
			break;
		case BIG_UBO_WITH_BASEVERTEX:
			render_big_ubo_base_vertex(total_time);
			break;
		case BIG_UBO_WITH_INSTANCING:
			render_big_ubo_with_instancing(total_time);
			break;
		case BIG_UBO_INDIRECT:
			render_big_ubo_indirect(total_time);
			break;
		};
	}

	void render_simple_uniform(double currentTime)
    {
        static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
        static const GLfloat one = 1.0f;
        float f = (float)currentTime * 0.3f;
        int i;
        int j;

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, green);
        glClearBufferfv(GL_DEPTH, 0, &one);

        glUseProgram(simple_uniform_program);

        for (i = 0; i < NUM_CUBES; i++)
        {
            float fi = 4.0f * (float)i / (float)NUM_CUBES;
            fi = 0.0f;
            mat4copy(transforms[i].proj_matrix, glm::perspective(glm::radians(50.0f),
                                                           (float)info.windowWidth / (float)info.windowHeight,
                                                           0.1f,
                                                           1000.0f));
            mat4copy(transforms[i].mv_matrix, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f)) *
                                      glm::translate(glm::mat4(1.0f), glm::vec3(sinf(5.1f * f + fi) * 1.0f,
                                                       cosf(7.7f * f + fi) * 1.0f,
                                                       sinf(6.3f * f + fi) * cosf(1.5f * f + fi) * 2.0f)) *
                                      glm::rotate(glm::mat4(1.0f), glm::radians(f * 45.0f + fi), glm::vec3(0.0f, 1.0f, 0.0f)) *
                                      glm::rotate(glm::mat4(1.0f), glm::radians(f * 81.0f + fi), glm::vec3(1.0f, 0.0f, 0.0f)));
        }

        for (j = 0; j < NUM_PASSES; j++)
        {
            for (i = 0; i < NUM_CUBES; i++)
            {
                glUniformMatrix4fv(uniforms.simple_uniforms.proj_location, 1, GL_FALSE, transforms[i].proj_matrix);
                glUniformMatrix4fv(uniforms.simple_uniforms.mv_location, 1, GL_FALSE, transforms[i].mv_matrix);
                glDrawArraysInstancedBaseInstance(draw_triangles ? GL_TRIANGLES : GL_POINTS, 0, 36, 1, 0);
            }
        }
    }

    void render_big_ubo_plus_uniform(double currentTime)
    {
		static const GLfloat blue[] = { 0.0f, 0.0f, 0.25f, 1.0f };
		static const GLfloat one = 1.0f;
		float f = (float)currentTime * 0.3f;
		int i;
		int j;

		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glClearBufferfv(GL_COLOR, 0, blue);
		glClearBufferfv(GL_DEPTH, 0, &one);

		glUseProgram(ubo_plus_uniform_program);

		glBindBufferBase(GL_UNIFORM_BUFFER, 0, transform_ubo);
		transform_t * transform = (transform_t *)glMapBufferRange(GL_UNIFORM_BUFFER, 0, NUM_CUBES * sizeof(transform_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

		for (i = 0; i < NUM_CUBES; i++)
		{
			float fi = 4.0f * (float)i / (float)NUM_CUBES;
			fi = 0.0f;
			mat4copy(transform[i].proj_matrix, glm::perspective(50.0f,
				(float)info.windowWidth / (float)info.windowHeight,
				0.1f,
				1000.0f));
			mat4copy(transform[i].mv_matrix, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f)) *
				glm::translate(glm::mat4(1.0f), glm::vec3(sinf(5.1f * f + fi) * 1.0f,
				cosf(7.7f * f + fi) * 1.0f,
				sinf(6.3f * f + fi) * cosf(1.5f * f + fi) * 2.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(f * 45.0f + fi), glm::vec3(0.0f, 1.0f, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(f * 81.0f + fi), glm::vec3(1.0f, 0.0f, 0.0f)));
		}
		glUnmapBuffer(GL_UNIFORM_BUFFER);

		for (j = 0; j < NUM_PASSES; j++)
		{
			for (i = 0; i < NUM_CUBES; i++)
			{
				glUniform1i(uniforms.ubo_plus_uniform.transform_index, i);
				glDrawArraysInstancedBaseInstance(draw_triangles ? GL_TRIANGLES : GL_POINTS, 0, 36, 1, 0);
			}
		}
    }

    void render_big_ubo_base_vertex(double currentTime)
    {
		static const GLfloat red[] = { 0.25f, 0.0f, 0.0f, 1.0f };
		static const GLfloat one = 1.0f;
		int i;
		int j;
		float f = (float)currentTime * 0.3f;

		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glClearBufferfv(GL_COLOR, 0, red);
		glClearBufferfv(GL_DEPTH, 0, &one);

		glUseProgram(ubo_plus_base_instance_program);

		glBindBufferBase(GL_UNIFORM_BUFFER, 0, transform_ubo);
		transform_t * transform = (transform_t *)glMapBufferRange(GL_UNIFORM_BUFFER, 0, NUM_CUBES * sizeof(transform_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

		for (i = 0; i < NUM_CUBES; i++)
		{
			float fi = 4.0f * (float)i / (float)NUM_CUBES;
			fi = 0.0f;
			mat4copy(transform[i].proj_matrix, glm::perspective(glm::radians(50.0f),
				(float)info.windowWidth / (float)info.windowHeight,
				0.1f,
				1000.0f));
			mat4copy(transform[i].mv_matrix, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f)) *
				glm::translate(glm::mat4(1.0f), glm::vec3(sinf(5.1f * f + fi) * 1.0f,
				cosf(7.7f * f + fi) * 1.0f,
				sinf(6.3f * f + fi) * cosf(1.5f * f + fi) * 2.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(f * 45.0f + fi), glm::vec3(0.0f, 1.0f, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(f * 81.0f + fi), glm::vec3(1.0f, 0.0f, 0.0f)));
		}
		glUnmapBuffer(GL_UNIFORM_BUFFER);

		for (j = 0; j < NUM_PASSES; j++)
		{
			for (i = 0; i < NUM_CUBES; i++)
			{
				glDrawArraysInstancedBaseInstance(draw_triangles ? GL_TRIANGLES : GL_POINTS, 0, 36, 1, i);
			}
		}
    }

    void render_big_ubo_with_instancing(double currentTime)
    {
        static const GLfloat yellow[] = { 0.25f, 0.25f, 0.0f, 1.0f };
        static const GLfloat one = 1.0f;
        int i;
        int j;
        float f = (float)currentTime * 0.3f;

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, yellow);
        glClearBufferfv(GL_DEPTH, 0, &one);

        glUseProgram(ubo_plus_base_instance_program);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, transform_ubo);
        transform_t * transform = (transform_t *)glMapBufferRange(GL_UNIFORM_BUFFER, 0, NUM_CUBES * sizeof(transform_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        for (i = 0; i < NUM_CUBES; i++)
        {
            float fi = 4.0f * (float)i / (float)NUM_CUBES;
            fi = 0.0f;
			mat4copy(transforms[i].proj_matrix, glm::perspective(glm::radians(50.0f),
				(float)info.windowWidth / (float)info.windowHeight,
				0.1f,
				1000.0f));
			mat4copy(transforms[i].mv_matrix, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f)) *
				glm::translate(glm::mat4(1.0f), glm::vec3(sinf(5.1f * f + fi) * 1.0f,
				cosf(7.7f * f + fi) * 1.0f,
				sinf(6.3f * f + fi) * cosf(1.5f * f + fi) * 2.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(f * 45.0f + fi), glm::vec3(0.0f, 1.0f, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(f * 81.0f + fi), glm::vec3(1.0f, 0.0f, 0.0f)));
        }
        glUnmapBuffer(GL_UNIFORM_BUFFER);

        for (j = 0; j < NUM_PASSES; j++)
        {
            glDrawArraysInstancedBaseInstance(draw_triangles ? GL_TRIANGLES : GL_POINTS, 0, 36, NUM_CUBES, 0);
        }
    }

    void render_big_ubo_indirect(double currentTime)
    {
        static const GLfloat purple[] = { 0.25f, 0.0f, 0.25f, 1.0f };
        static const GLfloat one = 1.0f;
        int i;
        int j;
        float f = (float)currentTime * 0.3f;

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, purple);
        glClearBufferfv(GL_DEPTH, 0, &one);

        glUseProgram(ubo_plus_base_instance_program);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, transform_ubo);
        transform_t * transform = (transform_t *)glMapBufferRange(GL_UNIFORM_BUFFER, 0, NUM_CUBES * sizeof(transform_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        for (i = 0; i < NUM_CUBES; i++)
        {
            float fi = 4.0f * (float)i / (float)NUM_CUBES;
            fi = 0.0f;
			mat4copy(transforms[i].proj_matrix, glm::perspective(glm::radians(50.0f),
				(float)info.windowWidth / (float)info.windowHeight,
				0.1f,
				1000.0f));
			mat4copy(transforms[i].mv_matrix, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f)) *
				glm::translate(glm::mat4(1.0f), glm::vec3(sinf(5.1f * f + fi) * 1.0f,
				cosf(7.7f * f + fi) * 1.0f,
				sinf(6.3f * f + fi) * cosf(1.5f * f + fi) * 2.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(f * 45.0f + fi), glm::vec3(0.0f, 1.0f, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(f * 81.0f + fi), glm::vec3(1.0f, 0.0f, 0.0f)));
		}
        glUnmapBuffer(GL_UNIFORM_BUFFER);

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_buffer);

        for (j = 0; j < NUM_PASSES; j++)
        {
            glMultiDrawArraysIndirect(draw_triangles ? GL_TRIANGLES : GL_POINTS, NULL, NUM_CUBES, 0);
            /*
            for (i = 0; i < NUM_CUBES; i++)
            {
                glDrawArraysInstancedBaseInstance(draw_triangles ? GL_TRIANGLES : GL_POINTS, 0, 36, 1, i);
            }
            */
        }
    }

	virtual void shutdown()
	{
		if(simple_uniform_program)
			glDeleteProgram(simple_uniform_program);

		if (ubo_plus_uniform_program)
			glDeleteProgram(ubo_plus_uniform_program);

		if(ubo_plus_base_instance_program)
			glDeleteProgram(ubo_plus_base_instance_program);
	}

	void onKey(int key, int scancode, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			switch(key)
			{
			case 'P':
				paused = !paused;
				break;
			case 'T':
				draw_triangles = !draw_triangles;
				break;
			case 'M':
				mode = (MODE)(mode + 1);
				if (mode > BIG_UBO_INDIRECT)
					mode = SIMPLE_UNIFORM;
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
	GLuint          simple_uniform_program;
	GLuint          ubo_plus_uniform_program;
	GLuint          ubo_plus_base_instance_program;
	GLuint          vao;
	GLuint          buffer;
	struct
	{
		struct
		{
			GLint           mv_location;
			GLint           proj_location;
		} simple_uniforms;
		struct
		{
			GLint           transform_index;
		} ubo_plus_uniform;
	} uniforms;

	GLuint          transform_ubo;
	GLuint          indirect_buffer;

	enum MODE
	{
		SIMPLE_UNIFORM,
		BIG_UBO_WITH_UNIFORM,
		BIG_UBO_WITH_BASEVERTEX,
		BIG_UBO_WITH_INSTANCING,
		BIG_UBO_INDIRECT,
	} mode;
	bool            draw_triangles;
	bool            paused;
};

void multimaterial_app::load_shaders()
{
	if(simple_uniform_program)
		glDeleteProgram(simple_uniform_program);

	if (ubo_plus_uniform_program)
		glDeleteProgram(ubo_plus_uniform_program);

	if(ubo_plus_base_instance_program)
		glDeleteProgram(ubo_plus_base_instance_program);

	simple_uniform_program = LoadShadersVF("../media/glsl/12_multimaterial/simpleuniforms.vs", "../media/glsl/12_multimaterial/simpleuniforms.fs");
	ubo_plus_uniform_program = LoadShadersVF("../media/glsl/12_multimaterial/ubo-plus-uniform.vs", "../media/glsl/12_multimaterial/ubo-plus-uniform.fs");
	ubo_plus_base_instance_program = LoadShadersVF("../media/glsl/12_multimaterial/ubo-plus-base-instance.vs", "../media/glsl/12_multimaterial/ubo-plus-base-instance.fs");

	uniforms.simple_uniforms.mv_location = glGetUniformLocation(simple_uniform_program, "mv_matrix");
	uniforms.simple_uniforms.proj_location = glGetUniformLocation(simple_uniform_program, "proj_matrix");
	uniforms.ubo_plus_uniform.transform_index = glGetUniformLocation(ubo_plus_uniform_program, "transform_index");
}

int main(int argc, char** argv)
{
	multimaterial_app a;
	a.run();
	return 0;
}