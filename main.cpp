#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <time.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include "GL\glew.h"
#include "GL\freeglut.h"
#include "shaderLoader.h"
#include "tekstura.h"

#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp" 
#include "glm/gtc/matrix_transform.hpp" 

using namespace std;

//Wymiary okna
int screen_width = 1280;
int screen_height = 720;

int menu_id;
int menu_choice = 1;

int pozycjaMyszyX; // na ekranie
int pozycjaMyszyY;
int mbutton; // wcisiety klawisz myszy

double kameraX = 60.0;
double kameraZ = 30.0;
double kameraD = -3.0;
double kameraPredkosc;
double kameraKat = 20;
double kameraPredkoscObrotu;
double poprzednie_kameraX;
double poprzednie_kameraZ;
double poprzednie_kameraD;

double rotation = 0;


//macierze
glm::mat4 MV; //modelview - macierz modelu i świata
glm::mat4 P;  //projection - macierz projekcji, czyli naszej perspektywy
glm::vec3 lightPos(0.0f, 500.0f, 0.0f);
GLuint lightColor_id = 0;
GLuint lightPos_id = 0;
GLuint viewPos_id = 0;

GLuint lightColor_id2 = 0;
GLuint lightPos_id2 = 0;
GLuint viewPos_id2 = 0;

GLuint lightColor_id3 = 0;
GLuint lightPos_id3 = 0;
GLuint viewPos_id3 = 0;

std::vector< glm::vec3 > vertices;
std::vector< glm::vec2 > uvs;
std::vector< glm::vec3 > normals;
std::vector< glm::vec3 > colors;
std::vector< glm::vec3 > gray_colors;

//shaders
GLuint programID1 = 0;
GLuint programID2 = 0;
GLuint programID3 = 0;
GLuint programID4 = 0;

unsigned int VBO, VBO_color, VBO_gray_color, VBO_normals, terrainVAO, vtex;
GLuint tex_id0;
GLint uniformTex0;

void calcNormal(float v[3][3], float out[3]) {

	float v1[3], v2[3];
	static const int x = 0; static const int y = 1; static const int z = 2;

	v1[x] = v[0][x] - v[1][x];
	v1[y] = v[0][y] - v[1][y];
	v1[z] = v[0][z] - v[1][z];

	v2[x] = v[1][x] - v[2][x];
	v2[y] = v[1][y] - v[2][y];
	v2[z] = v[1][z] - v[2][z];

	out[x] = v1[y] * v2[z] - v1[z] * v2[y];
	out[y] = v1[z] * v2[x] - v1[x] * v2[z];
	out[z] = v1[x] * v2[y] - v1[y] * v2[x];

}

void load(const char* filename, std::vector< glm::vec3 >& vertices, std::vector< glm::vec2 >& uvs) {
	std::vector< GLint > vertexIndices, uvIndices, normalIndices;

	std::vector< glm::vec3 > temp_vertices;
	std::vector< glm::vec2 > temp_uvs;

	GLint temp_glint = 0;

	string line;
	stringstream ss;
	string prefix;

	ifstream fptr(filename);

	if (fptr.fail())
	{
		printf("Blad w otwarciu pliku!\n");
		exit(1);
	}

	int counter = 0;
	while (getline(fptr, line)) {

		ss.clear();
		ss.str(line);
		ss >> prefix;

		if (prefix == "v") {
			glm::vec3 vertex;
			ss >> vertex.x >> vertex.y >> vertex.z;
			temp_vertices.push_back(vertex);
		}
		else if (prefix == "vt") {
			glm::vec2 uv;
			ss >> uv.x >> uv.y;
			temp_uvs.push_back(uv);
		}
		else if (prefix == "f") {
			counter = 0;
			while (ss >> temp_glint) {
				if (counter == 0)
					vertexIndices.push_back(temp_glint);
				else if (counter == 1)
					uvIndices.push_back(temp_glint);

				if (ss.peek() == '/') {
					++counter;
					ss.ignore(1, '/');
				}
				else if (ss.peek() == ' ') {
					counter = 0;
					ss.ignore(1, ' ');
				}
			}
		}
	}
	fptr.close();

	for (unsigned int i = 0; i < vertexIndices.size(); i++) {
		unsigned int vertexIndex = vertexIndices[i];
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		vertices.push_back(vertex);
	}
	for (unsigned int i = 0; i < uvIndices.size(); i++) {
		unsigned int uvIndex = uvIndices[i];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		uvs.push_back(uv);
	}

}

