constexpr auto PROYECTO = "PROYECTO";

#include <iostream>	
#include <codebase.h>
#include <vector>
#include <math.h>

using namespace cb;
static GLuint wheel;
static GLuint axis;
static GLuint cabins;
static GLuint transmission_gear;
static GLuint motor_gear;

float lastX = 0, lastY = 0;
bool isDragging = false;

static float rotationAngle = 0.0f;
float rotationSpeed = 0.0f; int fps = 60;
float angle = 0.0f;

struct CameraPosition {
	float posX, posY, posZ;
	float frontX, frontY, frontZ;
};

CameraPosition cameraPositions[] = {
	{0.790621f, 1.32f, 1.02143f, -0.610145f, -0.924547f, -0.79229f},
	{0.447824f, -0.78f, 0.300406f, -0.921863f, 0.397148f, -0.387515f},
	{0.790621f, -0.84f, 1.02143f, -0.576432f, 0.453991f, -0.817145f},
	{-1.34703f, -0.06f, 0.649987f, 0.879646f, 0.152986f, -0.475628f},
	{0.00280719f, -0.45f, 2.29942f, -0.0104735f, 0.152986f, -0.999945f},
	{0.0138368f, -0.36f, 0.696931f, 0.0209415f, 0.999848f, -0.999781f},
	{-1.20792f, 1.05f, 0.402992f, 0.951055f, -0.692142f, -0.309021f},
	{-0.954297f, -0.57f, -0.951878f, 0.726575f, 0.523986f, 0.687088f}
};

CameraPosition original[] = {
	{0.790621f, 1.32f, 1.02143f, -0.610145f, -0.924547f, -0.79229f},
	{0.447824f, -0.78f, 0.300406f, -0.921863f, 0.397148f, -0.387515f},
	{0.790621f, -0.84f, 1.02143f, -0.576432f, 0.453991f, -0.817145f},
	{-1.34703f, -0.06f, 0.649987f, 0.879646f, 0.152986f, -0.475628f},
	{0.00280719f, -0.45f, 2.29942f, -0.0104735f, 0.152986f, -0.999945f},
	{0.0138368f, -0.36f, 0.696931f, 0.0209415f, 0.999848f, -0.999781f},
	{-1.20792f, 1.05f, 0.402992f, 0.951055f, -0.692142f, -0.309021f},
	{-0.954297f, -0.57f, -0.951878f, 0.726575f, 0.523986f, 0.687088f}
};

float skyRed = 0.05f;
float skyGreen = 0.05f;
float skyBlue = 0.2f;

float red = 1;
float green = 1;
float blue = 1;

float lastTime = 0.0f;
float interval = 2.0f;

float cameraLastTime = 0.0f;
float cameraInterval = 10.0f;
int camindex = 0;

bool canUpdate = false;
bool done = false;
bool inputDetected = false;

bool keys[256];

vector<Vec3> puntosCircunferencia(float radio, int numPuntos, float desfase, float Z)
{
	vector<Vec3> puntos;
	for (float angulo = desfase; angulo < 2 * PI + desfase; angulo += 2 * PI / numPuntos)
		puntos.push_back(Vec3(radio * cos(angulo), radio * sin(angulo), Z));
	return puntos;
}

