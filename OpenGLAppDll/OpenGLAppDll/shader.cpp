#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;

#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include "shader.h"

int ReadShaderFile(const char * file_path, std::string& ShaderCode)
{
	// Read the Vertex Shader code from the file
	std::string LocalShaderCode;
	//std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	char buf[1024];
	size_t nread;

	memset(buf, 0, 1024);

	FILE * fp;
	fp = fopen(file_path, "r");
	if (!fp)
	{
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", file_path);
		return -1;
	}

	LocalShaderCode = "";
	printf(file_path);
	printf("\n");
	while ((nread = fread(buf, 1, 1023, fp)) > 0)
	{
		buf[nread] = 0;
		LocalShaderCode+=buf;
	}
	if (ferror(fp)) {
		/* deal with error */
	}
	fclose(fp);

	ShaderCode = LocalShaderCode;
	return 0;
}

GLuint LoadShadersV(const char * vertex_file_path)
{
	
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	//std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	char buf[1024];
	size_t nread;

	if(ReadShaderFile(vertex_file_path, VertexShaderCode) == -1)
		return 0;

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDeleteShader(VertexShaderID);

	return ProgramID;	
}

GLuint LoadShadersVF(const char * vertex_file_path,const char * fragment_file_path){

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	if(ReadShaderFile(vertex_file_path, VertexShaderCode) == -1)
		return 0;

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	if(ReadShaderFile(fragment_file_path, FragmentShaderCode) == -1)
		return 0;

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}


