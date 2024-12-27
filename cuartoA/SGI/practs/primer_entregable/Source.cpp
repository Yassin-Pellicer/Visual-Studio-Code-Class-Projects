constexpr auto PROYECTO = "PROYECTO";

#include <iostream>	
#include <codebase.h>
#include <vector>

using namespace cb;
static GLuint wheel;
static GLuint axis;
static GLuint cabins;
static GLuint transmission_gear;
static GLuint motor_gear;

float lastX = 0, lastY = 0;
bool isDragging = false;

static float rotationAngle = 0.0f;
const float rotationSpeed = 360.0f / 30.0f; int fps = 60;

float cameraPosX = 0.0f, cameraPosY = 0.0f, cameraPosZ = 3.0f;
float cameraYaw = -90.0f;
float cameraPitch = 0.0f;
float cameraSpeed = 0.1f;
float cameraRadius = 3.0;
float mouseSensitivity = 0.2f;

bool canUpdate = false;
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
	glColor3f(0.8f, 0.8f, 0.8f);
	for (int i = 0; i < nPuntos; i++) {
		glBegin(GL_TRIANGLE_STRIP);
		glVertex3fv(pEje[i % nPuntos]);
		glVertex3fv(pValle[i % nPuntos]);
		glVertex3fv(pValle[(i + 1) % nPuntos]);
		glVertex3fv(pPico[i % nPuntos]);
		glEnd();
	}

	for (int i = 0; i < nPuntos; i++) {
		glBegin(GL_TRIANGLE_STRIP);
		glVertex3fv(pEje[i % nPuntos]);
		glVertex3fv(pEje[i + 1 % nPuntos]);
		glVertex3fv(pValle[i + 1 % nPuntos]);
		glEnd();
	}

	for (int i = 0; i < nPuntos; i++) {
		glColor3f(0.1f, 0.1f, 0.1f);
		quad(pValle[i + 1 % nPuntos], pValle2[i + 1 % nPuntos], pPico2[(i) % nPuntos], pPico[(i) % nPuntos]);
		quad(pValle[i + 1 % nPuntos], pValle2[i + 1 % nPuntos], pPico2[(i + 1) % nPuntos], pPico[(i + 1) % nPuntos]);

		quad(pEje[i + 1 % nPuntos], pEje2[i + 1 % nPuntos], pEje2[(i) % nPuntos], pEje[(i) % nPuntos]);
		glEnd();
	}

	for (int i = 0; i < nPuntos; i++) {
		glColor3f(0.8f, 0.8f, 0.8f);
		quad(pValle[i + 1 % nPuntos], pValle2[i + 1 % nPuntos], pPico2[(i) % nPuntos], pPico[(i) % nPuntos]);
		quad(pValle[i + 1 % nPuntos], pValle2[i + 1 % nPuntos], pPico2[(i + 1) % nPuntos], pPico[(i + 1) % nPuntos]);

		quad(pEje[i + 1 % nPuntos], pEje2[i + 1 % nPuntos], pEje2[(i) % nPuntos], pEje[(i) % nPuntos]);
		glEnd();
	}

	glColor3f(0.8f, 0.8f, 0.8f);
	for (int i = 0; i < nPuntos; i++) {
		glBegin(GL_TRIANGLE_STRIP);
		glVertex3fv(pEje2[i % nPuntos]);
		glVertex3fv(pValle2[i % nPuntos]);
		glVertex3fv(pValle2[(i + 1) % nPuntos]);
		glVertex3fv(pPico2[i % nPuntos]);
		glEnd();
	}

	for (int i = 0; i < nPuntos; i++) {
		glBegin(GL_TRIANGLE_STRIP);
		glVertex3fv(pEje2[i % nPuntos]);
		glVertex3fv(pEje2[i + 1 % nPuntos]);
		glVertex3fv(pValle2[i + 1 % nPuntos]);
		glEnd();
	}

}