void gear3D(int nPuntos, float desfase, float pico, float valle, float eje, float Z, GLuint item) {

	vector<Vec3> pValle = puntosCircunferencia(valle, nPuntos, desfase - PI / nPuntos, Z);
	vector<Vec3> pEje = puntosCircunferencia(eje, nPuntos, desfase, Z);
	vector<Vec3> pPico = puntosCircunferencia(pico, nPuntos, desfase, Z);

	vector<Vec3> pEje2 = puntosCircunferencia(eje, nPuntos, desfase, Z + 0.025f);
	vector<Vec3> pValle2 = puntosCircunferencia(valle, nPuntos, desfase - PI / nPuntos, Z + 0.025f);
	vector<Vec3> pPico2 = puntosCircunferencia(pico, nPuntos, desfase, Z + 0.025f);

	item = glGenLists(1);
	glNewList(item, GL_COMPILE);
	glColor3f(0.8f, 0.5f, 0.0f);
	for (int i = 0; i < nPuntos; i++) {
		glBegin(GL_TRIANGLE_STRIP);
		glVertex3fv(pValle[(i + 1) % nPuntos]);
		glVertex3fv(pPico[i % nPuntos]);
		glVertex3fv(pEje[i % nPuntos]);
		glVertex3fv(pValle[(i) % nPuntos]);
		glVertex3fv(pEje[(i - 1 + nPuntos) % nPuntos]);
		glEnd();

	}

	for (int i = 0; i < nPuntos; i++) {
		glBegin(GL_TRIANGLE_STRIP);
		glVertex3fv(pValle2[(i + 1) % nPuntos]);
		glVertex3fv(pPico2[i % nPuntos]);
		glVertex3fv(pEje2[i % nPuntos]);
		glVertex3fv(pValle2[(i) % nPuntos]);
		glVertex3fv(pEje2[(i - 1 + nPuntos) % nPuntos]);
		glEnd();
	}

	glColor3f(1.0f, 1.0f, 0.0f);
	for (int i = 0; i < nPuntos; i++) {
		int nextIndex = (i + 1) % nPuntos;

		quad(pValle[nextIndex], pValle2[nextIndex], pPico2[i], pPico[i]);
		quad(pValle[nextIndex], pValle2[nextIndex], pPico2[nextIndex], pPico[nextIndex]);

		quad(pEje[nextIndex], pEje2[nextIndex], pEje2[i], pEje[i]);
		glEnd();
	}

	for (int i = 0; i < nPuntos; i++) {
		int nextIndex = (i + 1) % nPuntos;

		quad(pValle[nextIndex], pValle2[nextIndex], pPico2[i], pPico[i]);
		quad(pValle[nextIndex], pValle2[nextIndex], pPico2[nextIndex], pPico[nextIndex]);

		quad(pEje[nextIndex], pEje2[nextIndex], pEje2[i], pEje[i]);
		glEnd();
	}


}

vector<Vec3> cabin_points = puntosCircunferencia(0.05 * 15, 18, 0, 0.0);


