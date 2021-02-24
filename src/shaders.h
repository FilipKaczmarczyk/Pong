#ifndef __SHADERS_H__
#define __SHADERS_H__

std::string loadShaderSource( const std::string& shaderPath );
bool createShader( const std::string& shaderPath, GLenum shaderType, GLuint& shaderID );
void printShaderInfoLog( GLuint shader );
void printProgramInfoLog( GLuint program );
bool setupShaders( const std::string& vertexShaderPath, const std::string& fragmentShaderPath, GLuint& shaderProgram );

#endif /* __SHADERS_H__ */