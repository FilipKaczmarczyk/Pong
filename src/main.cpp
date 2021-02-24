#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <array>
#include <stdlib.h>     
#include <time.h> 

#include "shaders.h"

constexpr int WIN_WIDTH = 800; // SZEROKOå∆ OKNA
constexpr int WIN_HEIGHT = 600; // WYSOKOå∆ OKNA

// DEKLARACJA STRUKTURY WEKTORA DWUWYMIAROWEGO
struct vec2 {
	float x;
	float y;
};

// ZMIENNE OKREåLAJ•CE STA£E W£AåCIWOåCI OBIEKT”W
const float racketsSpeed = 300.0f; // SZYKBOå∆ PORUSZANIA SI  RAKIETEK
const float racketsHeight = 75.0f; // WYSOKOå∆ RAKIETEK
const float halfRacketsHeight = racketsHeight / 2.0f; // PO£OWA WYSOKOåCI RAKIETEK
const float racketsWidth = 10.0f; // SZEROKOåC RAKIETEK
const float halfRacketsWidth = racketsWidth / 2.0f; // PO£OWA SZEROKOåCI RAKIETEK
const float ballDiameter = 5.0f; // åEDNICA PI£ECZKI
const float ballRadius = 2.5f; // PROMIE— PI£ECZKI
const float racketLimit = halfRacketsHeight + ballRadius; // GRANICA PRZESUWANIA SI  RAKIETEK DO G”RY I W D”£ EKRANU

// TABLICA WIERZCHO£K”W RAKIETEK
float verticesForRackets[] =
{
	0.5f,  0.5f,
   -0.5f,  0.5f,
   -0.5f, -0.5f,
    0.5f, -0.5f
};

// TABLICA INDEKS”W RAKIETEK
unsigned int indicesForRackets[] =
{
	0, 1, 2,
	2, 3, 0
};

// TABLICA POZYCJI RAKIETEK
vec2 offsetsForRackets[] =
{
	20.0f, WIN_HEIGHT / 2.0f,
	WIN_WIDTH - 20.0f, WIN_HEIGHT / 2.0f
};

// TABLICA WYMIAR”W RAKIETEK
vec2 sizesForRackets = { racketsWidth, racketsHeight };

// WKAèNIKI NA WIERZCHO£KI ORAZ INDKESY DLA PI£ECZKI
float* verticesForBall;
unsigned int* indicesForBall;

unsigned int numOfTraingles = 20; // LICZBA TR”JK•T”W TWORZ•CYCH PI£ECZK 

vec2 offsetsForBall = { WIN_WIDTH / 2.0f, WIN_HEIGHT / 2.0f }; // POZYCJA PI£ECZKI

vec2 sizesForBall = { ballDiameter, ballDiameter }; //WYMIARY PI£ECZKI

// PR DKOå∆ PI£ECZKI ORAZ RAKIETEK
vec2 velocityForBall;
float velocityForRackets[2];

// PUNKTY 
unsigned int scoreForLeft = 0;
unsigned int scoreForRight = 0;

//******************************************************************************************
GLuint shaderProgram; // identyfikator programu cieniowania

GLuint vertexLoc; // lokalizacja atrybutu wierzcholka - wspolrzedne
GLuint colorLoc; // lokalizacja atrybutu wierzcholka - kolor

std::array<GLuint, 2> vao; // identyfikatory obiektow VAO
std::array<GLuint, 6> buffers; // identyfikatory obiektow VBO
std::array<GLuint, 2> ebos; // identyfikatory obiektow VBO
//******************************************************************************************

void errorCallback( int error, const char* description );
void cleanup();
void initGL();
void setupShaders();
void setupBuffers();
void renderScene();
void setOrthographicProjection(int shaderProgram, float left, float right, float bottom, float top, float near, float far);
void processInput(GLFWwindow* window, double dt);
void generateCircleArray(float*& vertices, unsigned int*& indices, unsigned int numOfTriangles, float radius);
void displayScore();
void ballDirection(unsigned int direction, float x, float yMin, float yMax);