void colorRange(std::vector< glm::vec3 >& vertices, std::vector< glm::vec3 >& colors) {
	vector <float> heights;
	vector<float>::iterator it;
	std::vector< glm::vec3 > height_color;

	for (unsigned int i = 0; i < vertices.size(); i++) {
		heights.push_back(vertices[i].y);
	}

	sort(heights.begin(), heights.end());
	it = unique(heights.begin(), heights.end());
	heights.resize(distance(heights.begin(), it));

	float r, g, b;
	float h_len = heights.size();
	for (float j = 0; j < h_len; j++) {
		if (j <= h_len / 4) {
			r = 0.0f;
			g = 0.0f + (j / (h_len / 4));
			b = 1.0f;
		}
		else if (j > h_len / 4 && j <= (h_len / 4) * 2) {
			r = 0.0f;
			g = 1.0f;
			b = (-1) * ((j / (h_len / 4)) - 2.0f);
		}
		else if (j > (h_len / 4) * 2 && j <= (h_len / 4) * 3) {
			r = (j / (h_len / 4)) - 2.0f;
			g = 1.0f;
			b = 0.0f;
		}
		else if (j > (h_len / 4) * 3 && j <= h_len) {
			r = 1.0f;
			g = (-1) * ((j / (h_len / 4)) - 4.0f);
			b = 0.0f;
		}
		height_color.push_back(glm::vec3(r, g, b));
	}

	for (unsigned int k = 0; k < vertices.size(); k++) {
		for (unsigned int l = 0; l < height_color.size(); l++) {
			if (vertices[k].y == heights[l]) {
				colors.push_back(height_color[l]);
				break;
			}
		}
	}
}

void mysz(int button, int state, int x, int y) {
	mbutton = button;
	switch (state) {
	case GLUT_UP:
		break;
	case GLUT_DOWN:
		pozycjaMyszyX = x;
		pozycjaMyszyY = y;
		poprzednie_kameraX = kameraX;
		poprzednie_kameraZ = kameraZ;
		poprzednie_kameraD = kameraD;
		break;
	}
}

void mysz_ruch(int x, int y) {
	if (mbutton == GLUT_LEFT_BUTTON) {
		kameraX = poprzednie_kameraX - (pozycjaMyszyX - x) * 0.1;
		kameraZ = poprzednie_kameraZ - (pozycjaMyszyY - y) * 0.1;
	}
	if (mbutton == GLUT_RIGHT_BUTTON) {
		kameraD = poprzednie_kameraD + (pozycjaMyszyY - y) * 0.1;
	}
}


void klawisz(GLubyte key, int x, int y) {
	switch (key) {
	case 'w':
		lightPos[0] += 10;
		break;
	case 's':
		lightPos[0] += -10;
		break;
	case 'a':
		lightPos[2] += -10;
		break;
	case 'd':
		lightPos[2] += 10;
		break;
	}
}

void rysuj1(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Kasowanie ekranu
	GLfloat color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glClearBufferfv(GL_COLOR, 0, color);

	glUseProgram(programID1); //u┐yj programu, czyli naszego shadera	

	MV = glm::mat4(1.0f);  //macierz jednostkowa
	MV = glm::translate(MV, glm::vec3(0, 0, kameraD));
	MV = glm::rotate(MV, (float)glm::radians(kameraZ), glm::vec3(1, 0, 0));
	MV = glm::rotate(MV, (float)glm::radians(kameraX), glm::vec3(0, 1, 0));
	glm::mat4 MVP = P * MV;
	GLuint MVP_id = glGetUniformLocation(programID1, "MVP"); // pobierz lokalizację zmiennej 'uniform' "MV" w programie
	glUniformMatrix4fv(MVP_id, 1, GL_FALSE, &(MVP[0][0]));	   // wyślij tablicę mv do lokalizacji "MV", która jest typu mat4	
	glUniformMatrix4fv(programID1, 1, GL_FALSE, &(MVP[0][0]));

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	glFlush();
	glutSwapBuffers();

}

