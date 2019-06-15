#include "../common/OpenGLApp.h"
#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/** \addtogroup Chapter02
*   @{
*       \addtogroup simpleclear
* Çå¿Õ±³¾°.
*	- glClearBufferfv Çå¿Õ±³¾°.
*       @{
*/

/**

*/
class simpleclear_app : public OpenGLApp
{
	void init()
	{
		static const char title[] = "OpenGL SuperBible - Simple Clear";
		OpenGLApp::init();
		memcpy(info.title, title, sizeof(title));
	}

	void render(double currentTime)
	{
		static const GLfloat red[] = {1.0f, 0.0f, 0.0f, 1.0f};
		const GLfloat color[] = {(float)sin(currentTime)*0.5f+0.5f,
								(float)cos(currentTime)*0.5f+0.5f,
								0.0f, 1.0f};
		glClearBufferfv(GL_COLOR, 0, color);
	}

protected:
};

/** @} @} */

int main(int argc, char** argv)
{
	simpleclear_app a;
	a.run();
	return 0;
}