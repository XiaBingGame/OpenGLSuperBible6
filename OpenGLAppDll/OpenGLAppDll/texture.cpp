#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <windows.h>
#include "texture.h"

static const unsigned char identifier[] =
{
	0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A
};


GLuint loadBMP_custom(const char * imagepath){

	printf("Reading image %s\n", imagepath);

	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	// Actual RGB data
	unsigned char * data;

	// Open the file
	FILE * file = fopen(imagepath,"rb");
	if (!file)							    {printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar(); return 0;}

	// Read the header, i.e. the 54 first bytes

	// If less than 54 bytes are read, problem
	if ( fread(header, 1, 54, file)!=54 ){ 
		printf("Not a correct BMP file\n");
		return 0;
	}
	// A BMP files always begins with "BM"
	if ( header[0]!='B' || header[1]!='M' ){
		printf("Not a correct BMP file\n");
		return 0;
	}
	// Make sure this is a 24bpp file
	if ( *(int*)&(header[0x1E])!=0  )         {printf("Not a correct BMP file\n");    return 0;}
	if ( *(int*)&(header[0x1C])!=24 )         {printf("Not a correct BMP file\n");    return 0;}

	// Read the information about the image
	dataPos    = *(int*)&(header[0x0A]);
	imageSize  = *(int*)&(header[0x22]);
	width      = *(int*)&(header[0x12]);
	height     = *(int*)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize==0)    imageSize=width*height*3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos==0)      dataPos=54; // The BMP header is done that way

	// Create a buffer
	data = new unsigned char [imageSize];

	// Read the actual data from the file into the buffer
	fread(data,1,imageSize,file);

	// Everything is in memory now, the file wan be closed
	fclose (file);

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);
	
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	// OpenGL has now copied the data. Free our own version
	delete [] data;

	// Poor filtering, or ...
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 

	// ... nice trilinear filtering.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
	glGenerateMipmap(GL_TEXTURE_2D);

	// Return the ID of the texture we just created
	return textureID;
}

// Since GLFW 3, glfwLoadTexture2D() has been removed. You have to use another texture loading library, 
// or do it yourself (just like loadBMP_custom and loadDDS)
//GLuint loadTGA_glfw(const char * imagepath){
//
//	// Create one OpenGL texture
//	GLuint textureID;
//	glGenTextures(1, &textureID);
//
//	// "Bind" the newly created texture : all future texture functions will modify this texture
//	glBindTexture(GL_TEXTURE_2D, textureID);
//
//	// Read the file, call glTexImage2D with the right parameters
//	glfwLoadTexture2D(imagepath, 0);
//
//	// Nice trilinear filtering.
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
//	glGenerateMipmap(GL_TEXTURE_2D);
//
//	// Return the ID of the texture we just created
//	return textureID;
//}



#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII

GLuint loadDDS(const char * imagepath){

	unsigned char header[124];

	FILE *fp; 

	/* try to open the file */ 
	fp = fopen(imagepath, "rb"); 
	if (fp == NULL){
		printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar(); 
		return 0;
	}

	/* verify the type of file */ 
	char filecode[4]; 
	fread(filecode, 1, 4, fp); 
	if (strncmp(filecode, "DDS ", 4) != 0) { 
		fclose(fp); 
		return 0; 
	}

	/* get the surface desc */ 
	fread(&header, 124, 1, fp); 

	unsigned int height      = *(unsigned int*)&(header[8 ]);
	unsigned int width	     = *(unsigned int*)&(header[12]);
	unsigned int linearSize	 = *(unsigned int*)&(header[16]);
	unsigned int mipMapCount = *(unsigned int*)&(header[24]);
	unsigned int fourCC      = *(unsigned int*)&(header[80]);


	unsigned char * buffer;
	unsigned int bufsize;
	/* how big is it going to be including all mipmaps? */ 
	bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize; 
	//buffer = (unsigned char*)malloc(bufsize * sizeof(unsigned char)); 
	unsigned int membufsize = bufsize * sizeof(unsigned char);
	buffer = new unsigned char[membufsize];
	fread(buffer, 1, bufsize, fp); 
	/* close the file pointer */ 
	fclose(fp);

	unsigned int components  = (fourCC == FOURCC_DXT1) ? 3 : 4; 
	unsigned int format;

	switch(fourCC) 
	{ 
	case FOURCC_DXT1: 
		format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; 
		break; 
	case FOURCC_DXT3: 
		format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; 
		break; 
	case FOURCC_DXT5: 
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; 
		break; 
	default: 
		//free(buffer);
		GlobalFree(buffer);
		return 0; 
	}

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);	

	unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16; 
	unsigned int offset = 0;

	/* load the mipmaps */ 
	for (unsigned int level = 0; level < mipMapCount && (width || height); ++level) 
	{ 
		unsigned int size = ((width+3)/4)*((height+3)/4)*blockSize; 
		glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height,  
			0, size, buffer + offset); 

		offset += size; 
		width  /= 2; 
		height /= 2; 

		// Deal with Non-Power-Of-Two textures. This code is not included in the webpage to reduce clutter.
		if(width < 1) width = 1;
		if(height < 1) height = 1;

	} 

	//free(buffer); 
	delete [] buffer;
	return textureID;
}