int main(int argc, char* argv[])
{
	// wartoúci potrzebene do obliczeÒ czasu miÍdzy klatkami
	double dt = 0.0;
	double lastFrame = 0.0;

	int framesWhenLastCollision = -1; // ile klatek temu wystπpi≥a poprzednia kolizja
	int framesToWaitForNextColision = 10; // ile klatek naleøy odczekaÊ miÍdzy kolizjami

	GLFWwindow* window;

	glfwSetErrorCallback( errorCallback ); // rejestracja funkcji zwrotnej do obslugi bledow

	if( !glfwInit() ) // inicjacja biblioteki GLFW
		exit( EXIT_FAILURE );

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 ); // inicjacja wersji kontekstu
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );

	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE ); // incicjacja profilu rdzennego

	window = glfwCreateWindow( WIN_WIDTH, WIN_HEIGHT, "Pong", nullptr, nullptr ); // utworzenie okna i zwiazanego z nim kontekstu
	if( !window )
	{
		glfwTerminate(); // konczy dzialanie biblioteki GLFW
		exit( EXIT_FAILURE );
	}

	glfwMakeContextCurrent( window );

	// inicjacja GLEW
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if( err != GLEW_OK )
	{
		std::cerr << "Blad: " << glewGetErrorString( err ) << std::endl;
		exit( 1 );
	}

	if( !GLEW_VERSION_3_3 )
	{
		std::cerr << "Brak obslugi OpenGL 3.3\n";
		exit( 2 );
	}

	glfwSwapInterval( 1 ); // v-sync on

	initGL();

	setOrthographicProjection(shaderProgram, 0, WIN_WIDTH, 0, WIN_HEIGHT, 0.0f, 1.0f);

	srand(time(NULL));
	ballDirection(0, 200.0f, -200.0f, 200.0f);

	displayScore();

	// glowna petla programu
	while( !glfwWindowShouldClose( window ) )
	{
		dt = glfwGetTime() - lastFrame;
		lastFrame += dt;

		unsigned int reset = 0; // zmienna inforumjπca, kto zdoby≥ punkt 

		if (framesWhenLastCollision != -1) {
			framesWhenLastCollision++;
		}

		// Sterowanie
		processInput(window, dt);

		//SPRAWDZANIE KOLIZJI PI£ECZKI Z G”R• I DO£EM OKNA
		if (offsetsForBall.y - ballRadius <= 0 || offsetsForBall.y + ballRadius >= WIN_HEIGHT)
		{
			velocityForBall.y *= -1;
		}

		// SPRAWDZANIE KOLIZJI PI£ECZKI Z LEW• åCIAN•
		if (offsetsForBall.x - ballRadius <= 0) {
			scoreForRight++;
			reset = 1;
		}

		// SPRAWDZANIE KOLIZJI PI£ECZKI Z PRAW•
		else if (offsetsForBall.x + ballRadius >= WIN_WIDTH) {
			scoreForLeft++;
			reset = 2;
		}

		// RESET POZYCJI I KIERUNKU RUCHU PI£ECZKI PO ZDOBYCIU PUNKTU
		if (reset) {
			offsetsForBall.x = WIN_WIDTH / 2.0f;
			offsetsForBall.y = WIN_HEIGHT / 2.0f;

			ballDirection(reset, 200.0f, -200.0f, 200.0f);

			displayScore();
		}

		/*-----------------------------------------
		SPRAWDZANIE KOLIZJI PI£ECZKI Z RAKIETKAMI
		------------------------------------------*/
		if (framesWhenLastCollision >= framesToWaitForNextColision || framesWhenLastCollision == -1) 
		{
			// SPRAWDZENIE CZY PI£ECZKA JEST PO LEWEJ CZY PRAWEJ STRONIE EKRANU
			int i = 0;
			if (offsetsForBall.x > WIN_HEIGHT / 2.0f)
			{
				i++;
			}

			// DYSTANS MI DZY PI£ECZK• A RAKIETK•
			vec2 distance = { std::abs(offsetsForBall.x - offsetsForRackets[i].x), std::abs(offsetsForBall.y - offsetsForRackets[i].y) };

			// SPRAWDZENIE CZY KOLIZJA WYST PUJE
			if (distance.x <= halfRacketsWidth + ballRadius && distance.y <= halfRacketsHeight + ballRadius)
			{
				bool collision = false;
				// SPRAWDZENIE CZY KOLIZJA WYST PUJE Z WYSOKOåCI• RAKIETKI
				if (distance.x <= halfRacketsWidth && distance.x >= (halfRacketsWidth - ballRadius))
				{
					collision = true;
					velocityForBall.x *= -1;
				}
				// SPRAWDZENIE CZY KOLIZJA WYST PUJE Z SZEROKOåCI• RAKIETKI
				else if (distance.y <= halfRacketsHeight && distance.y >= (halfRacketsHeight - ballRadius))
				{
					collision = true;
					velocityForBall.y *= -1;
				}

				if (collision)
				{
					velocityForBall.x *= 1.01f;
					// ZRESESTOWANIE OSTATNIEJ KLATKI Z KOLIZJ•
					framesWhenLastCollision = 0;
				}
			}
		}
		// AKTUALIZOWANIE POZYCJI RAKIETEK
		offsetsForRackets[0].y += velocityForRackets[0] * dt;
		offsetsForRackets[1].y += velocityForRackets[1] * dt;

		// AKTUALIZOWANIE POZYCJI PI£KI 
		offsetsForBall.x += velocityForBall.x * dt;
		offsetsForBall.y += velocityForBall.y * dt;

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT); // czyszczenie bufora koloru

		// AKTUALIZOWANIE POZYCJI PI£KI W GPU
		glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, 1 * sizeof(vec2), &offsetsForBall);

		// AKTUALIZOWANIE POZYCJI RAKIETEK W GPU
		glBindBuffer(GL_ARRAY_BUFFER, buffers[4]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, 2 * sizeof(vec2), offsetsForRackets);

		renderScene();

		glfwSwapBuffers( window ); // zamieniamy bufory
		glfwPollEvents(); // przetwarzanie zdarzen
	}

	cleanup();

	glfwDestroyWindow( window ); // niszczy okno i jego kontekst
	glfwTerminate();

	return 0;
}