GLuint LoadShadersVTF(const char * vertex_file_path, const char* tcs_file_path, const char* tes_file_path, const char* fragment_file_path)
{
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint TCSShaderID = glCreateShader(GL_TESS_CONTROL_SHADER);
	GLuint TESShaderID = glCreateShader(GL_TESS_EVALUATION_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	if(ReadShaderFile(vertex_file_path, VertexShaderCode) == -1)
		return 0;

	// Read the Fragment Shader code from the file
	std::string TCSShaderCode;
	if(ReadShaderFile(tcs_file_path, TCSShaderCode) == -1)
		return 0;

	std::string TESShaderCode;
	if(ReadShaderFile(tes_file_path, TESShaderCode) == -1)
		return 0;

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	if(ReadShaderFile(fragment_file_path, FragmentShaderCode) == -1)
		return 0;


	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	printf("Compiling shader : %s\n", tcs_file_path);
	char const * TCSSourcePointer = TCSShaderCode.c_str();
	glShaderSource(TCSShaderID, 1, &TCSSourcePointer , NULL);
	glCompileShader(TCSShaderID);

	// Check Vertex Shader
	glGetShaderiv(TCSShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(TCSShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> TCSShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(TCSShaderID, InfoLogLength, NULL, &TCSShaderErrorMessage[0]);
		printf("%s\n", &TCSShaderErrorMessage[0]);
	}

	printf("Compiling shader : %s\n", tes_file_path);
	char const * TESSourcePointer = TESShaderCode.c_str();
	glShaderSource(TESShaderID, 1, &TESSourcePointer , NULL);
	glCompileShader(TESShaderID);

	// Check Vertex Shader
	glGetShaderiv(TESShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(TESShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> TESShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(TESShaderID, InfoLogLength, NULL, &TESShaderErrorMessage[0]);
		printf("%s\n", &TESShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, TCSShaderID);
	glAttachShader(ProgramID, TESShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDeleteShader(VertexShaderID);
	glDeleteShader(TCSShaderID);
	glDeleteShader(TESShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}



GLuint LoadShadersVGF(const char * vertex_file_path, const char* geometry_file_path, const char * fragment_file_path)
{
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	if(ReadShaderFile(vertex_file_path, VertexShaderCode) == -1)
		return 0;

	// Read the Vertex Shader code from the file
	std::string GeometryShaderCode;
	if(ReadShaderFile(geometry_file_path, GeometryShaderCode) == -1)
		return 0;

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	if(ReadShaderFile(fragment_file_path, FragmentShaderCode) == -1)
		return 0;



	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	printf("Compiling shader : %s\n", geometry_file_path);
	char const * GeometrySourcePointer = GeometryShaderCode.c_str();
	glShaderSource(GeometryShaderID, 1, &GeometrySourcePointer , NULL);
	glCompileShader(GeometryShaderID);

	// Check Vertex Shader
	glGetShaderiv(GeometryShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(GeometryShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> GeometryShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(GeometryShaderID, InfoLogLength, NULL, &GeometryShaderErrorMessage[0]);
		printf("%s\n", &GeometryShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, GeometryShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDeleteShader(VertexShaderID);
	glDeleteShader(GeometryShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

GLuint LoadShadersVTGF(const char * vertex_file_path, const char* tcs_file_path, const char* tes_file_path, const char* geometry_file_path, const char * fragment_file_path)
{
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint TCSShaderID = glCreateShader(GL_TESS_CONTROL_SHADER);
	GLuint TESShaderID = glCreateShader(GL_TESS_EVALUATION_SHADER);
	GLuint GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	if(ReadShaderFile(vertex_file_path, VertexShaderCode) == -1)
		return 0;

	// Read the Fragment Shader code from the file
	std::string TCSShaderCode;
	if(ReadShaderFile(tcs_file_path, TCSShaderCode) == -1)
		return 0;

	std::string TESShaderCode;
	if(ReadShaderFile(tes_file_path, TESShaderCode) == -1)
		return 0;

	// Read the Vertex Shader code from the file
	std::string GeometryShaderCode;
	if(ReadShaderFile(geometry_file_path, GeometryShaderCode) == -1)
		return 0;

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	if(ReadShaderFile(fragment_file_path, FragmentShaderCode) == -1)
		return 0;


	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	printf("Compiling shader : %s\n", tcs_file_path);
	char const * TCSSourcePointer = TCSShaderCode.c_str();
	glShaderSource(TCSShaderID, 1, &TCSSourcePointer , NULL);
	glCompileShader(TCSShaderID);

	// Check Vertex Shader
	glGetShaderiv(TCSShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(TCSShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> TCSShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(TCSShaderID, InfoLogLength, NULL, &TCSShaderErrorMessage[0]);
		printf("%s\n", &TCSShaderErrorMessage[0]);
	}

	printf("Compiling shader : %s\n", tes_file_path);
	char const * TESSourcePointer = TESShaderCode.c_str();
	glShaderSource(TESShaderID, 1, &TESSourcePointer , NULL);
	glCompileShader(TESShaderID);

	// Check Vertex Shader
	glGetShaderiv(TESShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(TESShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> TESShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(TESShaderID, InfoLogLength, NULL, &TESShaderErrorMessage[0]);
		printf("%s\n", &TESShaderErrorMessage[0]);
	}

	printf("Compiling shader : %s\n", geometry_file_path);
	char const * GeometrySourcePointer = GeometryShaderCode.c_str();
	glShaderSource(GeometryShaderID, 1, &GeometrySourcePointer , NULL);
	glCompileShader(GeometryShaderID);

	// Check Vertex Shader
	glGetShaderiv(GeometryShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(GeometryShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> GeometryShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(GeometryShaderID, InfoLogLength, NULL, &GeometryShaderErrorMessage[0]);
		printf("%s\n", &GeometryShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	//printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, TCSShaderID);
	glAttachShader(ProgramID, TESShaderID);
	glAttachShader(ProgramID, GeometryShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDeleteShader(VertexShaderID);
	glDeleteShader(TCSShaderID);
	glDeleteShader(TESShaderID);
	glDeleteShader(GeometryShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}


GLuint LoadShadersCS(const char * computer_file_path){

	// Create the shaders
	GLuint ComputerShaderID = glCreateShader(GL_COMPUTE_SHADER);

	// Read the Compute Shader code from the file
	std::string ComputerShaderCode;
	if(ReadShaderFile(computer_file_path, ComputerShaderCode) == -1)
		return 0;

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Computer Shader
	printf("Compiling shader : %s\n", computer_file_path);
	char const * ComputerSourcePointer = ComputerShaderCode.c_str();
	glShaderSource(ComputerShaderID, 1, &ComputerSourcePointer , NULL);
	glCompileShader(ComputerShaderID);

	// Check Vertex Shader
	glGetShaderiv(ComputerShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(ComputerShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ComputerShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(ComputerShaderID, InfoLogLength, NULL, &ComputerShaderErrorMessage[0]);
		printf("%s\n", &ComputerShaderErrorMessage[0]);
	}


	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, ComputerShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDeleteShader(ComputerShaderID);

	return ProgramID;
}