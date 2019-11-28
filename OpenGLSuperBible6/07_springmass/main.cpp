#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/** \addtogroup Chapter07*   @{*       \addtogroup sprintmass* 使用 Transform Feedback 绘制, 并使用了 TBO 技术. 绘制了一个空中舞动的麻布.**	- glTransformFeedbackVaryings() 告知哪个着色器输出变量需要记录. 而后使用 glLinkProgram() 链接至程序*	- 创建 buffer, 而后绑定至 indexed transform feedback buffer binding point, buffer 类型为 GL_TARNSFORM_FEEDBACK_BUFFER*	- glEnable(GL_RASTERIZER_DISCARD); 可以禁止绘制任何东西.*	- 本例还使用了 Texture buffer, 创建了2个 GL_TEXTURE_BUFFER 纹理, 在 glTexBuffer 中设置格式为 GL_RGBA32F, 而后绑定至两个 VBO, 以便着色器中直接访问纹理来访问buffer内容.*	- 着色器: 更新部分 --- 根据重力, 空气阻力, 弹力计算出最终的作用力, 而后计算出新的位置与速度. 其最后结果写入两个 buffer 内.*       @{*/

enum BUFFER_TYPE_t
{
	POSITION_A,
	POSITION_B,
	VELOCITY_A,
	VELOCITY_B,
	CONNECTION
};

enum
{
	POINTS_X	 = 50,
	POINTS_Y = 50,
	POINTS_TOTAL = (POINTS_X * POINTS_Y),
	CONNECTIONS_TOTAL = (POINTS_X - 1) * POINTS_Y + (POINTS_Y - 1) * POINTS_X
};



class springmass_app : public OpenGLApp
{
public:
	typedef float myvec4[4];
	typedef float myvec3[3];
	typedef int myivec4[4];

	springmass_app()
		: m_iteration_index(0),
		  m_update_program(0),
		  m_render_program(0),
		  draw_points(true),
		  draw_lines(true),
		  iterations_per_frame(16)
	{}