/*------------------------------------------------------------------------------------------
** funkcja zwrotna do obslugi bledow biblioteki GLFW
** error - kod bledu
** description - opis bledu
**------------------------------------------------------------------------------------------*/
void errorCallback( int error, const char* description )
{
	std::cerr << "Error: " << description << std::endl;
}

/*------------------------------------------------------------------------------------------
** funkcja zwrotna do obslugi klawiatury
** window - okno, ktÛre otrzymalo zdarzenie
** key - klawisz jaki zostal nacisniety lub zwolniony
** scancode - scancode klawisza specyficzny dla systemu
** action - zachowanie klawisza (GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT)
** mods - pole bitowe zawierajace informacje o nacisnietych modyfikatorach (GLFW_MOD_SHIFT, GLFW_MOD_CONTROL, GLFW_MOD_ALT, GLFW_MOD_SUPER)
**------------------------------------------------------------------------------------------*/
void processInput(GLFWwindow* window, double dt) 
{
	// WYJåCIE Z PROGRAMU
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	//RESETOWANIE SZYBKOåCI RAKIETEK 
	velocityForRackets[0] = 0.0f;
	velocityForRackets[1] = 0.0f;

	// STEROWANIE RAKIETKAMI, NADAWNIE IM PR DKOåCI
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		if (offsetsForRackets[0].y < WIN_HEIGHT - racketLimit)
		{
			velocityForRackets[0] = racketsSpeed;
		}
		else 
		{
			offsetsForRackets[0].y = WIN_HEIGHT - racketLimit;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		if (offsetsForRackets[0].y > racketLimit)
		{
			velocityForRackets[0] = -racketsSpeed;
		}
		else
		{
			offsetsForRackets[0].y = halfRacketsHeight;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		if (offsetsForRackets[1].y < WIN_HEIGHT - racketLimit)
		{
			velocityForRackets[1] = racketsSpeed;
		}
		else
		{
			offsetsForRackets[1].y = WIN_HEIGHT - racketLimit;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) 
	{
		if (offsetsForRackets[1].y > racketLimit)
		{
			velocityForRackets[1] = -racketsSpeed;
		}
		else
		{
			offsetsForRackets[1].y = halfRacketsHeight;
		}
	}
}

/*------------------------------------------------------------------------------------------
** funkcja wykonywana przed zamknieciem programu
**------------------------------------------------------------------------------------------*/
void cleanup()
{
	glDeleteBuffers( (GLsizei)buffers.size(), buffers.data() );  // usuniecie VBO
	glDeleteVertexArrays( (GLsizei)vao.size(), vao.data() ); // usuniecie VAO
	glDeleteVertexArrays((GLsizei)ebos.size(), ebos.data()); // usuniecie EBO
	glDeleteProgram( shaderProgram ); // usuniecie programu cieniowania
}

/*------------------------------------------------------------------------------------------
** funkcja inicjujaca ustawienia OpenGL
**------------------------------------------------------------------------------------------*/
void initGL()
{
	std::cout << "GLEW = " << glewGetString( GLEW_VERSION ) << std::endl;
	std::cout << "GL_VENDOR = " << glGetString( GL_VENDOR ) << std::endl;
	std::cout << "GL_RENDERER = " << glGetString( GL_RENDERER ) << std::endl;
	std::cout << "GL_VERSION = " << glGetString( GL_VERSION ) << std::endl;
	std::cout << "GLSL = " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << std::endl;

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f ); // kolor (RGBA) uzywany do czyszczenia bufora koloru

	setupShaders();

	setupBuffers();
}

/*------------------------------------------------------------------------------------------
** funkcja tworzaca program cieniowania skladajacy sie z shadera wierzcholkow i fragmentow
**------------------------------------------------------------------------------------------*/
void setupShaders()
{
	if( !setupShaders( "shaders/vertex.vert", "shaders/fragment.frag", shaderProgram ) )
		exit( 3 );

	vertexLoc = glGetAttribLocation( shaderProgram, "vPosition" );
	colorLoc = glGetAttribLocation( shaderProgram, "vColor" );
}

/*------------------------------------------------------------------------------------------
** funkcja inicjujaca VAO oraz zawarte w nim VBO z danymi o modelu
**------------------------------------------------------------------------------------------*/
void setupBuffers()
{
	// Stworzenie tablicy wierzecho≥kÛw i indeksÛw dla pi≥ki
	generateCircleArray(verticesForBall, indicesForBall, numOfTraingles, 1.0f);

	glGenVertexArrays( (GLsizei)vao.size(), vao.data() ); // generowanie identyfikatorÛw VAO
	glGenBuffers( (GLsizei)buffers.size(), buffers.data() ); // generowanie identyfikatorÛw VBO
	glGenBuffers((GLsizei)ebos.size(), ebos.data()); // generowanie identyfikatorÛw VBO

/*------------------------------------------------------------------------------------------
** PI£ECZKA
**------------------------------------------------------------------------------------------*/

	glBindVertexArray(vao[0]);

	// VBO dla wierzcho≥kÛw dla pi≥ki
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, 2 * (numOfTraingles + 1) * sizeof(float), verticesForBall, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(0 * sizeof(float))); 
	glEnableVertexAttribArray(0); // wlaczenie tablicy atrybutu wierzcholka - wspolrzedne

	// VBO dla pozycji na ekranie dla pi≥ki
	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, 1 * sizeof(vec2), &offsetsForBall, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(0 * sizeof(float))); 
	glEnableVertexAttribArray(1); // wlaczenie tablicy atrybutu wierzcholka - wspolrzedne
	glVertexAttribDivisor(1, 1);

	// VBO dla wielkoúci pi≥ki
	glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
	glBufferData(GL_ARRAY_BUFFER, 1 * sizeof(vec2), &sizesForBall, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(0 * sizeof(float))); 
	glEnableVertexAttribArray(2); 
	glVertexAttribDivisor(2, 1);

	// EBO dla indeksÛw dla pi≥ki
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebos[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * numOfTraingles * sizeof(unsigned int), indicesForBall, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

/*------------------------------------------------------------------------------------------
** RAKIETKI
**------------------------------------------------------------------------------------------*/

	glBindVertexArray(vao[1]);

	// VBO dla wierzcho≥kÛw dla rakietek
	glBindBuffer(GL_ARRAY_BUFFER, buffers[3]);
	glBufferData(GL_ARRAY_BUFFER, 2 * 4 * sizeof(float), verticesForRackets, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[3]);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(0 * sizeof(float))); 
	glEnableVertexAttribArray(0); 

	// VBO dla pozycji na ekranie dla rakietek
	glBindBuffer(GL_ARRAY_BUFFER, buffers[4]);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(vec2), offsetsForRackets, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[4]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(0 * sizeof(float))); 
	glEnableVertexAttribArray(1); 
	glVertexAttribDivisor(1, 1);

	// VBO dla wielkoúci dla rakietek
	glBindBuffer(GL_ARRAY_BUFFER, buffers[5]);
	glBufferData(GL_ARRAY_BUFFER,  1 * sizeof(vec2), &sizesForRackets, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[5]);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(0 * sizeof(float))); 
	glEnableVertexAttribArray(2);
	glVertexAttribDivisor(2, 2);

	// EBO dla indeksÛw dla dla rakietek
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebos[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * 4 * sizeof(unsigned int), indicesForRackets, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

/*------------------------------------------------------------------------------------------
** funkcja rysujaca scene
**------------------------------------------------------------------------------------------*/
void renderScene()
{
	// wyrysowanie pi≥eczki 
	glBindVertexArray(vao[0]);
	glDrawElementsInstanced(GL_TRIANGLES, 3 * numOfTraingles, GL_UNSIGNED_INT, (void*)0, 1);

	// wyrysowanie rakietek 
	glBindVertexArray(vao[1]);
	glDrawElementsInstanced(GL_TRIANGLES, 3 * 2, GL_UNSIGNED_INT, (void*)0, 2);
}

/*------------------------------------------------------------------------------------------
** funkcja tworzπca rzut prostkπtny
**------------------------------------------------------------------------------------------*/
void setOrthographicProjection(int shaderProgram, float left, float right, float bottom, float top, float near, float far) 
{
	// macierz z wyliczonymi przekszt≥aceniami
	float mat[4][4] = {
		{ 2.0f / (right - left), 0.0f, 0.0f, 0.0f },
		{ 0.0f, 2.0f / (top - bottom), 0.0f, 0.0f },
		{ 0.0f, 0.0f, -2.0f / (far - near), 0.0f },
		{ -(right + left)/ (right - left), -(top + bottom) / (top - bottom), -(far + near) / (far - near), 1.0f }
	};

	glUseProgram(shaderProgram); // wlaczenie programu cieniowania
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &mat[0][0]); //przekszta≥cenie w shaderze wierzcho≥kÛw
}

