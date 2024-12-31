#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <codebase.h>
#include <freeimage/FreeImage.h>
#include <vector>
#include <random>

using namespace cb;

constexpr float INITIAL_SPEED = 0.05f;
constexpr float MAX_SPEED = 0.5f;
constexpr float MIN_SPEED = 0.05f;
constexpr float SPEED_INCREMENT = 0.025f;
constexpr float MOUSE_SENSITIVITY = 0.05f;
constexpr int GRID_SIZE = 10;
constexpr float CAMERA_Z = 3.0f;

GLuint grid;
float speed = INITIAL_SPEED;
float yaw = 0.0f;
float pitch = 0.0f;
float posX = 0.0f, posY = 0.0f, posZ = CAMERA_Z;
int lastMouseX = 0, lastMouseY = 0;

bool cockpit = true;
bool light_on = true;

std::vector<std::vector<float>> asteroid_positions;

float lightDistance = 10.0f;

float fps = 165;

static GLuint tex[4]; // Ids de texturas

void saveScreenshot(char* nombre, int ancho, int alto)
// Utiliza FreeImage para grabar un png
// nombre: Nombre del fichero con extensión p.e. salida.png
// ancho: Ancho del viewport en pixeles
// alto: Alto del viewport en pixeles
{
    int pix = ancho * alto;
    BYTE* pixels = new BYTE[3 * pix];
    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, ancho, alto, GL_BGR, GL_UNSIGNED_BYTE, pixels);
    FIBITMAP* img = FreeImage_ConvertFromRawBits(pixels, ancho, alto, ancho * 3, 24, 0xFF0000, 0x00FF00, 0x0000FF, false);
    FreeImage_Save(FIF_PNG, img, nombre, 0);
}

void loadTexture()
{
    glGenTextures(4, tex);
    //1b. Activar el objeto textura
    glBindTexture(GL_TEXTURE_2D, tex[0]);
    //1c. Cargar la imagen que servira de textura
    char asteroid[] = "assets/asteroid.jpg";
    loadImageFile(asteroid);

    //1b. Activar el objeto textura
    glBindTexture(GL_TEXTURE_2D, tex[1]);
    //1c. Cargar la imagen que servira de textura
    char grass[] = "assets/grass.jpg";
    loadImageFile(grass);

    //1b. Activar el objeto textura
    glBindTexture(GL_TEXTURE_2D, tex[2]);
    //1c. Cargar la imagen que servira de textura
    char background[] = "assets/stars.jpg";
    loadImageFile(background);

    //1b. Activar el objeto textura
    glBindTexture(GL_TEXTURE_2D, tex[3]);
    //1c. Cargar la imagen que servira de textura
    char cockpit[] = "assets/cockpit.png";
    loadImageFile(cockpit);
}

void load_background()
// Funcion de carga de la textura actual como fondo de la ventana
{
	glPushMatrix();
    glPushAttrib(GL_CURRENT_BIT); // Save current color state

    glDisable(GL_LIGHTING);
    glColor4f(0.5f, 0.5f, 0.5f, 1.0f);

	glEnable(GL_TEXTURE_2D); // Enable texturing
	glBindTexture(GL_TEXTURE_2D, tex[2]); // Bind the texture (tex[2])

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Define the way the texture is applied
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Set up the sphere
	GLUquadric* quadric = gluNewQuadric();  // Create a new quadric object for rendering the sphere

	// You can adjust the sphere radius and slices/loops
	gluQuadricTexture(quadric, GL_TRUE);  // Enable texture mapping for the sphere
	gluSphere(quadric, 7000.0f, 50, 50); // Draw the sphere with radius 2500 (for a 5000x5000 background)

	gluDeleteQuadric(quadric); // Clean up the quadric object

    glEnable(GL_LIGHTING);

	glPopMatrix();
    glPopAttrib(); // Restore previous color state


}

