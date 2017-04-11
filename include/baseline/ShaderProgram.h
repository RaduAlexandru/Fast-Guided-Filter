
// //CODE FROM http://www.nexcius.net/2012/11/20/how-to-load-a-glsl-shader-in-opengl-using-c/
//
// #ifndef GLSHADER_H
// #define GLSHADER_H
//
//
//
// #include "glad/glad.h"
//
// GLuint LoadShader(const char *vertex_path, const char *fragment_path);
//
// #endif


/***
author     : r3dux
version    : 0.3 - 15/01/2014
description: Gets GLSL source code either provided as strings or can load from filenames,
             compiles the shaders, creates a shader program which the shaders are linked
             to, then the program is validated and is ready for use via myProgram.use(),
             <draw-stuff-here> then calling myProgram.disable();

             Attributes and uniforms are stored in <string, int> maps and can be added
             via calls to addAttribute(<name-of-attribute>) and then the attribute
             index can be obtained via myProgram.attribute(<name-of-attribute>) - Uniforms
             work in the exact same way.
***/

#ifndef SHADER_PROGRAM_HPP
#define SHADER_PROGRAM_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class ShaderProgram {
public:
  ShaderProgram();
  ~ShaderProgram();
  void initFromFiles(std::string vertexShaderFilename, std::string fragmentShaderFilename);
  void initFromStrings(std::string vertexShaderSource, std::string fragmentShaderSource);
  inline void use(){if (initialised){
  		glUseProgram(m_program_id);
  	}
  	else{
  		std::string msg = "Shader program " + m_program_id;
  		msg += " not initialised - aborting.";
  		throw std::runtime_error(msg);
  	}
  };
  inline void disable(){
    glUseProgram(0);
  };
  GLuint attribute(const std::string attributeName);
  GLuint uniform(const std::string uniformName);
  int addAttribute(const std::string attributeName);
  int addUniform(const std::string uniformName);

private:

	// We'll use an enum to differentiate between shaders and shader programs when querying the info log
	enum class ObjectType
	{
		SHADER, PROGRAM
	};

	// Shader program and individual shader Ids
	GLuint m_program_id;
	GLuint m_vert_shader_id;
	GLuint m_frag_shader_id;

	// How many shaders are attached to the shader program
	GLuint m_shader_count;

	// // Map of attributes or uniform and their binding locations
	std::map<std::string, int> m_attribute_map;
	std::map<std::string, int> m_uniform_map;

	bool initialised;

  GLuint compileShader(std::string shaderSource, GLenum shaderType);
  void initialise(std::string vertexShaderSource, std::string fragmentShaderSource);
  std::string loadShaderFromFile(const std::string filename);
  std::string getInfoLog(ObjectType type, int id);


}; // End of class

#endif // SHADER_PROGRAM_HPP
