
//CODE FROM http://www.nexcius.net/2012/11/20/how-to-load-a-glsl-shader-in-opengl-using-c/
//THE ABOVE Is not used anymore

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
#include "baseline/ShaderProgram.h"



ShaderProgram::ShaderProgram():
  m_shader_count(0),
  initialised(false)
  {
	// Note: We MUST have a valid rendering context before generating the m_program_id or we'll segfault!
	m_program_id = glCreateProgram();
	glUseProgram(m_program_id);

}


ShaderProgram::~ShaderProgram()
{
	glDeleteProgram(m_program_id);
}


// Private method to compile a shader of a given type
GLuint ShaderProgram::compileShader(std::string shaderSource, GLenum shaderType){
	std::string shaderTypeString;
	switch (shaderType){
		case GL_VERTEX_SHADER:
			shaderTypeString = "GL_VERTEX_SHADER";
			break;
		case GL_FRAGMENT_SHADER:
			shaderTypeString = "GL_FRAGMENT_SHADER";
			break;
		case GL_GEOMETRY_SHADER:
			throw std::runtime_error("Geometry shaders are unsupported at this time.");
			break;
		default:
			throw std::runtime_error("Bad shader type enum in compileShader.");
			break;
	}

	// Generate a shader id
	// Note: Shader id will be non-zero if successfully created.
	GLuint shaderId = glCreateShader(shaderType);
	if (shaderId == 0){
		// Display the shader log via a runtime_error
		throw std::runtime_error("Could not create shader of type " + shaderTypeString + ": " + getInfoLog(ObjectType::SHADER, shaderId) );
	}

	// Get the source string as a pointer to an array of characters
	const char *shaderSourceChars = shaderSource.c_str();

	// Attach the GLSL source code to the shader
	// Params: GLuint shader, GLsizei count, const GLchar **string, const GLint *length
	// Note: The pointer to an array of source chars will be null terminated, so we don't need to specify the length and can instead use NULL.
	glShaderSource(shaderId, 1, &shaderSourceChars, NULL);
	glCompileShader(shaderId);

	// Check the compilation status and throw a runtime_error if shader compilation failed
	GLint shaderStatus;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &shaderStatus);
	if (shaderStatus == GL_FALSE){
		throw std::runtime_error(shaderTypeString + " compilation failed: " + getInfoLog(ObjectType::SHADER, shaderId) );
	}

	// If everything went well, return the shader id
	return shaderId;
}

// Private method to compile/attach/link/verify the shaders.
// Note: Rather than returning a boolean as a success/fail status we'll just consider
// a failure here to be an unrecoverable error and throw a runtime_error.
void ShaderProgram::initialise(std::string vertexShaderSource, std::string fragmentShaderSource){
	// Compile the shaders and return their id values
	m_vert_shader_id   = compileShader(vertexShaderSource,   GL_VERTEX_SHADER);
	m_frag_shader_id = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

	// Attach the compiled shaders to the shader program
	glAttachShader(m_program_id, m_vert_shader_id);
	glAttachShader(m_program_id, m_frag_shader_id);

	// Link the shader program - details are placed in the program info log
	glLinkProgram(m_program_id);

	// Once the shader program has the shaders attached and linked, the shaders are no longer required.
	// If the linking failed, then we're going to abort anyway so we still detach the shaders.
	glDetachShader(m_program_id, m_vert_shader_id);
	glDetachShader(m_program_id, m_frag_shader_id);

	// Check the program link status and throw a runtime_error if program linkage failed.
	GLint programLinkSuccess = GL_FALSE;
	glGetProgramiv(m_program_id, GL_LINK_STATUS, &programLinkSuccess);
	if (programLinkSuccess == GL_FALSE){
		throw std::runtime_error("Shader program link failed: " + getInfoLog(ObjectType::PROGRAM, m_program_id) );
	}

	glValidateProgram(m_program_id);

	// Check the validation status and throw a runtime_error if program validation failed
	GLint programValidatationStatus;
	glGetProgramiv(m_program_id, GL_VALIDATE_STATUS, &programValidatationStatus);
	if (programValidatationStatus == GL_FALSE){
		throw std::runtime_error("Shader program validation failed: " + getInfoLog(ObjectType::PROGRAM, m_program_id) );
	}


	// Finally, the shader program is initialised
	initialised = true;
}

// Private method to load the shader source code from a file
std::string ShaderProgram::loadShaderFromFile(const std::string filename)
{
	// Create an input filestream and attempt to open the specified file
	std::ifstream file( filename.c_str() );

	// If we couldn't open the file we'll bail out
	if ( !file.good() ){
		throw std::runtime_error("Failed to open file: " + filename);
	}

	// Otherwise, create a string stream...
	std::stringstream stream;

	// ...and dump the contents of the file into it.
	stream << file.rdbuf();

	// Now that we've read the file we can close it
	file.close();

	// Finally, convert the stringstream into a string and return it
	return stream.str();
}