void generate_grid()
{
    grid = glGenLists(1);
    glNewList(grid, GL_COMPILE);

    float stepSize = 10.0f;  // Size of each square in the grid
    int subdivisions = 4;   // Number of subdivisions per square (adjust as needed)
    float subStepSize = stepSize / subdivisions; // Step size for subdivisions

    for (int i = -GRID_SIZE; i < GRID_SIZE; ++i)
    {
        for (int j = -GRID_SIZE; j < GRID_SIZE; ++j)
        {
            // Subdivide the square into smaller quads
            for (int x = 0; x < subdivisions; ++x)
            {
                for (int y = 0; y < subdivisions; ++y)
                {
                    float xStart = i * stepSize + x * subStepSize;
                    float yStart = j * stepSize + y * subStepSize;

                    GLfloat v1[] = { xStart, yStart, 0.0f };
                    GLfloat v2[] = { xStart + subStepSize, yStart, 0.0f };
                    GLfloat v3[] = { xStart + subStepSize, yStart + subStepSize, 0.0f };
                    GLfloat v4[] = { xStart, yStart + subStepSize, 0.0f };

                    // Use the helper function to draw the quad
                    quad(v1, v2, v3, v4);
                }
            }
        }
    }

    glEndList();
}


void draw_asteroid(float x, float y, float z, float rotationX, float rotationY, float rotationZ, int scale, float rotatesX, float rotatesY, float rotatesZ, float rotationW)
    {
        glPushMatrix();

        glEnable(GL_TEXTURE_GEN_S); // Enable texture coordinate generation for S
        glEnable(GL_TEXTURE_GEN_T); // Enable texture coordinate generation for T

        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR); // Use object-space linear mapping
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR); // Use object-space linear mapping

        GLfloat plane_s[] = { 1.0f, 0.0f, 1.0f, 0.0f };
        GLfloat plane_t[] = { 0.0f, 1.0f, 0.0f, 1.0f };

        glTexGenfv(GL_S, GL_OBJECT_PLANE, plane_s); // Map S coordinates in object space
        glTexGenfv(GL_T, GL_OBJECT_PLANE, plane_t); // Map T coordinates in object space

        glPushAttrib(GL_CURRENT_BIT);
        glPushMatrix();
        glColor4f(0.5f, 0.35f, 0.25f, 1.0f);

        glTranslatef(x, y, z); // Move to the asteroid's position

        // Apply rotation around the asteroid's local center
        if (rotatesX == 0 && rotatesY == 0 && rotatesZ == 0) glRotatef(rotationW, 0, 1, 0);
        else  glRotatef(rotationW, rotatesX, rotatesY, rotatesZ);
        
        glScalef(1.35, 1.35, 1.35);

        // Draw the first shape
        glPushMatrix();
        glRotatef(rotationX, 1, 0, 1);
        glRotatef(rotationY, 0, 1, 0);
        glRotatef(rotationZ, 1, 0, 1);
        glScalef(1.5f * 4 * scale, 1.0f * 4 * scale, 0.7f * 4 * scale);
        glEnable(GL_TEXTURE_2D); // Enable texturing
        glBindTexture(GL_TEXTURE_2D, tex[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);
        glutSolidDodecahedron();
        glDisable(GL_TEXTURE_2D); // Disable texturing
        glPopMatrix();

        // Draw the second shape (additional components of the asteroid)
        glPushMatrix();
        glTranslatef(5, 15, 0); // Offset from the local center
        glRotatef(rotationX, 1, 0, 1);
        glRotatef(rotationY, 0, 1, 0);
        glRotatef(rotationZ, 1, 1, 1);
        glScalef(4.0f, 4.0f, 4.0f);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);
        glutSolidRhombicDodecahedron();
        glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        // Draw the third shape
        glPushMatrix();
        glTranslatef(5, 5, -5); // Offset from the local center
        glRotatef(rotationX, 0, 0, 1);
        glRotatef(rotationY, 0, 1, 0);
        glRotatef(rotationZ, 0, 1, 1);
        glScalef(3.4f * 4, 3.4f * 2, 3.9f * 5);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);
        glutSolidIcosahedron();
        glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        // Draw the fourth shape
        glPushMatrix();
        glTranslatef(-10, 25, 0); // Offset from the local center
        glRotatef(rotationX, 1, 0, 0);
        glRotatef(rotationY, 0, 1, 0);
        glRotatef(rotationZ, 1, 0, 1);
        glScalef(1.4f * 4 * scale, 1.3f * 4, 1.0f * 4 * scale);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);
        glutSolidIcosahedron();
        glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        glPopMatrix(); // Pop transformations for the asteroid

        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
        glPopMatrix();
        glPopAttrib();
}