void rysuj2(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Kasowanie ekranu
	GLfloat color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glClearBufferfv(GL_COLOR, 0, color);

	glUseProgram(programID2); //u┐yj programu, czyli naszego shadera	

	MV = glm::mat4(1.0f);  //macierz jednostkowa
	MV = glm::translate(MV, glm::vec3(0, 0, kameraD));
	MV = glm::rotate(MV, (float)glm::radians(kameraZ), glm::vec3(1, 0, 0));
	MV = glm::rotate(MV, (float)glm::radians(kameraX), glm::vec3(0, 1, 0));
	glm::mat4 MVP = P * MV;
	GLuint MVP_id = glGetUniformLocation(programID2, "MVP"); // pobierz lokalizację zmiennej 'uniform' "MV" w programie
	glUniformMatrix4fv(MVP_id, 1, GL_FALSE, &(MVP[0][0]));	   // wyślij tablicę mv do lokalizacji "MV", która jest typu mat4	

	glUniform3f(lightColor_id, 1.0f, 1.0f, 1.0f);
	glUniform3f(lightPos_id, lightPos[0], lightPos[1], lightPos[2]);

	glUniformMatrix4fv(programID2, 1, GL_FALSE, &(MVP[0][0]));

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	glFlush();
	glutSwapBuffers();

}

void rysuj3(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Kasowanie ekranu
	GLfloat color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glClearBufferfv(GL_COLOR, 0, color);

	glUseProgram(programID3); //u┐yj programu, czyli naszego shadera	

	MV = glm::mat4(1.0f);  //macierz jednostkowa
	MV = glm::translate(MV, glm::vec3(0, 0, kameraD));
	MV = glm::rotate(MV, (float)glm::radians(kameraZ), glm::vec3(1, 0, 0));
	MV = glm::rotate(MV, (float)glm::radians(kameraX), glm::vec3(0, 1, 0));
	glm::mat4 MVP = P * MV;
	GLuint MVP_id = glGetUniformLocation(programID3, "MVP"); // pobierz lokalizację zmiennej 'uniform' "MV" w programie
	glUniformMatrix4fv(MVP_id, 1, GL_FALSE, &(MVP[0][0]));	   // wyślij tablicę mv do lokalizacji "MV", która jest typu mat4	

	glUniform3f(lightColor_id2, 1.0f, 1.0f, 1.0f);
	glUniform3f(lightPos_id2, lightPos[0], lightPos[1], lightPos[2]);

	glUniformMatrix4fv(programID3, 1, GL_FALSE, &(MVP[0][0]));

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	glFlush();
	glutSwapBuffers();

}

void rysuj4(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Kasowanie ekranu
	GLfloat color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glClearBufferfv(GL_COLOR, 0, color);

	glUseProgram(programID4); //u┐yj programu, czyli naszego shadera	

	MV = glm::mat4(1.0f);  //macierz jednostkowa
	MV = glm::translate(MV, glm::vec3(0, 0, kameraD));
	MV = glm::rotate(MV, (float)glm::radians(kameraZ), glm::vec3(1, 0, 0));
	MV = glm::rotate(MV, (float)glm::radians(kameraX), glm::vec3(0, 1, 0));
	glm::mat4 MVP = P * MV;
	GLuint MVP_id = glGetUniformLocation(programID4, "MVP"); // pobierz lokalizację zmiennej 'uniform' "MV" w programie
	glUniformMatrix4fv(MVP_id, 1, GL_FALSE, &(MVP[0][0]));	   // wyślij tablicę mv do lokalizacji "MV", która jest typu mat4	

	glUniform3f(lightColor_id3, 1.0f, 1.0f, 1.0f);
	glUniform3f(lightPos_id3, lightPos[0], lightPos[1], lightPos[2]);

	glUniformMatrix4fv(programID4, 1, GL_FALSE, &(MVP[0][0]));

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_id0);
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	glUniform1i(uniformTex0, 0);

	glFlush();
	glutSwapBuffers();

}

void rozmiar(int width, int height) {
	screen_width = width;
	screen_height = height;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, screen_width, screen_height);

	P = glm::perspective(glm::radians(60.0f), (GLfloat)screen_width / (GLfloat)screen_height, 1.0f, 1000.0f);

	glutPostRedisplay(); // Przerysowanie sceny
}

void idle() {
	glutPostRedisplay();
}

void timer(int value) {
	glutTimerFunc(20, timer, 0);
}

void myDisplay() {
	switch (menu_choice) {
	case 1:
		rysuj1();
		break;
	case 2:
		rysuj2();
		break;
	case 3:
		rysuj3();
		break;
	case 4:
		rysuj4();
		break;
	case 5:
		exit(0);
	}
}

