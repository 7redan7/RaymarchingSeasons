#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

#include <GL/glew.h>
#include <shader.hpp>


unsigned int buildProgram(const std::string& vertex_file_path,
	const std::string& fragment_file_path)
{
	// Compile shaders
	unsigned int VertexShaderID = compileShader(GL_VERTEX_SHADER, vertex_file_path);
	unsigned int FragmentShaderID = compileShader(GL_FRAGMENT_SHADER, fragment_file_path);

	// Link Program
	unsigned int ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check Program
	int success;
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &success);

	if (!success)
	{
		int logLength;
		glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &logLength);

		std::string log(logLength, '\0');
		glGetProgramInfoLog(ProgramID, logLength, nullptr, log.data());

		std::cerr << "Errore nel linking del programma:\n"
				  << log << std::endl;

		glDeleteProgram(ProgramID);
		return 0;
	}

	return ProgramID;
}

unsigned int buildProgram(const std::string& vertex_file_path,
	const std::string& tess_control_file_path,
	const std::string& tess_evaluation_file_path,
	const std::string& fragment_file_path)
{
	// Compile shaders
	unsigned int VertexShaderID = compileShader(GL_VERTEX_SHADER, vertex_file_path);
	unsigned int TessControlShaderID = compileShader(GL_TESS_CONTROL_SHADER, tess_control_file_path);
	unsigned int TessEvaluationShaderID = compileShader(GL_TESS_EVALUATION_SHADER, tess_evaluation_file_path);
	unsigned int FragmentShaderID = compileShader(GL_FRAGMENT_SHADER, fragment_file_path);

	// Link Program
	unsigned int ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, TessControlShaderID);
	glAttachShader(ProgramID, TessEvaluationShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check Program
	int success;
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &success);

	if (!success)
	{
		int logLength;
		glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &logLength);

		std::string log(logLength, '\0');
		glGetProgramInfoLog(ProgramID, logLength, nullptr, log.data());

		std::cerr << "Errore nel linking del programma:\n"
				  << log << std::endl;

		glDeleteProgram(ProgramID);
		return 0;
	}

	return ProgramID;
}

unsigned int buildProgram(const std::string& compute_file_path)
{
	unsigned int ComputeShaderID = compileShader(GL_COMPUTE_SHADER, compute_file_path);

	unsigned int ProgramID = glCreateProgram();
	glAttachShader(ProgramID, ComputeShaderID);
	glLinkProgram(ProgramID);

	int success;
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &success);

	if (!success)
	{
		int logLength;
		glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &logLength);

		std::string log(logLength, '\0');
		glGetProgramInfoLog(ProgramID, logLength, nullptr, log.data());

		std::cerr << "Errore nel linking del programma:\n"
				  << log << std::endl;

		glDeleteProgram(ProgramID);
		return 0;
	}

	return ProgramID;
}

unsigned int compileShader(GLenum shaderType, const std::string& path)
{
	// Legge il file
	std::ifstream file(path);
	if (!file.is_open())
	{
		std::cerr << "Errore: impossibile aprire lo shader: " << path << std::endl;
		return 0;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string source = buffer.str();
	const char* shaderSource = source.c_str();

	// Crea lo shader
	unsigned int shader = glCreateShader(shaderType);

	// Carica il sorgente
	glShaderSource(shader, 1, &shaderSource, nullptr);

	// Compila
	glCompileShader(shader);

	// Controlla errori
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		GLint logLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

		std::string log(logLength, '\0');
		glGetShaderInfoLog(shader, logLength, nullptr, log.data());

		std::cerr << "Errore nella compilazione dello shader:\n"
				  << path << '\n'
				  << log << std::endl;

		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

