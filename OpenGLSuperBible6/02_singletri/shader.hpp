#ifndef SHADER_HPP
#define SHADER_HPP

GLuint LoadShadersVF(const char * vertex_file_path,const char * fragment_file_path);
GLuint LoadShadersVTF(const char * vertex_file_path, const char* tcs_file_path, const char* tes_file_path, const char * fragment_file_path);
GLuint LoadShadersVGF(const char * vertex_file_path, const char* geometry_file_path, const char * fragment_file_path);
GLuint LoadShadersVTGF(const char * vertex_file_path, const char* tcs_file_path, const char* tes_file_path, const char* geometry_file_path, const char * fragment_file_path);
#endif