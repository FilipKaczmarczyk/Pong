#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "shaders.h"

/*------------------------------------------------------------------------------------------
** funkcja wczytujaca z pliku kod zrodlowy shadera
** shaderPath - nazwa pliku z kodem zrodlowym shadera
** funkcja zwraca stringa z kodem zrodlowym shadera
**------------------------------------------------------------------------------------------*/
std::string loadShaderSource( const std::string& shaderPath )
{
	std::ifstream file;
	file.open( shaderPath, std::ios::in );

	if( !file )
	{
		std::cerr << "Nie mozna otworzyc pliku: " << shaderPath << std::endl;
		return "";
	}

	std::stringstream shaderStream;

	shaderStream << file.rdbuf();

	file.close();

	return shaderStream.str();
}

/*------------------------------------------------------------------------------------------
** funkcja tworzaca okreslony rodzaj shadera
** shaderPath - nazwa pliku z kodem zrodlowym shadera
** shaderType - typ tworzonego shadera
** shader - referencja na identyfikator tworzonego w funkcji shadera
** funkcja zwraca true jesli powiedzie sie tworzenie shadera
**------------------------------------------------------------------------------------------*/
bool createShader(const std::string& shaderPath, GLenum shaderType, GLuint &shaderID)
{
	bool result = true;
	shaderID = glCreateShader( shaderType ); // utworzenie identyfikatora shadera

	std::string source = loadShaderSource( shaderPath ); // wczytanie kodu zrodlowego shadera wierzchokow

	if( !source.empty() )
	{
		const char* shaderSource = source.c_str();
		glShaderSource( shaderID, 1, &shaderSource, nullptr ); // ustawienie kodu zrodlowego shadera

		glCompileShader( shaderID ); // kompilacja shadera

		GLint compileStatus;
		glGetShaderiv( shaderID, GL_COMPILE_STATUS, &compileStatus );
		if( compileStatus == 0 )
		{
			std::cerr << "Blad przy kompilacji: " << shaderPath << std::endl;
			printShaderInfoLog( shaderID ); // wyswietlenie logu shadera

			result = false;
			glDeleteShader( shaderID );
		}
	}
	else
	{
		result = false;
		glDeleteShader( shaderID );
	}

	return result;
}

/*------------------------------------------------------------------------------------------
** funkcja wyswietla zawartosc logu shadera
** shader - identyfikator shadera
**------------------------------------------------------------------------------------------*/
void printShaderInfoLog( GLuint shader )
{
	int infologLength = 0;

	glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &infologLength );

	if( infologLength > 0 )
	{
		int charsWritten = 0;
		char* infoLog = new char[infologLength];

		glGetShaderInfoLog( shader, infologLength, &charsWritten, infoLog );

		std::cerr << infoLog << std::endl;

		delete[] infoLog;
	}
}

/*------------------------------------------------------------------------------------------
** funkcja wyswietla zawartosc logu programu cieniowania
** program - identyfikator programu cieniowania
**------------------------------------------------------------------------------------------*/
void printProgramInfoLog(GLuint program)
{
    int infologLength = 0;

    glGetProgramiv( program, GL_INFO_LOG_LENGTH, &infologLength );
 
    if( infologLength > 0 )
    {
		int charsWritten = 0;
		char* infoLog = new char[infologLength];

        glGetProgramInfoLog( program, infologLength, &charsWritten, infoLog );

		std::cerr << infoLog << std::endl;

		delete [] infoLog;
    }
}

/*------------------------------------------------------------------------------------------
** funkcja tworzaca program cieniowania skladajacy sie z shadera wierzcholkow i fragmentow
** vertexShaderPath - nazwa pliku z kodem zrodlowym shadera wierzcholkow
** fragmentShaderPath - nazwa pliku z kodem zrodlowym shadera fragmentow
** shaderProgram - referencja na identyfikator tworzonego w funkcji programu
** funkcja zwraca true jesli powiedzie sie tworzenie programu cieniowania
**------------------------------------------------------------------------------------------*/
bool setupShaders(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, GLuint &shaderProgram)
{
	shaderProgram = glCreateProgram(); // utworzenie identyfikatora programu cieniowania
 
	GLuint vertexShader;
	if( !createShader( vertexShaderPath, GL_VERTEX_SHADER, vertexShader ) )
	{
		glDeleteProgram( shaderProgram );

		return false;
	}
	
	GLuint fragmentShader;
	if( !createShader( fragmentShaderPath, GL_FRAGMENT_SHADER, fragmentShader) )
	{
		glDeleteShader( vertexShader );
		glDeleteProgram( shaderProgram );

		return false;
	}

	glAttachShader( shaderProgram, vertexShader ); // dolaczenie shadera wierzcholkow
	glAttachShader( shaderProgram, fragmentShader ); // dolaczenie shadera fragmentow

    glLinkProgram( shaderProgram ); // linkowanie programu cieniowania

	GLint linkStatus;
	glGetProgramiv( shaderProgram, GL_LINK_STATUS, &linkStatus );
	if( linkStatus == 0 )
	{
		std::cerr << "Blad przy linkowaniu programu cieniowania (" << vertexShaderPath << ", " << fragmentShaderPath << ")\n";

		printProgramInfoLog( shaderProgram ); // wyswietlenie logu linkowania

		glDeleteShader( vertexShader );
		glDeleteShader( fragmentShader );
		glDeleteProgram( shaderProgram );

		return false;
	}

	glDeleteShader( vertexShader );
	glDeleteShader( fragmentShader );

	return true;
}