void myMenu(int id) {
	switch (id) {
	case 1:
		menu_choice = 1;
		break;
	case 2:
		menu_choice = 2;
		break;
	case 3:
		menu_choice = 3;
		break;
	case 4:
		menu_choice = 4;
		break;
	case 5:
		exit(0);
		break;
	}
}


int main(int argc, char** argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(screen_width, screen_height);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("OBJModel Loader");

	glewInit(); //init rozszerzeszeń OpenGL z biblioteki GLEW
	glutIdleFunc(idle);			// def. funkcji rysuj¦cej w czasie wolnym procesoora (w efekcie: ci¦gle wykonywanej)
	glutReshapeFunc(rozmiar); // def. obs-ugi zdarzenia resize (GLUT)

	glutKeyboardFunc(klawisz);		// def. obsługi klawiatury
	glutMouseFunc(mysz); 		// def. obsługi zdarzenia przycisku myszy (GLUT)
	glutMotionFunc(mysz_ruch); // def. obsługi zdarzenia ruchu myszy (GLUT)

	glutDisplayFunc(myDisplay);
	menu_id = glutCreateMenu(myMenu);
	glutAddMenuEntry("Gray grid map", 1);
	glutAddMenuEntry("Gray surface map", 2);
	glutAddMenuEntry("Heightmap", 3);
	glutAddMenuEntry("Texture map", 4);
	glutAddMenuEntry("Exit", 5);
	glutAttachMenu(GLUT_MIDDLE_BUTTON);

	glutTimerFunc(20, timer, 0);

	glEnable(GL_DEPTH_TEST);

	load("./icelandicmountain/IcelandicMountain.obj", vertices, uvs);

	/*unsigned int i = 0;
	while (i < vertices.size()) {
		float temp_normal[3];
		float p[3][3] = { {vertices[i].x, vertices[i].y, vertices[i].z}, {vertices[i + 1].x, vertices[i + 1].y, vertices[i + 1].z}, {vertices[i + 3].x, vertices[i + 3].y, vertices[i + 3].z} };
		calcNormal(p, temp_normal);
		for (unsigned int j = 0; j < 4; j++) {
			normals.push_back(glm::vec3(temp_normal[0], temp_normal[1], temp_normal[2]));
		}
		i = i + 4;
	}*/
	normals = vertices;

	colorRange(vertices, colors);

	for (unsigned int i = 0; i < vertices.size(); i++) {
		gray_colors.push_back(glm::vec3(0.5f, 0.5f, 0.5f));
	}

	programID1 = loadShaders("vertex_shader.glsl", "fragment_shader1.glsl");
	programID2 = loadShaders("vertex_shader.glsl", "fragment_shader2.glsl");
	programID3 = loadShaders("vertex_shader.glsl", "fragment_shader3.glsl");
	programID4 = loadShaders("vertex_shader.glsl", "fragment_shader4.glsl");

	lightColor_id = glGetUniformLocation(programID2, "lightColor");
	lightPos_id = glGetUniformLocation(programID2, "lightPos");

	lightColor_id2 = glGetUniformLocation(programID3, "lightColor");
	lightPos_id2 = glGetUniformLocation(programID3, "lightPos");

	lightColor_id3 = glGetUniformLocation(programID4, "lightColor");
	lightPos_id3 = glGetUniformLocation(programID4, "lightPos");

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &VBO_gray_color);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_gray_color);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(gray_colors[0]), &gray_colors[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &VBO_color);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_color);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(colors[0]), &colors[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(2);

	glGenBuffers(1, &VBO_normals);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_normals);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(normals[0]), &normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(3);

	tex_id0 = WczytajTeksture("./icelandicmountain/iceberg.bmp");
	if (tex_id0 == -1)
	{
		MessageBox(NULL, "Nie znaleziono pliku z teksturą", "Problem", MB_OK | MB_ICONERROR);
		exit(0);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_id0);
	uniformTex0 = glGetUniformLocation(programID4, "tex0");
	glUniform1i(uniformTex0, 0);

	glGenBuffers(1, &vtex);
	glBindBuffer(GL_ARRAY_BUFFER, vtex);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(uvs[0]), &uvs[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vtex);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glutMainLoop();

	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &VBO_normals);
	glDeleteBuffers(1, &VBO_color);
	glDeleteBuffers(1, &VBO_gray_color);
	glDeleteBuffers(1, &vtex);
	return(0);
}