vector<Vec3> cabin_points = puntosCircunferencia(0.05 * 15, 18, 0, 0.0);

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
	glColor3f(0.0f, 1.0f, 1.0f);

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

	// Draw the bottom cap (filling the bottom face)
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.0f, 0.0f, axis[0].z + desfase * 0.75); // Center of the bottom circle
	for (int i = 0; i <= numPuntos; i++) {
		int idx = i % numPuntos; // Wrap around to close the fan
		glVertex3f(axis[idx].x, axis[idx].y, axis[idx].z + desfase * 0.75);
	}
	glEnd();

	// Draw the top cap (filling the top face)
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.0f, 0.0f, axis[0].z + desfase * 1.25); // Center of the top circle
	for (int i = 0; i <= numPuntos; i++) {
		int idx = i % numPuntos; // Wrap around to close the fan
		glVertex3f(axis[idx].x, axis[idx].y, axis[idx].z + desfase * 1.25);
	}
	glEnd();

	// Draw the bottom cap (filling the bottom face)
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.0f, 0.0f, axis[0].z - desfase * 2); // Center of the bottom circle
	for (int i = 0; i <= numPuntos; i++) {
		int idx = i % numPuntos; // Wrap around to close the fan
		glVertex3f(axis[idx].x, axis[idx].y, axis[idx].z - desfase * 2);
	}
	glEnd();

	// Draw the top cap (filling the top face)
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.0f, 0.0f, axis[0].z - desfase * 2.5); // Center of the top circle
	for (int i = 0; i <= numPuntos; i++) {
		int idx = i % numPuntos; // Wrap around to close the fan
		glVertex3f(axis[idx].x, axis[idx].y, axis[idx].z - desfase * 2.5);
	}
	glEnd();

	// SUPPORT
	glLineWidth(20.0f);
	glBegin(GL_LINES);
	glVertex3f(0.0f, 0.0f, desfase * 1);  // Starting point of the line
	glVertex3f(-starting_radio * 10, -starting_radio * 18, desfase); // Ending point of the line
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(0.0f, 0.0f, desfase * 1);  // Starting point of the line
	glVertex3f(+starting_radio * 10, -starting_radio * 18, desfase); // Ending point of the line
	glEnd();

	glLineWidth(10.0f);
	glBegin(GL_LINES);
	glVertex3f(0.0f, 0.0f, -desfase * 2.2);  // Starting point of the line
	glVertex3f(-starting_radio * 10, -starting_radio * 18, -desfase * 2.2); // Ending point of the line
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(0.0f, 0.0f, -desfase * 2.2);  // Starting point of the line
	glVertex3f(+starting_radio * 10, -starting_radio * 18, -desfase * 2.2); // Ending point of the line
	glEnd();

	glEndList();
}

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
	int numPuntos = 14;
	vector<Vec3> circle;

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

	// ROOF-CONNECTION AND ROOF
	for (int i = 0; i < numPuntos; i++) {
		glBegin(GL_LINES);
		glVertex3f(circle[i].x, circle[i].y, circle[i].z + size / 2); // Back vertex
		glVertex3f(circle[i].x, circle[i].y + 0.05, circle[i].z + size / 2); // Front vertex
		glEnd();

		glBegin(GL_LINES);
		glVertex3f(circle[i].x, circle[i].y, circle[i].z - size / 2); // Back vertex
		glVertex3f(circle[i].x, circle[i].y + 0.05, circle[i].z - size / 2); // Front vertex
		glEnd();

		glBegin(GL_TRIANGLE_STRIP);
		for (int i = 0; i < numPuntos; i++) {
			if (circle[i].y + 0.05 > (circle[0].y + circle[numPuntos / 2].y) * 0.5 + 0.03) {
				glVertex3f(circle[i].x, circle[i].y + 0.05, circle[i].z - size / 2);
				glVertex3f(circle[i].x, circle[i].y + 0.05, circle[i].z + size / 2);
			}
		}
		glEnd();
	}
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

void draw_motor_wheel(int nPuntos, float desfase, float pico, float valle, float eje, float Z, float posX, float posY, float posZ) {
	motor_gear = glGenLists(1);

	// Generate and compile display list
	glNewList(motor_gear, GL_COMPILE);

	// Apply the position transformation
	glPushMatrix();  // Save the current matrix
	glTranslatef(posX, posY, posZ);  // Move the gear to the specified position

	// Call gear3D function to draw the motor gear
	gear3D(nPuntos, desfase, pico, valle, eje, Z, motor_gear);

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

	vector<Vec3> axis = puntosCircunferencia(axis_r, numPuntos, 0, 0);
	vector<Vec3> middle = puntosCircunferencia(axis_m, numPuntos, 0, 0);
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

	glLineWidth(1.0f);

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

	glLineWidth(3.0f);

	// DECORATION
	glBegin(GL_LINES);
	for (int i = 0; i < 26; i++) {
		glVertex3f(decoration_axis[i].x, decoration_axis[i].y, decoration_axis[i].z);
		glVertex3f(decoration_outer[i].x, decoration_outer[i].y, decoration_outer[i].z);
	}
	glEnd();

	glBegin(GL_LINES);
	for (int i = 0; i < numPuntos * 2; i++) {
		glVertex3f(decoration_axis[i].x, decoration_axis[i].y, decoration_axis[i].z - desfase);
		glVertex3f(decoration_outer[i].x, decoration_outer[i].y, decoration_outer[i].z - desfase);
	}
	glEnd();

	gear3D(247, PI / 2, axis_o + 0.02, axis_o, axis_o, 0.005, transmission_gear);

	glEndList();
}