const unsigned int swap32(const unsigned int u32)
{
	union
	{
		unsigned int u32;
		unsigned char u8[4];
	} a, b;

	a.u32 = u32;
	b.u8[0] = a.u8[3];
	b.u8[1] = a.u8[2];
	b.u8[2] = a.u8[1];
	b.u8[3] = a.u8[0];

	return b.u32;
}


unsigned int calculate_stride(const header& h, unsigned int width, unsigned int pad = 4)
{
	unsigned int channels = 0;

	switch (h.glbaseinternalformat)
	{
	case GL_RED:    channels = 1;
		break;
	case GL_RG:     channels = 2;
		break;
	case GL_BGR:
	case GL_RGB:    channels = 3;
		break;
	case GL_BGRA:
	case GL_RGBA:   channels = 4;
		break;
	}

	unsigned int stride = h.gltypesize * channels * width;

	stride = (stride + (pad - 1)) & ~(pad - 1);

	return stride;
}


unsigned int calculate_face_size(const header& h)
{
	unsigned int stride = calculate_stride(h, h.pixelwidth);

	return stride * h.pixelheight;
}



unsigned int loadKTX(const char * filename, unsigned int tex)
{
	FILE * fp;
	GLuint temp = 0;
	GLuint retval = 0;
	header h;
	size_t data_start, data_end;
	unsigned char * data;
	GLenum target = GL_NONE;

	fp = fopen(filename, "rb");

	if (!fp)
		return 0;

	if (fread(&h, sizeof(h), 1, fp) != 1)
		goto fail_read;

	if (memcmp(h.identifier, identifier, sizeof(identifier)) != 0)
		goto fail_header;

	if (h.endianness == 0x04030201)
	{
		// No swap needed
	}
	else if (h.endianness == 0x01020304)
	{
		// Swap needed
		h.endianness            = swap32(h.endianness);
		h.gltype                = swap32(h.gltype);
		h.gltypesize            = swap32(h.gltypesize);
		h.glformat              = swap32(h.glformat);
		h.glinternalformat      = swap32(h.glinternalformat);
		h.glbaseinternalformat  = swap32(h.glbaseinternalformat);
		h.pixelwidth            = swap32(h.pixelwidth);
		h.pixelheight           = swap32(h.pixelheight);
		h.pixeldepth            = swap32(h.pixeldepth);
		h.arrayelements         = swap32(h.arrayelements);
		h.faces                 = swap32(h.faces);
		h.miplevels             = swap32(h.miplevels);
		h.keypairbytes          = swap32(h.keypairbytes);
	}
	else
	{
		goto fail_header;
	}

	// Guess target (texture type)
	if (h.pixelheight == 0)
	{
		if (h.arrayelements == 0)
		{
			target = GL_TEXTURE_1D;
		}
		else
		{
			target = GL_TEXTURE_1D_ARRAY;
		}
	}
	else if (h.pixeldepth == 0)
	{
		if (h.arrayelements == 0)
		{
			if (h.faces == 0)
			{
				target = GL_TEXTURE_2D;
			}
			else
			{
				target = GL_TEXTURE_CUBE_MAP;
			}
		}
		else
		{
			if (h.faces == 0)
			{
				target = GL_TEXTURE_2D_ARRAY;
			}
			else
			{
				target = GL_TEXTURE_CUBE_MAP_ARRAY;
			}
		}
	}
	else
	{
		target = GL_TEXTURE_3D;
	}

	// Check for insanity...
	if (target == GL_NONE ||                                    // Couldn't figure out target
		(h.pixelwidth == 0) ||                                  // Texture has no width???
		(h.pixelheight == 0 && h.pixeldepth != 0))              // Texture has depth but no height???
	{
		goto fail_header;
	}

	temp = tex;
	if (tex == 0)
	{
		glGenTextures(1, &tex);
	}

	glBindTexture(target, tex);

	data_start = ftell(fp) + h.keypairbytes;
	fseek(fp, 0, SEEK_END);
	data_end = ftell(fp);
	fseek(fp, data_start, SEEK_SET);

	data = new unsigned char [data_end - data_start];
	memset(data, 0, data_end - data_start);

	fread(data, 1, data_end - data_start, fp);

	if (h.miplevels == 0)
	{
		h.miplevels = 1;
	}

	switch (target)
	{
	case GL_TEXTURE_1D:
		glTexStorage1D(GL_TEXTURE_1D, h.miplevels, h.glinternalformat, h.pixelwidth);
		glTexSubImage1D(GL_TEXTURE_1D, 0, 0, h.pixelwidth, h.glformat, h.glinternalformat, data);
		break;
	case GL_TEXTURE_2D:
		glTexStorage2D(GL_TEXTURE_2D, h.miplevels, h.glinternalformat, h.pixelwidth, h.pixelheight);
		{
			unsigned char * ptr = data;
			unsigned int height = h.pixelheight;
			unsigned int width = h.pixelwidth;
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			for (unsigned int i = 0; i < h.miplevels; i++)
			{
				glTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, width, height, h.glformat, h.gltype, ptr);
				ptr += height * calculate_stride(h, width, 1);
				height >>= 1;
				width >>= 1;
				if (!height)
					height = 1;
				if (!width)
					width = 1;
			}
		}
		break;
	case GL_TEXTURE_3D:
		glTexStorage3D(GL_TEXTURE_3D, h.miplevels, h.glinternalformat, h.pixelwidth, h.pixelheight, h.pixeldepth);
		glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, h.pixelwidth, h.pixelheight, h.pixeldepth, h.glformat, h.gltype, data);
		break;
	case GL_TEXTURE_1D_ARRAY:
		glTexStorage2D(GL_TEXTURE_1D_ARRAY, h.miplevels, h.glinternalformat, h.pixelwidth, h.arrayelements);
		glTexSubImage2D(GL_TEXTURE_1D_ARRAY, 0, 0, 0, h.pixelwidth, h.arrayelements, h.glformat, h.gltype, data);
		break;
	case GL_TEXTURE_2D_ARRAY:
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, h.miplevels, h.glinternalformat, h.pixelwidth, h.pixelheight, h.arrayelements);
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, h.pixelwidth, h.pixelheight, h.arrayelements, h.glformat, h.gltype, data);
		break;
	case GL_TEXTURE_CUBE_MAP:
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, h.miplevels, h.glinternalformat, h.pixelwidth, h.pixelheight);
		// glTexSubImage3D(GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0, h.pixelwidth, h.pixelheight, h.faces, h.glformat, h.gltype, data);
		{
			unsigned int face_size = calculate_face_size(h);
			for (unsigned int i = 0; i < h.faces; i++)
			{
				glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, h.pixelwidth, h.pixelheight, h.glformat, h.gltype, data + face_size * i);
			}
		}
		break;
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, h.miplevels, h.glinternalformat, h.pixelwidth, h.pixelheight, h.arrayelements);
		glTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, 0, 0, 0, h.pixelwidth, h.pixelheight, h.faces * h.arrayelements, h.glformat, h.gltype, data);
		break;
	default:                                               // Should never happen
		goto fail_target;
	}

	if (h.miplevels == 1)
	{
		glGenerateMipmap(target);
	}

	retval = tex;

fail_target:
	delete [] data;

fail_header:;
fail_read:;
	fclose(fp);

	return retval;
}