void draw_cube(float size) {
	glBegin(GL_QUADS);

	// Front face
	glVertex3f(-size / 2, -size / 2, size / 2);
	glVertex3f(size / 2, -size / 2, size / 2);
	glVertex3f(size / 2, size / 2, size / 2);
	glVertex3f(-size / 2, size / 2, size / 2);

	// Back face
	glVertex3f(-size / 2, -size / 2, -size / 2);
	glVertex3f(-size / 2, size / 2, -size / 2);
	glVertex3f(size / 2, size / 2, -size / 2);
	glVertex3f(size / 2, -size / 2, -size / 2);

	// Left face
	glVertex3f(-size / 2, -size / 2, -size / 2);
	glVertex3f(-size / 2, -size / 2, size / 2);
	glVertex3f(-size / 2, size / 2, size / 2);
	glVertex3f(-size / 2, size / 2, -size / 2);

	// Right face
	glVertex3f(size / 2, -size / 2, -size / 2);
	glVertex3f(size / 2, size / 2, -size / 2);
	glVertex3f(size / 2, size / 2, size / 2);
	glVertex3f(size / 2, -size / 2, size / 2);


	// Bottom face
	glVertex3f(-size / 2, -size / 2, size / 2);
	glVertex3f(-size / 2, -size / 2, -size / 2);
	glVertex3f(size / 2, -size / 2, -size / 2);
	glVertex3f(size / 2, -size / 2, size / 2);

	glEnd();

	// WIREFRAMES

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(2.0f);
	glColor3f(1.0f, 1.0f, 0.0f);

	glBegin(GL_QUADS);
	// Front face
	glVertex3f(-size / 2, -size / 2, size / 2);
	glVertex3f(size / 2, -size / 2, size / 2);
	glVertex3f(size / 2, size / 2, size / 2);
	glVertex3f(-size / 2, size / 2, size / 2);

	// Back face
	glVertex3f(-size / 2, -size / 2, -size / 2);
	glVertex3f(-size / 2, size / 2, -size / 2);
	glVertex3f(size / 2, size / 2, -size / 2);
	glVertex3f(size / 2, -size / 2, -size / 2);

	// Left face
	glVertex3f(-size / 2, -size / 2, -size / 2);
	glVertex3f(-size / 2, -size / 2, size / 2);
	glVertex3f(-size / 2, size / 2, size / 2);
	glVertex3f(-size / 2, size / 2, -size / 2);

	// Right face
	glVertex3f(size / 2, -size / 2, -size / 2);
	glVertex3f(size / 2, size / 2, -size / 2);
	glVertex3f(size / 2, size / 2, size / 2);
	glVertex3f(size / 2, -size / 2, size / 2);

	// Bottom face
	glVertex3f(-size / 2, -size / 2, size / 2);
	glVertex3f(-size / 2, -size / 2, -size / 2);
	glVertex3f(size / 2, -size / 2, -size / 2);
	glVertex3f(size / 2, -size / 2, size / 2);

	glEnd();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void draw_cabin(float x, float y, float z, float desfase) {
	float size = 0.10f;

	glPushMatrix();
	glTranslatef(x, y - size / 2, z);
	glColor3f(0.8f, 0.5f, 0.0f);
	draw_cube(size);

	// DECORATION
	glColor3f(1.0f, 1.0f, 1.0f);
	int numPuntos = 64;
	vector<Vec3> circle;
	vector<Vec3> circle2;
	vector<Vec3> circle3;
	vector<Vec3> circle4;

	circle = puntosCircunferencia(0.07, numPuntos, 0, 0);

	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < numPuntos; i++) {
		glVertex3f(circle[i].x, circle[i].y, circle[i].z - size / 2);
	}
	glEnd();

	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < numPuntos; i++) {
		glVertex3f(circle[i].x, circle[i].y, circle[i].z + size / 2);
	}
	glEnd();
	glPopMatrix();

	glPushMatrix();

	glTranslatef(x, y, z);
	glColor3f(1.0f, 1.0f, 0.0f);

	circle = puntosCircunferencia(0.05, numPuntos, 0, 0);

	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < numPuntos; i++) {
		glVertex3f(circle[i].x, circle[i].y, circle[i].z - size / 2 - 0.001);
	}
	glEnd();

	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < numPuntos; i++) {
		glVertex3f(circle[i].x, circle[i].y, circle[i].z + size / 2 + 0.001);
	}
	glEnd();

	circle2 = puntosCircunferencia(0.02, numPuntos, 0, 0);

	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < numPuntos; i++) {
		glVertex3f(circle2[i].x, circle2[i].y, circle2[i].z - size / 2 - 0.001);
	}
	glEnd();

	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < numPuntos; i++) {
		glVertex3f(circle2[i].x, circle2[i].y, circle2[i].z + size / 2 + 0.001);
	}
	glEnd();

	circle3 = puntosCircunferencia(0.06, numPuntos, 0, 0);
	circle4 = puntosCircunferencia(0.08, numPuntos, 0, 0);
	glLineWidth(3.0f);

	glColor3f(red, green, blue);
	glBegin(GL_TRIANGLE_STRIP);
	for (int i = 0; i <= numPuntos; i++) {
		glVertex3f(circle3[i].x, circle3[i].y - 0.03, circle3[i].z - size / 2 - 0.002);
		glVertex3f(circle4[i].x, circle4[i].y - 0.05, circle4[i].z - size / 2 - 0.002);
	}
	glEnd();

	glBegin(GL_TRIANGLE_STRIP);
	for (int i = 0; i <= numPuntos; i++) {
		glVertex3f(circle3[i].x, circle3[i].y - 0.03, circle3[i].z + size / 2 + 0.002);
		glVertex3f(circle4[i].x, circle4[i].y - 0.05, circle4[i].z + size / 2 + 0.002);
	}
	glEnd();

	glColor3f(1.0f, 1.0f, 0.0f);
	glLineWidth(2.0f);

	// ROOF-CONNECTION AND ROOF
	for (int i = 0; i < numPuntos / 2; i++) {
		glBegin(GL_LINES);
		glVertex3f(circle[i].x, circle[i].y, circle[i].z + size / 2); // Back vertex
		glVertex3f(circle[i].x, circle[i].y + 0.05, circle[i].z + size / 2); // Front vertex
		glEnd();

		glBegin(GL_LINES);
		glVertex3f(circle[i].x, circle[i].y, circle[i].z - size / 2); // Back vertex
		glVertex3f(circle[i].x, circle[i].y + 0.05, circle[i].z - size / 2); // Front vertex
		glEnd();
	}

	glBegin(GL_TRIANGLE_STRIP);
	for (int i = 0; i < numPuntos / 2 + 1; i++) {
		glVertex3f(circle[i].x, circle[i].y + 0.05, circle[i].z - size / 2);
		glVertex3f(circle[i].x, circle[i].y + 0.05, circle[i].z + size / 2);
	}
	glEnd();

	glPopMatrix();
}

void draw_cabins(vector<Vec3> points)
{
	for (int i = 0; i < points.size() - 1; i++)
	{
		glPushMatrix();
		draw_cabin(points[i].x, points[i].y, -0.05, 0.5);
		glPopMatrix();
	}
}

