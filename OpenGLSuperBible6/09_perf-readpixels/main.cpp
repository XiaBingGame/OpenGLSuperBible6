#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/** \addtogroup Chapter09*   @{*       \addtogroup perf-readpixels* 书籍源码没有着色器.*       @{*/

class perf_readpixels_app : public OpenGLApp
{
public:
	perf_readpixels_app() :
	  render_program(0),
		  mode(MODE_READ_OFF),
		  paused(false){}
	void init()
	{
		static const char title[] = "OpenGL SuperBible - ReadPixels Test";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}
protected:
	virtual void startup();

	virtual void render(double currentTime)
	{
		static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, green);

	}

	virtual void shutdown()
	{

	}

private:
	void load_shaders();

protected:
	GLuint              render_program;
	GLuint              pixel_buffers[5];

	sb6::object         object;

	enum MODE
	{
		MODE_READ_OFF,
		MODE_READ_TO_SYSMEM,
		MODE_READ_PBO_SAME_FRAME,
		MODE_READ_PBO_FRAME1,
		MODE_READ_PBO_FRAME2,
		MODE_READ_PBO_FRAME3,
		MODE_READ_PBO_FRAME4
	} mode;
	bool                paused;

	struct
	{
		GLint proj_matrix;
		GLint mv_matrix;
	} uniforms;
};

void perf_readpixels_app::load_shaders()
{
	if (render_program)
		glDeleteProgram(render_program);
}

void perf_readpixels_app::startup()
{

}

void perf_readpixels_app::load_shaders()
{
	if (render_program)
		glDeleteProgram(render_program);
	// 书籍源码没有着色器.
}

/** @} @} */

int main(int argc, char** argv)
{
	perf_readpixels_app a;
	a.run();
	return 0;
}