void generate_distribution_asteroids()
{
    float range_min = -1500;
    float range_max = 1500;

    std::uniform_int_distribution<> dis(0, 1);
    std::random_device rd;
    std::mt19937 gen(rd());

    for (int i = 0; i < 1500; ++i) {
        float x = range_min + static_cast<float>(rand()) / RAND_MAX * (range_max - range_min);
        float y = range_min + static_cast<float>(rand()) / RAND_MAX * (range_max - range_min);
        float z = range_min + static_cast<float>(rand()) / RAND_MAX * (range_max - range_min);
        float destination_x = range_min - 25 + static_cast<float>(rand()) / RAND_MAX * (50);
        float destination_y = range_min - 25 + static_cast<float>(rand()) / RAND_MAX * (50);
        float destination_z = range_min - 25 + static_cast<float>(rand()) / RAND_MAX * (50);
        float rot_x = static_cast<int>(range_min + static_cast<float>(rand()) / RAND_MAX * (range_max - range_min)) % 360;
        float rot_y = static_cast<int>(range_min + static_cast<float>(rand()) / RAND_MAX * (range_max - range_min)) % 360;
        float rot_z = static_cast<int>(range_min + static_cast<float>(rand()) / RAND_MAX * (range_max - range_min)) % 360;
        float rot_w = static_cast<int>(range_min + static_cast<float>(rand()) / RAND_MAX * (range_max - range_min)) % 360;
        float scale = static_cast<int>(range_min + static_cast<float>(rand()) / RAND_MAX * (range_max - range_min)) % 8 + 1;
        int rotatesX = dis(gen);
        int rotatesY = dis(gen);
        int rotatesZ = dis(gen);
        float rotateSpeed = ((float)rand() / RAND_MAX) * 0.3 + 0.01;

        std::vector<float> positions;
        positions.push_back(x);
        positions.push_back(y);
        positions.push_back(z);
        positions.push_back(rot_x);
        positions.push_back(rot_y);
        positions.push_back(rot_z);
        positions.push_back(scale);
        positions.push_back(rotatesX);
        positions.push_back(rotatesY);
        positions.push_back(rotatesZ);
        positions.push_back(rot_w);
        positions.push_back(rotateSpeed);

        asteroid_positions.push_back(positions);
    }
}

void asteroid_field() 
{
    for (int i = 0; i < asteroid_positions.size(); i++) {
        asteroid_positions.at(i)[10] += asteroid_positions.at(i)[11];
        draw_asteroid(asteroid_positions.at(i)[0], asteroid_positions.at(i)[1], asteroid_positions.at(i)[2], asteroid_positions.at(i)[3], asteroid_positions.at(i)[4], asteroid_positions.at(i)[5],
            asteroid_positions.at(i)[6], asteroid_positions.at(i)[7], asteroid_positions.at(i)[8], asteroid_positions.at(i)[9], asteroid_positions.at(i)[10]);
    }
}

void draw_field() 
{
    glPushMatrix();
    glPushAttrib(GL_CURRENT_BIT); // Save current color state

    glEnable(GL_TEXTURE_2D); // Enable texturing
    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, tex[1]);
    GLfloat v0[] = { -100.0f, -100.0f, 0.0f }; // Bottom-left
    GLfloat v1[] = { 100.0f, -100.0f, 0.0f };  // Bottom-right
    GLfloat v2[] = { 100.0f, 100.0f, 0.0f };   // Top-right
    GLfloat v3[] = { -100.0f, 100.0f, 0.0f };  // Top-left
    glColor4f(0.5f, 0.5f, 0.5f, 1.0f);

    quadtex(v0, v1, v2, v3, 0.0f, 1.0f, 0.0f, 1.0f, 100, 100);
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Define the way the texture is applied
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // Draw the grid using the display list
    glDisable(GL_TEXTURE_2D); // Disable texturing
    glPopMatrix();
    glPopAttrib(); // Restore previous color state

}