void draw_axis(float desfase, int numPuntos, float starting_radio)
{
	// AXIS
	float axis_r = starting_radio;

	axis = glGenLists(1);
	glNewList(axis, GL_COMPILE);

	glColor3f(1.0f, 1.0f, 1.0f);

	vector<Vec3> axis;

	axis = puntosCircunferencia(axis_r, numPuntos, 0, 0);

	glBegin(GL_TRIANGLE_STRIP);
	for (int i = 0; i <= numPuntos; i++) {
		int idx = i % numPuntos;
		glVertex3f(axis[idx].x, axis[idx].y, axis[idx].z - desfase * 2.8);
		glVertex3f(axis[idx].x, axis[idx].y, axis[idx].z + desfase * 1.5);
	}
	glEnd();

	// OUTER SCREWS
	glColor3f(0.8f, 0.5f, 0.0f);

	axis = puntosCircunferencia(axis_r + 0.02, numPuntos, 0, 0);

	glBegin(GL_TRIANGLE_STRIP);
	for (int i = 0; i <= numPuntos; i++) {
		int idx = i % numPuntos;
		glVertex3f(axis[idx].x, axis[idx].y, axis[idx].z + desfase * 0.75);
		glVertex3f(axis[idx].x, axis[idx].y, axis[idx].z + desfase * 1.25);
	}
	glEnd();

	glBegin(GL_TRIANGLE_STRIP);
	for (int i = 0; i <= numPuntos; i++) {
		int idx = i % numPuntos;
		glVertex3f(axis[idx].x, axis[idx].y, axis[idx].z - desfase * 2);
		glVertex3f(axis[idx].x, axis[idx].y, axis[idx].z - desfase * 2.5);
	}
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.0f, 0.0f, axis[0].z + desfase * 0.75);
	for (int i = 0; i <= numPuntos; i++) {
		int idx = i % numPuntos;
		glVertex3f(axis[idx].x, axis[idx].y, axis[idx].z + desfase * 0.75);
	}
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.0f, 0.0f, axis[0].z + desfase * 1.25);
	for (int i = 0; i <= numPuntos; i++) {
		int idx = i % numPuntos;
		glVertex3f(axis[idx].x, axis[idx].y, axis[idx].z + desfase * 1.25);
	}
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.0f, 0.0f, axis[0].z - desfase * 2);
	for (int i = 0; i <= numPuntos; i++) {
		int idx = i % numPuntos;
		glVertex3f(axis[idx].x, axis[idx].y, axis[idx].z - desfase * 2);
	}
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.0f, 0.0f, axis[0].z - desfase * 2.5);
	for (int i = 0; i <= numPuntos; i++) {
		int idx = i % numPuntos;
		glVertex3f(axis[idx].x, axis[idx].y, axis[idx].z - desfase * 2.5);
	}
	glEnd();

	// MOTOR

	glPushMatrix();
	glTranslatef(-starting_radio * 10, -starting_radio * 15, desfase);
	glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
	draw_cabin(0, 0, 0, 0);
	glPopMatrix();

	// SUPPORT
	glBegin(GL_TRIANGLES);

	glVertex3f(0.0f, 0.0f, desfase * 1);
	glVertex3f(-starting_radio * 10, -starting_radio * 18, desfase * 2.2);
	glVertex3f(-starting_radio * 10 + 0.1, -starting_radio * 18, desfase * 2.2);

	glVertex3f(0.0f, 0.0f, desfase * 1);
	glVertex3f(-starting_radio * 10, -starting_radio * 18, desfase * 2.2 - 0.1);
	glVertex3f(-starting_radio * 10 + 0.1, -starting_radio * 18, desfase * 2.2 - 0.1);

	glVertex3f(0.0f, 0.0f, desfase * 1);
	glVertex3f(-starting_radio * 10, -starting_radio * 18, desfase * 2.2);
	glVertex3f(-starting_radio * 10, -starting_radio * 18, desfase * 2.2 - 0.1);

	glVertex3f(0.0f, 0.0f, desfase * 1);
	glVertex3f(-starting_radio * 10 + 0.1, -starting_radio * 18, desfase * 2.2);
	glVertex3f(-starting_radio * 10 + 0.1, -starting_radio * 18, desfase * 2.2 - 0.1);
	glEnd();

	glBegin(GL_TRIANGLES);
	glVertex3f(0.0f, 0.0f, desfase * 1);
	glVertex3f(+starting_radio * 10, -starting_radio * 18, desfase * 2.2);
	glVertex3f(+starting_radio * 10 + 0.1, -starting_radio * 18, desfase * 2.2);

	glVertex3f(0.0f, 0.0f, desfase * 1);
	glVertex3f(+starting_radio * 10, -starting_radio * 18, desfase * 2.2 - 0.1);
	glVertex3f(+starting_radio * 10 + 0.1, -starting_radio * 18, desfase * 2.2 - 0.1);

	glVertex3f(0.0f, 0.0f, desfase * 1);
	glVertex3f(+starting_radio * 10, -starting_radio * 18, desfase * 2.2);
	glVertex3f(+starting_radio * 10, -starting_radio * 18, desfase * 2.2 - 0.1);

	glVertex3f(0.0f, 0.0f, desfase * 1);
	glVertex3f(+starting_radio * 10 + 0.1, -starting_radio * 18, desfase * 2.2);
	glVertex3f(+starting_radio * 10 + 0.1, -starting_radio * 18, desfase * 2.2 - 0.1);
	glEnd();

	glBegin(GL_TRIANGLES);
	glVertex3f(0.0f, 0.0f, -desfase * 2.2);
	glVertex3f(-starting_radio * 10, -starting_radio * 18, -desfase * 2.2);
	glVertex3f(-starting_radio * 10 + 0.1, -starting_radio * 18, -desfase * 2.2);

	glVertex3f(0.0f, 0.0f, -desfase * 2.2);
	glVertex3f(-starting_radio * 10, -starting_radio * 18, -desfase * 2.2 - 0.1);
	glVertex3f(-starting_radio * 10 + 0.1, -starting_radio * 18, -desfase * 2.2 - 0.1);

	glVertex3f(0.0f, 0.0f, -desfase * 2.2);
	glVertex3f(-starting_radio * 10, -starting_radio * 18, -desfase * 2.2);
	glVertex3f(-starting_radio * 10, -starting_radio * 18, -desfase * 2.2 - 0.1);

	glVertex3f(0.0f, 0.0f, -desfase * 2.2);
	glVertex3f(-starting_radio * 10 + 0.1, -starting_radio * 18, -desfase * 2.2);
	glVertex3f(-starting_radio * 10 + 0.1, -starting_radio * 18, -desfase * 2.2 - 0.1);
	glEnd();

	glBegin(GL_TRIANGLES);
	glVertex3f(0.0f, 0.0f, -desfase * 2.2);
	glVertex3f(+starting_radio * 10, -starting_radio * 18, -desfase * 2.2);
	glVertex3f(+starting_radio * 10 + 0.1, -starting_radio * 18, -desfase * 2.2);

	glVertex3f(0.0f, 0.0f, -desfase * 2.2);
	glVertex3f(+starting_radio * 10, -starting_radio * 18, -desfase * 2.2 - 0.1);
	glVertex3f(+starting_radio * 10 + 0.1, -starting_radio * 18, -desfase * 2.2 - 0.1);

	glVertex3f(0.0f, 0.0f, -desfase * 2.2);
	glVertex3f(+starting_radio * 10, -starting_radio * 18, -desfase * 2.2);
	glVertex3f(+starting_radio * 10, -starting_radio * 18, -desfase * 2.2 - 0.1);

	glVertex3f(0.0f, 0.0f, -desfase * 2.2);
	glVertex3f(+starting_radio * 10 + 0.1, -starting_radio * 18, -desfase * 2.2);
	glVertex3f(+starting_radio * 10 + 0.1, -starting_radio * 18, -desfase * 2.2 - 0.1);
	glEnd();

	// FLOOR
	float floorHeight = -starting_radio * 18;
	glBegin(GL_QUADS);
	glColor3f(0.05f, 0.17f, 0.05f);
	// Define the four corners of the floor at the desired height
	glVertex3f(-100.0f, floorHeight, 100.0f);  // Top left corner
	glVertex3f(100.0f, floorHeight, 100.0f);  // Top right corner
	glVertex3f(100.0f, floorHeight, -100.0f); // Bottom right corner
	glVertex3f(-100.0f, floorHeight, -100.0f); // Bottom left corner
	glEnd();

	// PLATFORM
	float deco_o = starting_radio * 20 - 0.05;
	glColor3f(0.7f, 0.7f, 0.8f);
	vector<Vec3> circle = puntosCircunferencia(deco_o, 32, 0, 0);
	glBegin(GL_TRIANGLE_STRIP);
	for (int i = 32 / 2 + 32 / 8; i <= 32 - 32 / 8; i++) {
		glVertex3f(circle[i].x, circle[i].y + 0.05, circle[i].z - 0.35);
		glVertex3f(circle[i].x, circle[i].y + 0.05, circle[i].z + 0.25);
	}
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.8f, 0.5f, 0.0f);
	for (int i = 32 / 2 + 32 / 8; i < 32 - 32 / 8; i++) {
		glVertex3f(circle[i].x, circle[i].y + 0.05, circle[i].z - 0.35);
		glVertex3f(circle[i + 1].x, circle[i + 1].y + 0.05, circle[i + 1].z - 0.35);
		glVertex3f(circle[i + 1].x, circle[i + 1].y - 0.5, circle[i + 1].z - 0.35);
		glVertex3f(circle[i].x, circle[i].y - 0.5, circle[i].z - 0.35);
	}
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.8f, 0.5f, 0.0f);
	for (int i = 32 / 2 + 32 / 8; i < 32 - 32 / 8; i++) {
		glVertex3f(circle[i].x, circle[i].y + 0.05, circle[i].z + 0.25);
		glVertex3f(circle[i + 1].x, circle[i + 1].y + 0.05, circle[i + 1].z + 0.25);
		glVertex3f(circle[i + 1].x, circle[i + 1].y - 0.5, circle[i + 1].z + 0.25);
		glVertex3f(circle[i].x, circle[i].y - 0.5, circle[i].z + 0.25);
	}
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.8f, 0.5f, 0.0f);
	for (int i = 32 / 2 + 32 / 8; i <= 32 - 32 / 8; i++) {
		glVertex3f(circle[i].x, circle[i].y + 0.05, circle[i].z + 0.25);
		glVertex3f(circle[i].x, circle[i].y + 0.05, circle[i].z - 0.35);
		glVertex3f(circle[i].x, circle[i].y - 0.5, circle[i].z - 0.35);
		glVertex3f(circle[i].x, circle[i].y - 0.5, circle[i].z + 0.25);
	}
	glEnd();

	glEndList();
}