/*------------------------------------------------------------------------------------------
** funkcja tworzπca tablice wierzcho≥kÛw oraz indeksÛw dla pi≥eczki
**------------------------------------------------------------------------------------------*/

void generateCircleArray(float*& vertices, unsigned int*& indices, unsigned int numOfTriangles, float radius = 0.5f)
{
	vertices = new float[(numOfTriangles + 1) * 2];

	// pierwszy wierzcho≥ek
	vertices[0] = 0.0f;
	vertices[1] = 0.0f;

	indices = new unsigned int[numOfTriangles * 3];

	float pi = 4 * atanf(1.0f);
	float theta = 0.0f;

	for(unsigned int i = 0; i < numOfTriangles; i++) 
	{
		// pozycje wierzcho≥kÛw trÛjkπtÛw tworzπcych pi≥eczkÍ
		vertices[(i + 1) * 2] = radius * cosf(theta);
		vertices[(i + 1) * 2 + 1] = radius * sinf(theta);

		// indeksy wierzcho≥kÛw trÛjkπtÛw tworzπcych pi≥eczkÍ
		indices[i * 3] = 0;
		indices[i * 3 + 1] = i + 1;
		indices[i * 3 + 2] = i + 2;

		/*
			0	1	2
			0	2	3
			0	3	4
			0	4	5
			...
			0	20	21 -> ostatni indeks trzeba zamieniÊ na 1, øeby ko≥o siÍ zamknÍ≥o.
		*/

		theta += 2 * pi / numOfTriangles;
	}

	// ostatni indeks
	indices[(numOfTriangles - 1) * 3 + 2] = 1;

}

/*------------------------------------------------------------------------------------------
** funkcja wyúwietlajπca punkty
**------------------------------------------------------------------------------------------*/
void displayScore() {
	std::cout << scoreForLeft << " - " << scoreForRight << std::endl;
}

/*------------------------------------------------------------------------------------------
** funkcja ustalajπca kierunek pi≥eczki
**------------------------------------------------------------------------------------------*/
void ballDirection(unsigned int direction, float x, float yMin, float yMax) {
	
	if (direction == 1) {
		velocityForBall.x = x;
	}
	else if(direction == 2)
	{
		velocityForBall.x = -x;
	}
	else 
	{
		int randomDir = rand() % 2 + 1;
		if (randomDir == 1) 
		{
			velocityForBall.x = x;
		}
		else 
		{
			velocityForBall.x = -x;
		}
	}

	velocityForBall.y = ((yMax - yMin) * ((((float)rand()) / (float)RAND_MAX))) + yMin;
}