void drawUIImage() {
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable texturing
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex[3]);  // Bind the texture (tex[4])

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Define the way the texture is applied
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // Switch to orthographic projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();  // Save the current projection matrix
    glLoadIdentity();  // Reset the projection matrix
    gluOrtho2D(0.0, 1.0, 0.0, 1.0);  // Set an orthographic projection for the texture

    // Switch back to modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();  // Save the current modelview matrix
    glLoadIdentity();  // Reset the modelview matrix

    // Define the quad (a rectangle) to apply the texture
    float quadWidth = 1.0f;  // You can scale this as needed
    float quadHeight = 1.0f;  // You can scale this as needed

    // Draw the quad (this is where the texture will be applied)
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);           // Bottom-left
    glTexCoord2f(1.0f, 0.0f); glVertex2f(quadWidth, 0.0f);       // Bottom-right
    glTexCoord2f(1.0f, 1.0f); glVertex2f(quadWidth, quadHeight); // Top-right
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, quadHeight);      // Top-left
    glEnd();

    // Restore previous matrix states
    glPopMatrix();  // Restore the modelview matrix
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();  // Restore the projection matrix

    // Disable texturing
    glDisable(GL_TEXTURE_2D);

    // Disable blending
    glDisable(GL_BLEND);
}

void update_camera()
{
    float frontX = cos(yaw * PI / 180.0f);
    float frontZ = tan(pitch * PI / 180.0f);
    float frontY = -sin(yaw * PI / 180.0f);

    gluLookAt(posX, posY, posZ, posX + frontX, posY + frontY, posZ + frontZ, 0.0, 0.0, 1.0);

    float leftOffsetX = -2.0f * sin(yaw * PI / 180.0f);
    float leftOffsetZ = 2.0f * cos(yaw * PI / 180.0f);

    float leftAngle = yaw * PI / 180 + PI / 90.0f;
    float rightAngle = yaw * PI / 180 - PI / 90.0f;

    GLfloat light_position_left[] = { posX + lightDistance * sin(leftAngle), posY + lightDistance * cos(leftAngle) , posZ, 1.0f };
    GLfloat light_direction_left[] = { frontX, frontY, frontZ };

    GLfloat light_position_right[] = { posX - lightDistance * sin(rightAngle), posY - lightDistance * cos(rightAngle) , posZ, 1.0f };
    GLfloat light_direction_right[] = { frontX, frontY, frontZ };
    
	glLightfv(GL_LIGHT1, GL_POSITION, light_position_left);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, light_direction_left);

	glLightfv(GL_LIGHT2, GL_POSITION, light_position_right);
	glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, light_direction_right);

    glutPostRedisplay();
}

void animate(int n)
{
    float frontX = cos(yaw * PI / 180.0f) * cos(pitch * PI / 180.0f);
    float frontZ = sin(pitch * PI / 180.0f);
    float frontY = -sin(yaw * PI / 180.0f) * cos(pitch * PI / 180.0f);

    posX += speed * frontX;
    posY += speed * frontY;
    posZ += speed * frontZ;

    glutTimerFunc(1000 / fps, animate, 0);
}