void draw_motor_wheel(int nPuntos, float desfase, float pico, float valle, float eje, float Z, float posX, float posY, float posZ) {
	motor_gear = glGenLists(1);

	// Generate and compile display list
	glNewList(motor_gear, GL_COMPILE);

	// Apply the position transformation
	glPushMatrix();  // Save the current matrix
	glTranslatef(posX, posY, posZ);  // Move the gear to the specified position

	// Call gear3D function to draw the motor gear
	gear3D(nPuntos, desfase, pico, valle, eje, Z, motor_gear);
	GLUquadric* quadric = gluNewQuadric();
	float cylinderLength = 0.155f; // Length of the cylinder
	float cylinderRadius = 0.01f; // Radius of the cylinder

	glPushMatrix();
	gluCylinder(quadric, cylinderRadius, cylinderRadius, cylinderLength, 32, 32);
	glPopMatrix();

	gluDeleteQuadric(quadric);

	glPopMatrix();  // Restore the matrix

	glEndList();
}


void draw_wheel(float starting_radio, int numPuntos, float desfase)
{
	float axis_r = starting_radio;
	float axis_m = starting_radio * 6;
	float axis_mt = starting_radio * 11;
	float axis_o = starting_radio * 15;
	float deco_r = starting_radio;
	float deco_o = starting_radio * 16;

	float middle_round = axis_m * 1.40;

	vector<Vec3> axis = puntosCircunferencia(axis_r, numPuntos, 0, 0);
	vector<Vec3> middle = puntosCircunferencia(axis_m, numPuntos, 0, 0);
	vector<Vec3> middle_r = puntosCircunferencia(middle_round, numPuntos, 0, 0);
	vector<Vec3> middle_t = puntosCircunferencia(axis_mt, numPuntos, 0, 0);
	vector<Vec3> outer = puntosCircunferencia(axis_o, numPuntos, 0, 0.0);
	vector<Vec3> decoration_axis = puntosCircunferencia(deco_r, numPuntos * 2, 0, 0);
	vector<Vec3> decoration_outer = puntosCircunferencia(deco_o, numPuntos * 2, 0, 0);

	vector<vector<Vec3>> array = { axis, middle, middle_t, outer };

	wheel = glGenLists(1);
	glNewList(wheel, GL_COMPILE);
	for (const auto& element : array) {

		// WHEELS
		glColor3f(1.0f, 1.0f, 1.0f);

		if (element == array[array.size() - 1])
			glLineWidth(4.0f);
		else
			glLineWidth(1.0f);

		glBegin(GL_LINE_LOOP);
		for (int i = 0; i < numPuntos; i++) {
			glVertex3f(element[i].x, element[i].y, element[i].z);
		}
		glEnd();

		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_LINE_LOOP);
		for (int i = 0; i < numPuntos; i++) {
			glVertex3f(element[i].x, element[i].y, element[i].z - desfase);
		}
		glEnd();

		if (element == array[array.size() - 1]) continue;

		// FRAME
		glBegin(GL_LINES);
		for (int i = 0; i < numPuntos; i++) {
			glVertex3f(element[i].x, element[i].y, element[i].z);
			glVertex3f(element[i].x, element[i].y, element[i].z - desfase);
		}
		glEnd();
	}

	// FRAME
	glColor3f(.5f, .6f, 0.2f);

	glLineWidth(3.0f);
	glBegin(GL_LINES);
	glLineWidth(2.0f);
	for (int i = 0; i < numPuntos; i++) {
		glVertex3f(axis[i].x, axis[i].y, axis[i].z + 0.05);
		glVertex3f(outer[i].x, outer[i].y, outer[i].z);
	}
	glEnd();

	glBegin(GL_LINES);
	glLineWidth(2.0f);
	for (int i = 0; i < numPuntos; i++) {
		glVertex3f(axis[i].x, axis[i].y, axis[i].z - desfase - 0.08);
		glVertex3f(outer[i].x, outer[i].y, outer[i].z - desfase);
	}
	glEnd();

	glColor3f(1.0f, 1.0f, 1.0f);

	glLineWidth(3.0f);

	// CROSS-FRAME
	glBegin(GL_LINES);
	for (int i = 0; i < numPuntos; i++) {
		for (int j = 0; j < array.size() - 1; j++) {
			glVertex3f(array[j][i].x, array[j][i].y, array[j][i].z - desfase);
			glVertex3f(array[j][i + 1].x, array[j][i + 1].y, array[j][i + 1].z);
		}
	}
	glEnd();

	glBegin(GL_LINES);
	for (int i = 0; i < numPuntos; i++) {
		for (int j = 0; j < array.size() - 1; j++) {
			glVertex3f(array[j][i + 1].x, array[j][i + 1].y, array[j][i + 1].z - desfase);
			glVertex3f(array[j][i].x, array[j][i].y, array[j][i].z);
		}
	}
	glEnd();

	glBegin(GL_LINES);
	for (int i = 0; i < numPuntos; i++) {
		for (int j = 0; j < array.size() - 2; j++) {
			glVertex3f(array[j + 1][i].x, array[j + 1][i].y, array[j + 1][i].z - desfase);
			glVertex3f(array[j][i].x, array[j][i].y, array[j][i].z);
		}
	}
	glEnd();

	glBegin(GL_LINES);
	for (int i = 0; i < numPuntos; i++) {
		for (int j = 0; j < array.size() - 2; j++) {
			glVertex3f(array[j][i].x, array[j][i].y, array[j][i].z - desfase);
			glVertex3f(array[j + 1][i].x, array[j + 1][i].y, array[j + 1][i].z);
		}
	}
	glEnd();

	glLineWidth(5.0f);

	// DECORATION
	glBegin(GL_LINES);
	glLineWidth(20.0f);
	for (int i = 0; i < 26; i++) {
		glVertex3f(decoration_axis[i].x, decoration_axis[i].y, decoration_axis[i].z);
		glVertex3f(decoration_outer[i].x, decoration_outer[i].y, decoration_outer[i].z);
	}
	glEnd();

	glBegin(GL_LINES);
	glLineWidth(20.0f);
	for (int i = 0; i < numPuntos * 2; i++) {
		glVertex3f(decoration_axis[i].x, decoration_axis[i].y, decoration_axis[i].z - desfase);
		glVertex3f(decoration_outer[i].x, decoration_outer[i].y, decoration_outer[i].z - desfase);
	}
	glEnd();

	gear3D(420, PI / 2, axis_o + 0.005, axis_o, axis_o, 0.005, transmission_gear);

	glEndList();
}