	void init()
	{
		static const char title[] = "OpenGL SuperBible - Spring-Mass Simulator";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup()
	{
		int i, j;
		load_shaders();
		myvec4* initial_positions = new myvec4[POINTS_TOTAL];
		myvec3* initial_velocities = new myvec3[POINTS_TOTAL];
		myivec4* connection_vectors = new myivec4[POINTS_TOTAL];

		int n = 0;

		for (j = 0; j < POINTS_Y; j++)
		{
			float fj = (float)j / (float)POINTS_Y;
			for (i = 0; i < POINTS_X; i++)
			{
				float fi = (float)i / (float)POINTS_X;

				// 位置初始化为 -25 到 25
				// z 方向上下起伏
				// w 为质量
				initial_positions[n][0] = (fi - 0.5f) * (float)POINTS_X;
				initial_positions[n][1] = (fj - 0.5f) * (float)POINTS_Y;
				initial_positions[n][2] = 0.6f * sinf(fi) * cosf(fj);
				initial_positions[n][3] = 1.0f;

				initial_velocities[n][0] = 0.0f;
				initial_velocities[n][1] = 0.0f;
				initial_velocities[n][2] = 0.0f;

				connection_vectors[n][0] = -1;
				connection_vectors[n][1] = -1;
				connection_vectors[n][2] = -1;
				connection_vectors[n][3] = -1;

				// connection_vectors 保存横纵四个方向的相邻顶点的索引.
				if (j != (POINTS_Y - 1))
				{
					if (i != 0)
						connection_vectors[n][0] = n - 1;

					if (j != 0)
						connection_vectors[n][1] = n - POINTS_X;

					if (i != (POINTS_X - 1))
						connection_vectors[n][2] = n + 1;

					if (j != (POINTS_Y - 1))
						connection_vectors[n][3] = n + POINTS_X;
				}
				n++;
			}
		}

		glGenVertexArrays(2, m_vao);
		glGenBuffers(5, m_vbo);

		// 一共五个 buffer
		for (i = 0; i < 2; i++) {
			glBindVertexArray(m_vao[i]);

			glBindBuffer(GL_ARRAY_BUFFER, m_vbo[POSITION_A + i]);
			glBufferData(GL_ARRAY_BUFFER, POINTS_TOTAL * sizeof(myvec4), initial_positions, GL_DYNAMIC_COPY);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, m_vbo[VELOCITY_A + i]);
			glBufferData(GL_ARRAY_BUFFER, POINTS_TOTAL * sizeof(myvec3), initial_velocities, GL_DYNAMIC_COPY);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
			glEnableVertexAttribArray(1);

			glBindBuffer(GL_ARRAY_BUFFER, m_vbo[CONNECTION]);
			glBufferData(GL_ARRAY_BUFFER, POINTS_TOTAL * sizeof(myivec4), connection_vectors, GL_STATIC_DRAW);
			glVertexAttribIPointer(2, 4, GL_INT, 0, NULL);
			glEnableVertexAttribArray(2);
		}

		delete [] initial_positions;
		delete [] initial_velocities;
		delete [] connection_vectors;

		// 创建了2个 GL_TEXTURE_BUFFER 纹理, 在 glTexBuffer 中设置格式为 GL_RGBA32F, 分属两个 vbo
		glGenTextures(2, m_pos_tbo);
		glBindTexture(GL_TEXTURE_BUFFER, m_pos_tbo[0]);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, m_vbo[POSITION_A]);
		glBindTexture(GL_TEXTURE_BUFFER, m_pos_tbo[1]);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, m_vbo[POSITION_B]);

		int lines = (POINTS_X - 1) * POINTS_Y + (POINTS_Y - 1) * POINTS_X;

		// 设置所有线条的绘制索引.
		glGenBuffers(1, &m_index_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, lines * 2 * sizeof(int), NULL, GL_STATIC_DRAW);

		int * e = (int *)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, lines * 2 * sizeof(int), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

		for (j = 0; j < POINTS_Y; j++)  
		{
			for (i = 0; i < POINTS_X - 1; i++)
			{
				*e++ = i + j * POINTS_X;
				*e++ = 1 + i + j * POINTS_X;
			}
		}

		for (i = 0; i < POINTS_X; i++)
		{
			for (j = 0; j < POINTS_Y - 1; j++)
			{
				*e++ = i + j * POINTS_X;
				*e++ = POINTS_X + i + j * POINTS_X;
			}
		}

		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	}

	virtual void render(double currentTime)
	{
		int i;
		glUseProgram(m_update_program);

		glEnable(GL_RASTERIZER_DISCARD);

		for (i = iterations_per_frame; i != 0; --i)
		{
			glBindVertexArray(m_vao[m_iteration_index & 1]);
			glBindTexture(GL_TEXTURE_BUFFER, m_pos_tbo[m_iteration_index & 1]);
			m_iteration_index++;
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_vbo[POSITION_A + (m_iteration_index & 1)]);
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, m_vbo[VELOCITY_A + (m_iteration_index & 1)]);
			glBeginTransformFeedback(GL_POINTS);
			glDrawArrays(GL_POINTS, 0, POINTS_TOTAL);
			glEndTransformFeedback();
		}

		glDisable(GL_RASTERIZER_DISCARD);

		static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };

		glClearBufferfv(GL_COLOR, 0, black);

		glUseProgram(m_render_program);

		if (draw_points)
		{
			glPointSize(4.0f);
			glDrawArrays(GL_POINTS, 0, POINTS_TOTAL);
		}

		if (draw_lines)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
			glDrawElements(GL_LINES, CONNECTIONS_TOTAL * 2, GL_UNSIGNED_INT, NULL);
		}
	}

	virtual void shutdown()
	{
		glDeleteProgram(m_update_program);
		glDeleteProgram(m_render_program);

		glDeleteBuffers(5, m_vbo);
		glDeleteVertexArrays(2, m_vao);
	}

	void onKey(int key, int scancode, int action, int mods)
	{
		if (action)
		{
			switch (key)
			{
			case 'R': load_shaders();
				return;
				break;
			case 'L': draw_lines = !draw_lines;
				return;
				break;
			case 'P': draw_points = !draw_points;
				return;
				break;
			case GLFW_KEY_KP_ADD: iterations_per_frame++;
				return;
				break;
			case GLFW_KEY_KP_SUBTRACT: iterations_per_frame--;
				return;
				break;
			}
		}

		OpenGLApp::onKey(key, scancode, action, mods);
	}

private:
	void load_shaders();

protected:
	GLuint m_vao[2];
	GLuint m_vbo[5];
	GLuint m_index_buffer;
	GLuint m_pos_tbo[2];
	GLuint m_update_program;
	GLuint m_render_program;
	GLuint m_C_loc;
	GLuint m_iteration_index;
	
	bool draw_points;
	bool draw_lines;
	int iterations_per_frame;
};

void springmass_app::load_shaders()
{
	if(m_update_program)
		glDeleteProgram(m_update_program);
	if(m_render_program)
		glDeleteProgram(m_render_program);

	m_update_program = LoadShadersV("../media/glsl/07_springmass_update.vs");
	static const char * tf_varyings[] = 
	{
		"tf_position_mass",
		"tf_velocity"
	};

	// glTransformFeedbackVaryings() 告知哪个着色器输出变量需要记录.
	glTransformFeedbackVaryings(m_update_program, 2, tf_varyings, GL_SEPARATE_ATTRIBS);
	glLinkProgram(m_update_program);

	m_render_program = LoadShadersVF("../media/glsl/07_springmass_render.vs", "../media/glsl/07_springmass_render.fs");
}

/** @} @} */

int main(int argc, char** argv)
{
	springmass_app a;
	a.run();
	return 0;
}