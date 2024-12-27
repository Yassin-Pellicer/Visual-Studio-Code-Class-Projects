#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <codebase.h>

using namespace cb;

constexpr float INITIAL_SPEED = 0.05f;
constexpr float MAX_SPEED = 0.5f;
constexpr float MIN_SPEED = 0.05f;
constexpr float SPEED_INCREMENT = 0.025f;
constexpr float MOUSE_SENSITIVITY = 0.05f;
constexpr int GRID_SIZE = 1000;
constexpr float CAMERA_Z = 3.0f;

GLuint grid;
float speed = INITIAL_SPEED;
float yaw = 0.0f;
float pitch = 0.0f;
float posX = 0.0f, posY = 0.0f, posZ = CAMERA_Z;
int lastMouseX = 0, lastMouseY = 0;

float lightDistance = 20.0f;

float fps = 165;

void generate_grid()
{
    grid = glGenLists(1);
    glNewList(grid, GL_COMPILE);

    glColor3f(.2f, .2f, .2f);

    float stepSize = 20.0f;  // Size of each square in the grid (adjust as needed)

    for (int i = -GRID_SIZE; i < GRID_SIZE; ++i)
    {
        for (int j = -GRID_SIZE; j < GRID_SIZE; ++j)
        {
            // Define the four corners of a square
            glBegin(GL_QUADS);
            glVertex3f(i * stepSize, j * stepSize, 0.0f);               // Bottom-left corner
            glVertex3f((i + 1) * stepSize, j * stepSize, 0.0f);         // Bottom-right corner
            glVertex3f((i + 1) * stepSize, (j + 1) * stepSize, 0.0f);   // Top-right corner
            glVertex3f(i * stepSize, (j + 1) * stepSize, 0.0f);         // Top-left corner
            glEnd();
        }
    }

    glEnd();
    glEndList();
}

void draw_teapot()
{
    glPushMatrix();
    glColor3f(1.0f, 0.75f, 0.4f);
    glTranslatef(100, 0, 50);
    glRotatef(20, 1, 0, 1);
    glutSolidTeapot(20); // Draw a solid teapot
    glPopMatrix();

    glPushMatrix();
    glColor3f(1.0f, 0.75f, 0.4f);
    glTranslatef(400, -75, 50);
    glRotatef(20, 1, 0, 1);
    glutSolidTeapot(200); // Draw a solid teapot
    glPopMatrix();

}

void update_camera()
{
    float frontX = cos(yaw * PI / 180.0f);
    float frontZ = sin(pitch * PI / 180.0f);
    float frontY = -sin(yaw * PI / 180.0f);

    gluLookAt(posX, posY, posZ, posX + frontX, posY + frontY, posZ + frontZ, 0.0, 0.0, 1.0);

    float leftOffsetX = -2.0f * sin(yaw * PI / 180.0f);
    float leftOffsetZ = 2.0f * cos(yaw * PI / 180.0f);

    float leftAngle = yaw * PI / 180 + PI / 90.0f;
    float rightAngle = yaw * PI / 180 - PI / 90.0f;

    GLfloat light_position_left[] = { posX + lightDistance * sin(leftAngle), posY + lightDistance * cos(leftAngle) , posZ, 1.0f};
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
    float frontX = cos(yaw * PI / 180.0f);
    float frontZ = sin(pitch * PI / 180.0f);
    float frontY = -sin(yaw * PI / 180.0f);

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
    gluPerspective(60.0, (double)w / (double)h, 0.1, 10000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void lighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat light_ambient[] = { 0.2, 0.2, 0.2, 0.2 };
    GLfloat light_diffuse[] = { 0.2, 0.2, 0.2, .2 };
    GLfloat light_specular[] = { .5, .5, .5, .5 };
    GLfloat light_position[] = { 1.0, 1.0, .5, 0.0 };

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    GLfloat mat_ambient[] = { 0.2, 0.2, 0.2, 2.0 };
    GLfloat mat_diffuse[] = { 0.8, 0.2, 0.2, 2.0 };
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 50.0 };

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);

    GLfloat ambient_spot[] = { 0.2f, 0.2f, 0.7f, 1.0f };
    GLfloat diffuse_spot[] = { 0.1f, 0.8f, 1.0f, 0.6f };
    GLfloat specular_spot[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT1, GL_AMBIENT, ambient_spot);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse_spot);
    glLightfv(GL_LIGHT1, GL_SPECULAR, specular_spot);
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 20.0f);
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 100.0f);

    glLightfv(GL_LIGHT2, GL_AMBIENT, ambient_spot);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, diffuse_spot);
    glLightfv(GL_LIGHT2, GL_SPECULAR, specular_spot);
    glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 20.0f);
    glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 100.0f);
    glEnable(GL_COLOR_MATERIAL);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH); // Modelo de iluminacion suave

    glLoadIdentity();
    update_camera();
    glCallList(grid);
    draw_teapot();
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

    if (pitch > 100.0f) pitch = 100.0f;
    if (pitch < -100.0f) pitch = -100.0f;

    glutWarpPointer(centerX, centerY);

    lastMouseX = centerX;
    lastMouseY = centerY;
}

void init() 
{
    generate_grid();
    lighting();
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
    }
}

int main(int argc, char** argv)
{
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

    return 0;
}