void updateCamera(int camind)
{
	int cameraIndex = (camind + 1) % 8;  // Loop through the positions

	// Get the current camera position and front vector
	float cameraPosX = cameraPositions[cameraIndex].posX;
	float cameraPosY = cameraPositions[cameraIndex].posY;
	float cameraPosZ = cameraPositions[cameraIndex].posZ;
	float frontX = cameraPositions[cameraIndex].frontX;
	float frontY = cameraPositions[cameraIndex].frontY;
	float frontZ = cameraPositions[cameraIndex].frontZ;

	if (cameraIndex == 5) { cameraPositions[cameraIndex].posZ += 0.0001;  cameraPositions[cameraIndex].frontY -= 0.0007; }
	if (cameraIndex == 4) {
		angle += 0.007f; 
		if (angle >= 360.0f) angle -= 360.0f; 

		float radius = 2.5f;
		cameraPosX = radius * cos(angle); 
		cameraPosZ = radius * sin(angle); 
		cameraPosY = -0.75f; 
		frontX = -cameraPosX; 
		frontY = -cameraPosY;
		frontZ = -cameraPosZ;
	}

	if (cameraIndex == 2){
		angle += 0.005f;
		if (angle >= 360.0f) angle -= 360.0f;

		float radius = 2.0f;
		cameraPosX = radius * cos(angle);
		cameraPosZ = radius * sin(angle);
		cameraPosY = 1.25f;
		frontX = -cameraPosX;
		frontY = -cameraPosY;
		frontZ = -cameraPosZ;
	}

	gluLookAt(cameraPosX, cameraPosY, cameraPosZ,
		cameraPosX + frontX, cameraPosY + frontY, cameraPosZ + frontZ,
		0.0f, 1.0f, 0.0f);
}