void reshape(GLint w, GLint h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(80.0, (double)w / (double)h, 0.1, 10000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void lighting()
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Make the lighting warmer by increasing red and green components
    GLfloat light_ambient[] = { 0.2f, 0.45f, 0.5f, 1.0f };  // Warmer ambient light (more red and green)
    GLfloat light_diffuse[] = { 0.6f, 0.4f, 0.7f, 1.0f };  // Warmer diffuse light
    GLfloat light_specular[] = { 1.0f, 0.9f, 0.8f, 1.0f }; // Warmer specular light
    GLfloat light_position[] = { 1.0f, 1.0f, 0.5f, 0.0f }; // Light position

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    // Material properties (make the material slightly warmer as well)
    GLfloat mat_ambient[] = { 0.3f, 0.2f, 0.1f, 1.0f }; // Warmer ambient material
    GLfloat mat_diffuse[] = { 0.8f, 0.4f, 0.2f, 1.0f }; // Warmer diffuse material
    GLfloat mat_specular[] = { 1.0f, 0.8f, 0.6f, 1.0f }; // Warmer specular material
    GLfloat mat_shininess[] = { 10.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    // Enable secondary lights
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);

    GLfloat ambient_spot[] = { 0.3f, 0.2f, 0.1f, 1.0f }; // Warmer spot ambient
    GLfloat diffuse_spot[] = { 0.6f, 0.5f, 0.3f, 1.0f }; // Warmer spot diffuse
    GLfloat specular_spot[] = { 1.0f, 0.9f, 0.8f, 1.0f }; // Warmer spot specular

    // Spotlights setup with a sharper beam
    glLightfv(GL_LIGHT1, GL_AMBIENT, ambient_spot);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse_spot);
    glLightfv(GL_LIGHT1, GL_SPECULAR, specular_spot);
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 8.0f);  // Sharper beam, lower cutoff angle
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 50.0f); // Higher exponent for sharper beam

    glLightfv(GL_LIGHT2, GL_AMBIENT, ambient_spot);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, diffuse_spot);
    glLightfv(GL_LIGHT2, GL_SPECULAR, specular_spot);
    glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 8.0f);  // Sharper beam for second spotlight
    glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 50.0f); // Higher exponent for sharper beam

    // Enable material properties
    glEnable(GL_COLOR_MATERIAL);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH); // Modelo de iluminacion suave
    glLoadIdentity();

    update_camera();

    glPushMatrix();
    load_background();
    draw_field();
    asteroid_field();
    glPopMatrix();

    glDepthMask(GL_FALSE);
    if(cockpit) drawUIImage();
    glDepthMask(GL_TRUE);

    glutSwapBuffers();
}

void passive_motion(int x, int y)
{
    int centerX = glutGet(GLUT_WINDOW_WIDTH) / 2;
    int centerY = glutGet(GLUT_WINDOW_HEIGHT) / 2;

    int deltaX = x - lastMouseX;
    int deltaY = y - lastMouseY;

    if (x < lastMouseX)
        yaw -= MOUSE_SENSITIVITY * abs(deltaX);
    else if (x > lastMouseX)
        yaw += MOUSE_SENSITIVITY * abs(deltaX);

    if (y < lastMouseY)
        pitch += MOUSE_SENSITIVITY * abs(deltaY);
    else if (y > lastMouseY)
        pitch -= MOUSE_SENSITIVITY * abs(deltaY);

    if (pitch > 90.0f) pitch = 90.0f;
    if (pitch < -90.0f) pitch = -90.0f;

    glutWarpPointer(centerX, centerY);

    lastMouseX = centerX;
    lastMouseY = centerY;
}

void init()
{
    generate_grid();
    lighting();
    loadTexture();
    generate_distribution_asteroids();
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'a':
        speed += SPEED_INCREMENT;
        if (speed > MAX_SPEED)
            speed = MAX_SPEED;
        break;
    case 'z':
        speed -= SPEED_INCREMENT;
        if (speed < MIN_SPEED)
            speed = MIN_SPEED;
        break;
    case 'l':
        if (light_on) 
        {
            glDisable(GL_LIGHT1);
            glDisable(GL_LIGHT2);
            light_on = !light_on;
        }
        else
        {
            glEnable(GL_LIGHT1);
            glEnable(GL_LIGHT2);
            light_on = !light_on;
        }
        break;
    case 'c':
        cockpit = !cockpit;
        break;
    }
}

int main(int argc, char** argv)
{
    FreeImage_Initialise();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1920, 1080);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("3D Flight Camera");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutPassiveMotionFunc(passive_motion);
    glutKeyboardFunc(keyboard);

    glutSetCursor(GLUT_CURSOR_NONE);


    lastMouseX = glutGet(GLUT_WINDOW_WIDTH) / 2;
    lastMouseY = glutGet(GLUT_WINDOW_HEIGHT) / 2;

    glutTimerFunc(1000 / fps, animate, 0);
    glutMainLoop();
    FreeImage_DeInitialise();

    return 0;
}