void updateCamera()
{
	// Calculate new camera direction based on yaw and pitch
	float frontX = cos(cameraYaw * PI / 180.0f);
	float frontY = sin(cameraPitch * PI / 180.0f);
	float frontZ = sin(cameraYaw * PI / 180.0f);

	if (keys['d']) {  // Move left
		cameraPosX -= frontZ * cameraSpeed * 0.5;
		cameraPosZ += frontX * cameraSpeed * 0.5;
	}
	if (keys['a']) {  // Move right
		cameraPosX += frontZ * cameraSpeed * 0.5;
		cameraPosZ -= frontX * cameraSpeed * 0.5;
	}

	if (!inputDetected)
	{
		// Update the camera position based on WASD keys
		if (keys['w']) {  // Move forward
			if (cameraRadius > 1.5) cameraRadius -= 0.1;
		}
		if (keys['s']) {  // Move backward
			cameraRadius += 0.1;
		}

		cameraYaw += 0.5f;
		if (cameraYaw > 269.0f) cameraYaw = -90.0f;

		frontX = -cos(cameraYaw * PI / 180.0f);

		cameraPosX = 0 + cos(cameraYaw * PI / 180.0f) * cameraRadius;
		cameraPosZ = 0 - sin(cameraYaw * PI / 180.0f) * cameraRadius;
	}

	// Update the camera position based on WASD keys
	if (keys['w']) {  // Move forward
		cameraPosX += frontX * cameraSpeed * 0.5;
		cameraPosZ += frontZ * cameraSpeed * 0.5;
	}
	if (keys['s']) {  // Move backward
		cameraPosX -= frontX * cameraSpeed * 0.5;
		cameraPosZ -= frontZ * cameraSpeed * 0.5;
	}

	if (keys[32]) {
		cameraPosY += cameraSpeed * 0.3;
	}
	if (keys['c']) {
		cameraPosY -= cameraSpeed * 0.3;
	}

	if (keys['v']) {
		inputDetected = !inputDetected;
		cameraPosX = 0.0f, cameraPosY = 0.0f, cameraPosZ = 3.0f;
		cameraYaw = -90.0f;
		cameraPitch = 0.0f;
		cameraRadius = 3.0f;
		Sleep(150);
	}

	gluLookAt(cameraPosX, cameraPosY, cameraPosZ,
		cameraPosX + frontX, cameraPosY + frontY, cameraPosZ + frontZ,
		0.0f, 1.0f, 0.0f);  // Camera looks in the front direction
}

// Function to capture mouse motion for camera rotation
void mouseMotion(int x, int y)
{
	int deltaX = x - lastX;
	int deltaY = y - lastY;

	cameraYaw += deltaX * mouseSensitivity;
	cameraPitch -= deltaY * mouseSensitivity;

	if (cameraPitch > 89.0f) cameraPitch = 89.0f;
	if (cameraPitch < -89.0f) cameraPitch = -89.0f;

	lastX = x;
	lastY = y;
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

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glShadeModel(GL_SMOOTH); // Modelo de iluminacion suave

	glLoadIdentity();

	glScalef(4.0f, 4.0f, 4.0f);

	updateCamera();

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

	glTranslatef(-0.5f, -0.67f, -0.005f);

	glRotatef(-rotationAngle * 8, 0.0f, 0.0f, 1.0f); 

	glTranslatef(0.5f, 0.67f, 0.005f);

	glCallList(motor_gear);

	glutSwapBuffers();
}

void update(int value)
{
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
	// After 5 seconds, allow updates
	canUpdate = true;
}

void init()
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	draw_wheel(0.05, 18, 0.1);
	draw_axis(0.1, 18, 0.05);
	draw_motor_wheel(23, PI / 3, 0.08f, 0.065f, 0.02f, 0.01f, -0.5f, -0.67f, -0.005f);
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
	glutMotionFunc(mouseMotion);
	glutMouseFunc(mouseButton);
	glutTimerFunc(0, update, 0);
	glutTimerFunc(5000, startUpdating, 0);
	glutMainLoop();
}