void reshape(GLint w, GLint h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (double)w / (double)h, 1.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void changeColor() {
	// Generate random values for RGB (0.0 to 1.0)
	red = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	green = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	blue = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glScalef(5.0f, 5.0f, 5.0f);

	float currentTime = static_cast<float>(glutGet(GLUT_ELAPSED_TIME)) / 1000.0f; // Time in seconds

	if (currentTime - lastTime >= interval && currentTime >= 5) {
		changeColor();
		lastTime = currentTime;
	}

	if (currentTime - cameraLastTime >= cameraInterval) {
		cameraLastTime = currentTime;
		std::memcpy(cameraPositions, original, sizeof(original));
		camindex++;
	}
	updateCamera(camindex);

	draw_cabins(cabin_points);

	glCallList(axis);

	glCallList(transmission_gear);

	glPushMatrix();

	glRotatef(rotationAngle, 0.0f, 0.0f, 1.0f);

	for (int i = 0; i < 18; i++)
	{
		float angle = (rotationAngle + (360.0 / 18.0 * i)) * PI / 180.0;
		cabin_points[i].x = cos(angle) * 0.05 * 15;
		cabin_points[i].y = sin(angle) * 0.05 * 15;
	}

	glCallList(wheel);

	glPopMatrix();

	glTranslatef(-0.5f, -0.644f, -0.005f);

	glRotatef(-rotationAngle * 9, 0.0f, 0.0f, 1.0f);

	glTranslatef(0.5f, 0.644f, 0.005f);

	glCallList(motor_gear);

	glutSwapBuffers();
}

void update(int value)
{
	if (rotationSpeed == 0) rotationSpeed = 0.0001;
	if (360.0f / 30.0f > rotationSpeed) rotationSpeed *= 1.015;
	if (canUpdate) {
		rotationAngle += rotationSpeed / fps;
		if (rotationAngle >= 360.0f) {
			rotationAngle -= 360.0f;
		}
	}

	glutPostRedisplay();

	glutTimerFunc(1000 / fps, update, 0);
}

void startUpdating(int value)
{
	canUpdate = true;
}

void init()
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(skyRed, skyGreen, skyBlue, 1.0f);
	draw_wheel(0.05, 18, 0.1);
	draw_axis(0.1, 18, 0.05);
	draw_motor_wheel(32, PI / 3, 0.065f, 0.060f, 0.01f, 0.01f, -0.5f, -0.644f, -0.005f);
}

void keyboard(unsigned char key, int x, int y)
{
	keys[key] = true;
}

void keyboardUp(unsigned char key, int x, int y)
{
	keys[key] = false;
}

void mouseButton(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		lastX = x;
		lastY = y;
	}
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("FPS Camera");
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutMouseFunc(mouseButton);
	glutTimerFunc(0, update, 0);
	glutTimerFunc(5000, startUpdating, 0);
	glutMainLoop();
}