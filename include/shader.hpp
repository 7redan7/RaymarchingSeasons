unsigned int buildProgram(const std::string& vertex_file_path,
	const std::string& fragment_file_path);

unsigned int buildProgram(const std::string& vertex_file_path,
	const std::string& tess_control_file_path,
	const std::string& tess_evaluation_file_path,
	const std::string& fragment_file_path);

unsigned int buildProgram(const std::string& compute_file_path);

unsigned int compileShader(GLenum shaderType, const std::string& path);