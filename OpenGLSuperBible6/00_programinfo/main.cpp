/************************************************************************/
// 可以获取着色器的信息, 主要是输入变量的布局.
/************************************************************************/
#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

static const struct type_to_name_entry
{
	GLenum type;
	const char * name;
}
type_to_name_table[] =
{
	GL_FLOAT,                       "float",
	GL_FLOAT_VEC2,                  "vec2",
	GL_FLOAT_VEC3,                  "vec3",
	GL_FLOAT_VEC4,                  "vec4",
	GL_DOUBLE,                      "double",
	GL_DOUBLE_VEC2,                 "dvec2",
	GL_DOUBLE_VEC3,                 "dvec3",
	GL_DOUBLE_VEC4,                 "dvec4",
	GL_INT,                         "int",
	GL_INT_VEC2,                    "ivec2",
	GL_INT_VEC3,                    "ivec3",
	GL_INT_VEC4,                    "ivec4",
	GL_UNSIGNED_INT,                "uint",
	GL_UNSIGNED_INT_VEC2,           "uvec2",
	GL_UNSIGNED_INT_VEC3,           "uvec3",
	GL_UNSIGNED_INT_VEC4,           "uvec4",
	GL_BOOL,                        "bool",
	GL_BOOL_VEC2,                   "bvec2",
	GL_BOOL_VEC3,                   "bvec3",
	GL_BOOL_VEC4,                   "bvec4",
	GL_NONE,                        NULL
};

static const char * type_to_name(GLenum type)
{
	for (const type_to_name_entry * ptr = &type_to_name_table[0]; ptr->name != NULL; ptr++)
	{
		if (ptr->type == type)
			return ptr->name;
	}

	return NULL;
}

class programinfo_app : public OpenGLApp
{
public:
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Program Information";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		program = LoadShadersVF("../media/glsl/00_programinfo.vs", "../media/glsl/00_programinfo.fs");

		GLint outputs;

		glGetProgramInterfaceiv(program, GL_PROGRAM_OUTPUT, GL_ACTIVE_RESOURCES, &outputs);

		static const GLenum props[] = { GL_TYPE, GL_LOCATION, GL_ARRAY_SIZE };
		static const char * prop_name[] = { "type", "location", "array size" };
		GLint i;
		GLint params[4];
		GLchar name[64];
		const char * type_name;
		char buffer[1024];

		glGetProgramInfoLog(program, sizeof(buffer), NULL, buffer);

		for (i = 0; i < outputs; i++)
		{
			glGetProgramResourceName(program, GL_PROGRAM_OUTPUT, i, sizeof(name), NULL, name);
			glGetProgramResourceiv(program, GL_PROGRAM_OUTPUT, i, 3, props, 3, NULL, params);
			type_name = type_to_name(params[0]);
			if (params[2] != 0)
			{
				sprintf(buffer, "Index %d: %s %s[%d] @ location %d.", i, type_name, name, params[2], params[1]);
				std::cout << buffer << std::endl;
			}
			else
			{
				sprintf(buffer, "Index %d: %s %s @ location %d.", i, type_name, name, params[1]);
				std::cout << buffer << std::endl;
			}
		}
	}

	virtual void render(double currentTime)
	{
		static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };

		glViewport(0, 0, info.windowWidth, info.windowHeight);
		glClearBufferfv(GL_COLOR, 0, black);
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

int main(int argc, char** argv)
{
	programinfo_app a;
	a.run();
	return 0;
}