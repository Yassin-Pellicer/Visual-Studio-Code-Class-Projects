#define PROYECTO "Geometria"

#include <iostream>	
#include <codebase.h>
#include <vector>

using namespace cb;
static GLuint engranaje;

//Funcion de muestreo de una circunferencia
vector<Vec3> puntosCircunferencia(float radio, int numPuntos, float desfase = 0)
{
	vector<Vec3> puntos;
	for (float angulo = desfase; angulo < 2 * PI + desfase; angulo += 2 * PI / numPuntos)
		puntos.push_back(Vec3(radio * cos(angulo), radio * sin(angulo), 0));
	return puntos;
}

void gear(int nPuntos, float desfase, float pico, float valle, float eje) 
{
	vector<Vec3> pExterior = puntosCircunferencia(valle, nPuntos, desfase - PI / nPuntos);
	vector<Vec3> pInterior = puntosCircunferencia(eje, nPuntos, desfase);
	vector<Vec3> pFar = puntosCircunferencia(pico, nPuntos, desfase);

	engranaje = glGenLists(1);
	glNewList(engranaje, GL_COMPILE);

	for (int i = 0; i < nPuntos; i++) {
		glBegin(GL_TRIANGLE_STRIP);
		glVertex3fv(pInterior[i % nPuntos]);
		glVertex3fv(pExterior[i % nPuntos]);
		glVertex3fv(pExterior[(i + 1) % nPuntos]);
		glVertex3fv(pFar[i % nPuntos]);
		glEnd();
	}
	glEndList();
}

void init()
{
	gear(27, PI / 2, 0.9, 0.4, 0.2);
}

// Funcion de atencion al evento de dibujo
void display()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Dibujo cuerpo engranaje */
	glPolygonMode(GL_FRONT, GL_FILL);
	glColor3fv(ROJO);
	glCallList(engranaje);

	/* Dibujo outline engranaje*/
	glPolygonMode(GL_FRONT, GL_LINE);
	glColor3fv(BLANCO);
	glLineWidth(1);
	glCallList(engranaje);

	/* Dibujo ejes */
	glBegin(GL_LINES);
	glVertex2f(-1.0, 0);
	glVertex2f(1.0, 0);
	glVertex2f(0, -1.0);
	glVertex2f(0, 1.0);
	glEnd();

	glFlush();
}

// Funcion de atencion al redimensionamiento
void reshape(GLint w, GLint h)
{
}

int main(int argc, char** argv)
{
	// Inicializaciones
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(50, 50);

	// Crear ventana
	glutCreateWindow(PROYECTO);

	// Funcion de inicializacion propia
	init();

	// Registrar callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	// Bucle de atencion a eventos
	glutMainLoop();
}