// Private method to return the current shader program info log as a string
std::string ShaderProgram::getInfoLog(ObjectType type, int id)
{
	GLint infoLogLength;
	if (type == ObjectType::SHADER)
	{
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
	}
	else // type must be ObjectType::PROGRAM
	{
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
	}

	GLchar *infoLog = new GLchar[infoLogLength + 1];
	if (type == ObjectType::SHADER)
	{
		glGetShaderInfoLog(id, infoLogLength, NULL, infoLog);
	}
	else // type must be ObjectType::PROGRAM
	{
		glGetProgramInfoLog(id, infoLogLength, NULL, infoLog);
	}

	// Convert the info log to a string
	std::string infoLogString(infoLog);

	// Delete the char array version of the log
	delete[] infoLog;

	// Finally, return the string version of the info log
	return infoLogString;
}



// Method to initialise a shader program from shaders provided as files
void ShaderProgram::initFromFiles(std::string vertexShaderFilename, std::string fragmentShaderFilename)
{
	// Get the shader file contents as strings
	std::string vertexShaderSource   = loadShaderFromFile(vertexShaderFilename);
	std::string fragmentShaderSource = loadShaderFromFile(fragmentShaderFilename);

	initialise(vertexShaderSource, fragmentShaderSource);
}

// Method to initialise a shader program from shaders provided as strings
void ShaderProgram::initFromStrings(std::string vertexShaderSource, std::string fragmentShaderSource)
{
	initialise(vertexShaderSource, fragmentShaderSource);
}

// // Method to enable the shader program - we'll suggest this for inlining
// inline void ShaderProgram::use()
// {
// 	// Santity check that we're initialised and ready to go...
//
// }
//
// // Method to disable the shader - we'll also suggest this for inlining
// inline void ShaderProgram::disable()
// {
// 	glUseProgram(0);
// }

// Method to return the bound location of a named attribute, or -1 if the attribute was not found
GLuint ShaderProgram::attribute(const std::string attributeName)
{
	// You could do this method with the single line:
	//
	//		return m_attribute_map[attribute];
	//
	// BUT, if you did, and you asked it for a named attribute which didn't exist
	// like: m_attribute_map["FakeAttrib"] then the method would return an invalid
	// value which will likely cause the program to segfault. So we're making sure
	// the attribute asked for exists, and if it doesn't then we alert the user & bail.

	// Create an iterator to look through our attribute map (only create iterator on first run -
	// reuse it for all further calls).
	static std::map<std::string, int>::const_iterator attributeIter;

	// Try to find the named attribute
	attributeIter = m_attribute_map.find(attributeName);

	// Not found? Bail.
	if ( attributeIter == m_attribute_map.end() ){
		throw std::runtime_error("Could not find attribute in shader program: " + attributeName);
	}

	// Otherwise return the attribute location from the attribute map
	return m_attribute_map[attributeName];
}

// Method to returns the bound location of a named uniform
GLuint ShaderProgram::uniform(const std::string uniformName)
{
	// Note: You could do this method with the single line:
	//
	// 		return uniformLocList[uniform];
	//
	// But we're not doing that. Explanation in the attribute() method above.

	// Create an iterator to look through our uniform map (only create iterator on first run -
	// reuse it for all further calls).
	static std::map<std::string, int>::const_iterator uniformIter;

	// Try to find the named uniform
	uniformIter = m_uniform_map.find(uniformName);

	// Found it? Great - pass it back! Didn't find it? Alert user and halt.
	if ( uniformIter == m_uniform_map.end() ){
		throw std::runtime_error("Could not find uniform in shader program: " + uniformName);
	}

	// Otherwise return the attribute location from the uniform map
	return m_uniform_map[uniformName];
}

// Method to add an attribute to the shader and return the bound location
int ShaderProgram::addAttribute(const std::string attributeName)
{
	// Add the attribute location value for the attributeName key
	m_attribute_map[attributeName] = glGetAttribLocation( m_program_id, attributeName.c_str() );

	// Check to ensure that the shader contains an attribute with this name
	if (m_attribute_map[attributeName] == -1){
		throw std::runtime_error("Could not add attribute: " + attributeName + " - location returned -1.");
	}


	// Return the attribute location
	return m_attribute_map[attributeName];
}

// Method to add a uniform to the shader and return the bound location
int ShaderProgram::addUniform(const std::string uniformName)
{
	// Add the uniform location value for the uniformName key
	m_uniform_map[uniformName] = glGetUniformLocation( m_program_id, uniformName.c_str() );

	// Check to ensure that the shader contains a uniform with this name
	if (m_uniform_map[uniformName] == -1){
		throw std::runtime_error("Could not add uniform: " + uniformName + " - location returned -1.");
	}

	// Return the uniform location
	return m_uniform_map[uniformName];
}
