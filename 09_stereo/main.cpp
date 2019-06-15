// 貌似立体绘制需要显示设备的支持.

#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class stereo_app : public OpenGLApp
{
public:
	stereo_app() :
		view_program(0),
		show_light_depth_program(0),
		mode(RENDER_FULL),
		paused(false),
		separation(2.0f)
	{

	}

	void init();

protected:
	virtual void startup()
	{
		
	}

	virtual void render(double currentTime)
	{
		static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
		//glClearBufferfv(GL_COLOR, 0, green);

		
	}

	virtual void shutdown()
	{
		glDeleteProgram(view_program);
		glDeleteProgram(show_light_depth_program);
	}

protected:
	GLuint          view_program;
	GLint           show_light_depth_program;

	struct
	{
		struct
		{
			GLint   mvp;
		} light;
		struct
		{
			GLint   mv_matrix;
			GLint   proj_matrix;
			GLint   shadow_matrix;
			GLint   full_shading;
			GLint   specular_albedo;
			GLint   diffuse_albedo;
		} view;
	} uniforms;

	enum { OBJECT_COUNT = 4 };
	struct
	{
		sb6::object     obj;
		mmat4     model_matrix;
	} objects[OBJECT_COUNT];

	glm::mat4     light_view_matrix;
	glm::mat4     light_proj_matrix;

	glm::mat4     camera_view_matrix[2];
	glm::mat4     camera_proj_matrix;

	GLuint          quad_vao;

	float           separation;

	enum
	{
		RENDER_FULL,
		RENDER_LIGHT,
		RENDER_DEPTH
	} mode;

	bool paused;
};

void stereo_app::init()
{
	static const char title[] = "OpenGL SuperBible - Stereo";
	OpenGLApp::init();

	info.flags.fullscreen = 1;
	info.flags.stereo = 1;
	info.windowWidth = 0;
	info.windowHeight = 0;

	memcpy(info.title, title, sizeof(title));
}

int main(int argc, char** argv)
{
	stereo_app a;
	a.run();
	return 0;
}