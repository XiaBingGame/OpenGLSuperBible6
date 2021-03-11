#include <iostream>
#include "gpucgapplication.h"

#ifdef _DEBUG
#pragma comment(lib, "OpenGLAppDll_d.lib")
#else
#pragma comment(lib, "OpenGLAppDll.lib")
#endif // _DEBUG
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glew32.lib")


int main(int argc, char** argv)
{
	template_app app;
	app.run();
	return 0;
}