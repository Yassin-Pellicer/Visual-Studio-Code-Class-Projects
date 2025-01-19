#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <codebase.h>
#include <freeimage/FreeImage.h>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <mmsystem.h>
#include <windows.h>
#pragma comment(lib, "winmm.lib")
#include <random>

using namespace cb;

constexpr float INITIAL_SPEED = 0.0f;
constexpr float MAX_SPEED = 2.5f;
constexpr float MIN_SPEED = 0.00f;
constexpr float SPEED_INCREMENT = 0.025f;
constexpr float MOUSE_SENSITIVITY = 0.05f;
constexpr int GRID_SIZE = 10;
constexpr float CAMERA_Z = 3.0f;

std::chrono::steady_clock::time_point lastPressedTime = std::chrono::steady_clock::now();
const std::chrono::milliseconds triggerInterval(100);

GLuint grid;
float speed = INITIAL_SPEED;
float yaw = 0.0f;
float pitch = 0.0f;
float posX = 0.0f, posY = 0.0f, posZ = CAMERA_Z;
int lastMouseX = 0, lastMouseY = 0;
float laserSpeed = 10;

bool cockpit = true;
bool light_on = true;

std::vector<std::vector<float>> asteroid_positions;
std::vector<std::vector<float>> asteroid_hangar_positions;
std::vector<std::vector<float>*> lasers;

float lightDistance = 10.0f;

float fps = 165;

static GLuint tex[8];

float frontX = cos(yaw * PI / 180.0f);
float frontZ = tan(pitch * PI / 180.0f);
float frontY = -sin(yaw * PI / 180.0f);

float copyX = frontX;
float copyY = frontY;
float copyZ = frontZ;

float yawCopy = yaw;
float pitchCopy = pitch;

bool thirdPerson = false;

void saveScreenshot(char* nombre, int ancho, int alto)
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
    glGenTextures(8, tex);
    glBindTexture(GL_TEXTURE_2D, tex[0]);
    char asteroid[] = "assets/asteroid.jpg";
    loadImageFile(asteroid);

    glBindTexture(GL_TEXTURE_2D, tex[1]);
    char grass[] = "assets/grass.jpg";
    loadImageFile(grass);

    glBindTexture(GL_TEXTURE_2D, tex[2]);
    char background[] = "assets/stars.jpg";
    loadImageFile(background);

    glBindTexture(GL_TEXTURE_2D, tex[3]);
    char cockpit[] = "assets/cockpit.png";
    loadImageFile(cockpit);

    glBindTexture(GL_TEXTURE_2D, tex[4]);
    char metal[] = "assets/metal.jpg";
    loadImageFile(metal);

    glBindTexture(GL_TEXTURE_2D, tex[5]);
    char road[] = "assets/road.png";
    loadImageFile(road);

    glBindTexture(GL_TEXTURE_2D, tex[6]);
    char exp[] = "assets/explosion.png";
    loadImageFile(exp);

    glBindTexture(GL_TEXTURE_2D, tex[7]);
    char plasma[] = "assets/plasma.jpg";
    loadImageFile(plasma);
}

void load_background()
{
	glPushMatrix();
    glPushAttrib(GL_CURRENT_BIT);

    glDisable(GL_LIGHTING);

	glEnable(GL_TEXTURE_2D); 
	glBindTexture(GL_TEXTURE_2D, tex[2]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	GLUquadric* quadric = gluNewQuadric();

	gluQuadricTexture(quadric, GL_TRUE); 
    gluSphere(quadric, 7000.0f, 50, 50);

	gluDeleteQuadric(quadric);

    glEnable(GL_LIGHTING);

	glPopMatrix();
    glPopAttrib();
}

void generate_grid()
{
    grid = glGenLists(1);
    glNewList(grid, GL_COMPILE);

    float stepSize = 10.0f;  
      int subdivisions = 4;   
      float subStepSize = stepSize / subdivisions;  

    for (int i = -GRID_SIZE; i < GRID_SIZE; ++i)
    {
        for (int j = -GRID_SIZE; j < GRID_SIZE; ++j)
        {
             
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

	glEnable(GL_TEXTURE_GEN_S); 
  	glEnable(GL_TEXTURE_GEN_T); 
  
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);   
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);   

	GLfloat plane_s[] = { 1.0f, 0.0f, 1.0f, 0.0f };
	GLfloat plane_t[] = { 0.0f, 1.0f, 0.0f, 1.0f };

	glTexGenfv(GL_S, GL_OBJECT_PLANE, plane_s);   
	glTexGenfv(GL_T, GL_OBJECT_PLANE, plane_t);   

	glTranslatef(x, y, z);   

	  
	if (rotatesX == 0 && rotatesY == 0 && rotatesZ == 0) glRotatef(rotationW, 0, 1, 0);
	else  glRotatef(rotationW, rotatesX, rotatesY, rotatesZ);
	
	glScalef(1.35, 1.35, 1.35);
	  
	glPushMatrix();
	glRotatef(rotationX, 1, 0, 1);
	glRotatef(rotationY, 0, 1, 0);
	glRotatef(rotationZ, 1, 0, 1);
	glScalef(1.5f * 4 * scale, 1.0f * 4 * scale, 0.7f * 4 * scale);
	glEnable(GL_TEXTURE_2D);   
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);
	glutSolidDodecahedron();
	glDisable(GL_TEXTURE_2D);   
	glPopMatrix();

	glPushMatrix();
	glTranslatef(5, 15, 0);   
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

	glPushMatrix();
	glTranslatef(5, 5, -5);   
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
	  
	glPushMatrix();
	glTranslatef(-10, 25, 0);   
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

	glPopMatrix();   

	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glPopMatrix();
	glPopAttrib();
}

void generate_distribution_asteroids()
{
    float range_min = -3000;
    float range_max = 3000;

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

struct Billboard {
    float position[3];   
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
};

std::vector<Billboard> activeBillboards;

void draw_explosion(const Billboard& billboard) {
      
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_LIGHTING);
      
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex[6]);
      
    float modelview[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);

    float right[3] = { modelview[0], modelview[4], modelview[8] };
    float up[3] = { modelview[1], modelview[5], modelview[9] };

    float size = 200.0f;   

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(billboard.position[0] - size * right[0] - size * up[0],
        billboard.position[1] - size * right[1] - size * up[1],
        billboard.position[2] - size * right[2] - size * up[2]);

    glTexCoord2f(1, 0); glVertex3f(billboard.position[0] + size * right[0] - size * up[0],
        billboard.position[1] + size * right[1] - size * up[1],
        billboard.position[2] + size * right[2] - size * up[2]);

    glTexCoord2f(1, 1); glVertex3f(billboard.position[0] + size * right[0] + size * up[0],
        billboard.position[1] + size * right[1] + size * up[1],
        billboard.position[2] + size * right[2] + size * up[2]);

    glTexCoord2f(0, 1); glVertex3f(billboard.position[0] - size * right[0] + size * up[0],
        billboard.position[1] - size * right[1] + size * up[1],
        billboard.position[2] - size * right[2] + size * up[2]);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
      
    glEnable(GL_LIGHTING);
}


void spawn_explosion(float x, float y, float z) {
    Billboard newBillboard = { {x, y, z}, std::chrono::high_resolution_clock::now() };
    activeBillboards.push_back(newBillboard);
}

void update_explosions() {
    auto now = std::chrono::high_resolution_clock::now();

    activeBillboards.erase(std::remove_if(activeBillboards.begin(), activeBillboards.end(),
        [now](const Billboard& b) {
            return std::chrono::duration<float>(now - b.startTime).count() > 0.1f;
        }),
        activeBillboards.end());
      
    for (const auto& billboard : activeBillboards) {
        draw_explosion(billboard);
    }
}

void check_collision() {
    const float COLLISION_THRESHOLD = 100.0f;
      
    std::vector<size_t> asteroids_to_remove;
    std::vector<size_t> lasers_to_remove;
      
    for (size_t l = 0; l < lasers.size(); l++) {
        auto& laser = *lasers[l];   
        float laser_x = laser[0];
        float laser_y = laser[1];
        float laser_z = laser[2];

        float distance_from_center = sqrt(laser_x * laser_x + laser_y * laser_y + laser_z * laser_z);
        if (distance_from_center > 7000) {
            lasers_to_remove.push_back(l);
            continue;   
        }
          
        for (size_t a = 0; a < asteroid_positions.size(); a++) {

            auto& asteroid = asteroid_positions[a];
            float asteroid_x = asteroid[0];
            float asteroid_y = asteroid[1];
            float asteroid_z = asteroid[2];
            float asteroid_scale = asteroid[6];
              
            float dx = laser_x - asteroid_x;
            float dy = laser_y - asteroid_y;
            float dz = laser_z - asteroid_z;
            float distance = sqrt(dx * dx + dy * dy + dz * dz);

            float adjusted_threshold = COLLISION_THRESHOLD ;

            if (distance < adjusted_threshold) {
                spawn_explosion(
                    laser_x - (dx * 0.5f),    
                    laser_y - (dy * 0.5f),
                    laser_z - (dz * 0.5f)
                );

                PlaySound(TEXT("assets/explosion.wav"), NULL, SND_FILENAME | SND_ASYNC );

                glutPostRedisplay();
                  
                asteroids_to_remove.push_back(a);
                lasers_to_remove.push_back(l);

                break;   
            }
        }
    }

    std::sort(asteroids_to_remove.begin(), asteroids_to_remove.end(), std::greater<size_t>());
    std::sort(lasers_to_remove.begin(), lasers_to_remove.end(), std::greater<size_t>());

    // Remove asteroids
    for (size_t index : asteroids_to_remove) {
        asteroid_positions.erase(asteroid_positions.begin() + index);
    }

    // Remove lasers (and free memory)
    for (size_t index : lasers_to_remove) {
        delete lasers[index];
        lasers.erase(lasers.begin() + index);
    }
}

void laser(float x, float y, float z, float yaw, float pitch)
{
    static bool seeded = false;
    if (!seeded) {
        std::srand(static_cast<unsigned>(std::time(0)));
        seeded = true;
    }

    float leftAngle = yaw * PI / 180 + PI / 90.0f;
    float rightAngle = yaw * PI / 180 - PI / 90.0f;

    GLUquadric* quadric = gluNewQuadric();

    glPushAttrib(GL_CURRENT_BIT);
    glPushMatrix();
    glDisable(GL_LIGHTING);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex[7]);
    gluQuadricTexture(quadric, GL_TRUE);

    float randomAngle1 = std::rand() % 360;
    glTranslatef(x + 10 * sin(leftAngle), y + 10 * cos(leftAngle), z);
    glRotatef(randomAngle1, 1.0f, 0.0f, 0.0f);

    gluSphere(quadric, 2, 5, 5);

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glPopMatrix();
    glPopAttrib();

    glPushAttrib(GL_CURRENT_BIT);
    glPushMatrix();
    glDisable(GL_LIGHTING);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex[7]);
    gluQuadricTexture(quadric, GL_TRUE);

    float randomAngle2 = std::rand() % 360;
    glTranslatef(x - 10 * sin(rightAngle), y - 10 * cos(rightAngle), z);
    glRotatef(randomAngle2, 0.0f, 1.0f, 0.0f);

    gluSphere(quadric, 2, 5, 5);

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glPopMatrix();
    glPopAttrib();

    gluDeleteQuadric(quadric);
}

void draw_lasers()
{
    if (!lasers.empty()) {
        for (size_t i = 0; i < lasers.size(); i++) { // Use < instead of <=
            std::vector<float>* l = lasers.at(i);     
			laser((*l)[0], (*l)[1], (*l)[2], (*l)[3], (*l)[4]);   
        }
    }
}

void generate_distribution_asteroids_hangar()
{
    float range_min = -250;
    float range_max = 250;

    std::uniform_int_distribution<> dis(0, 1);
    std::random_device rd;
    std::mt19937 gen(rd());

    for (int i = 0; i < 250; ++i) {
        float x = range_min + static_cast<float>(rand()) / RAND_MAX * (range_max - range_min);
        float y = range_min + static_cast<float>(rand()) / RAND_MAX * (range_max - range_min);
        float z = range_min + static_cast<float>(rand()) / RAND_MAX * (35 - range_min);
        float destination_x = range_min - 25 + static_cast<float>(rand()) / RAND_MAX * (50);
        float destination_y = range_min - 25 + static_cast<float>(rand()) / RAND_MAX * (50);
        float destination_z = range_min - 25 + static_cast<float>(rand()) / RAND_MAX * (50);
        float rot_x = static_cast<int>(range_min + static_cast<float>(rand()) / RAND_MAX * (range_max - range_min)) % 360;
        float rot_y = static_cast<int>(range_min + static_cast<float>(rand()) / RAND_MAX * (range_max - range_min)) % 360;
        float rot_z = static_cast<int>(range_min + static_cast<float>(rand()) / RAND_MAX * (range_max - range_min)) % 360;
        float rot_w = static_cast<int>(range_min + static_cast<float>(rand()) / RAND_MAX * (range_max - range_min)) % 360;
        float scale = static_cast<int>(range_min + static_cast<float>(rand()) / RAND_MAX * (range_max - range_min)) % 7 + 1;
        int rotatesX = dis(gen);
        int rotatesY = dis(gen);
        int rotatesZ = dis(gen);
        float rotateSpeed = 0;


        float distance = sqrt(x * x + y * y + z * z);
        if (distance < 200.0f) {
            float scale_factor = (distance + 50.0f) / distance;
            x *= scale_factor;
            y *= scale_factor;
            z *= scale_factor;
        }

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

        asteroid_hangar_positions.push_back(positions);
    }
}

void asteroids_hangar()
{
    for (int i = 0; i < asteroid_hangar_positions.size(); i++) {
        asteroid_hangar_positions.at(i)[10] += asteroid_hangar_positions.at(i)[11];
        draw_asteroid(
            asteroid_hangar_positions.at(i)[0], asteroid_hangar_positions.at(i)[1], asteroid_hangar_positions.at(i)[2],
            asteroid_hangar_positions.at(i)[3], asteroid_hangar_positions.at(i)[4], asteroid_hangar_positions.at(i)[5],
            asteroid_hangar_positions.at(i)[6], asteroid_hangar_positions.at(i)[7], asteroid_hangar_positions.at(i)[8],
            asteroid_hangar_positions.at(i)[9], asteroid_hangar_positions.at(i)[10]
        );
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



void drawBox(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
      
    glScalef(1.2, 1.2, 1.2);

    glBegin(GL_QUADS);

    // Front face
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, 0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, 0.5f);

    // Back face
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f);

    // Top face
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, 0.5f, 0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, 0.5f, 0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f);

    // Bottom face
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, -0.5f, -0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);

    // Right face
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, 0.5f, 0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);

    // Left face
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, 0.5f, 0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f);

    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void drawShip(float x, float y, float z, float yaw, float pitch) {
    glPushMatrix();

    // Translate to the ship's position
    glTranslatef(x, y, z);
      
    glRotatef(-yawCopy, 0.0f, 0.0f, 1.0f);    
    glRotatef(-pitchCopy, 0.0f, 1.0f, 0.0f);    

    glutSolidSphere(1, 10, 10);
    glPushMatrix();
    glScalef(0.75, 2.5, 0.25);
    drawBox(0.0f, -0.5f, 0.0f);   
    drawBox(0.0f, 0.5f, 0.0f);   
    glPopMatrix();

    glPushMatrix();
    glScalef(0.75, 2, 0.25);
    glTranslatef(0.0f, -2.0f, 0.0f);   
    glutSolidRhombicDodecahedron();
    glPopMatrix();
    glPushMatrix();
    glScalef(0.75, 2, 0.25);
    glTranslatef(0.0f, 2.0f, 0.0f);   
    glutSolidRhombicDodecahedron();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, -3.0f, 0.0f);   
    glScalef(0.25, 0.25, 3);
    glutSolidRhombicDodecahedron();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.0f, 3.0f, 0.0f);   
    glScalef(0.25, 0.25, 3);
    glutSolidRhombicDodecahedron();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, -3.5f, 0.0f);   
    glScalef(0.75, 0.25, 2);
    glutSolidRhombicDodecahedron();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.0f, 3.5f, 0.0f);   
    glScalef(0.75, 0.25, 2);
    glutSolidRhombicDodecahedron();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-1, 0.0f, 0.0f);   
    glScalef(0.75, 0.25, 2);
    glRotatef(90, 1, 0, 0);
    glutSolidRhombicDodecahedron();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.0f, 3.5f, 0.0f);   
    glScalef(0.75, 0.25, 2);
    glutSolidRhombicDodecahedron();
    glPopMatrix();

    glPopMatrix();
}

void draw_hangar() 
{
    glPushMatrix();
      
    glRotatef(90, 1, 0, 0);
    glTranslatef(0, 10.005, 0);
    glScalef(40, 20, 20);

      
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex[4]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // Front face
    GLfloat front_v0[] = { -1.0f, -0.5f,  1.0f };
    GLfloat front_v1[] = { 1.0f, -0.5f,  1.0f };
    GLfloat front_v2[] = { 1.0f,  0.5f,  1.0f };
    GLfloat front_v3[] = { -1.0f,  0.5f,  1.0f };
    quadtex(front_v0, front_v1, front_v2, front_v3, 0.0f, 1.0f, 0.0f, 1.0f, 50, 50);

    // Back face
    GLfloat back_v0[] = { -1.0f, -0.5f, -1.0f };
    GLfloat back_v1[] = { 1.0f, -0.5f, -1.0f };
    GLfloat back_v2[] = { 1.0f,  0.5f, -1.0f };
    GLfloat back_v3[] = { -1.0f,  0.5f, -1.0f };
    quadtex(back_v0, back_v1, back_v2, back_v3, 0.0f, 1.0f, 0.0f, 1.0f, 50, 50);

    //DOORS

    //back
    GLfloat left_door_v0[] = { -1.0f, -0.5f,  0.5f }; // Bottom-left
    GLfloat left_door_v1[] = { -1.0f, -0.5f,  1.0f }; // Bottom-right
    GLfloat left_door_v2[] = { -1.0f,  0.5f,  1.0f }; // Top-right
    GLfloat left_door_v3[] = { -1.0f,  0.5f,  0.5f }; // Top-left

    quadtex(left_door_v0, left_door_v1, left_door_v2, left_door_v3, 0.0f, 1.0f, 0.0f, 1.0f, 50, 50);

    GLfloat right_door_v0[] = { -1.0f, -0.5f,  -1.0f }; // Bottom-left
    GLfloat right_door_v1[] = { -1.0f, -0.5f,  -0.5f }; // Bottom-right
    GLfloat right_door_v2[] = { -1.0f,  0.5f,  -0.5f }; // Top-right
    GLfloat right_door_v3[] = { -1.0f,  0.5f,  -1.0f }; // Top-left

    quadtex(right_door_v0, right_door_v1, right_door_v2, right_door_v3, 0.0f, 1.0f, 0.0f, 1.0f, 50, 50);

    //front
    GLfloat _left_door_v0[] = { 1.0f, -0.5f,  0.5f }; // Bottom-left
    GLfloat _left_door_v1[] = { 1.0f, -0.5f,  1.0f }; // Bottom-right
    GLfloat _left_door_v2[] = { 1.0f,  0.5f,  1.0f }; // Top-right
    GLfloat _left_door_v3[] = { 1.0f,  0.5f,  0.5f }; // Top-left

    quadtex(_left_door_v0, _left_door_v1, _left_door_v2, _left_door_v3, 0.0f, 1.0f, 0.0f, 1.0f, 50, 50);

    GLfloat _right_door_v0[] = { 1.0f, -0.5f,  -1.0f }; // Bottom-left
    GLfloat _right_door_v1[] = { 1.0f, -0.5f,  -0.5f }; // Bottom-right
    GLfloat _right_door_v2[] = { 1.0f,  0.5f,  -0.5f }; // Top-right
    GLfloat _right_door_v3[] = { 1.0f,  0.5f,  -1.0f }; // Top-left

    quadtex(_right_door_v0, _right_door_v1, _right_door_v2, _right_door_v3, 0.0f, 1.0f, 0.0f, 1.0f, 50, 50);

    // Top face
    GLfloat top_v0[] = { -1.0f,  0.5f, -1.0f };
    GLfloat top_v1[] = { 1.0f,  0.5f, -1.0f };
    GLfloat top_v2[] = { 1.0f,  0.5f,  1.0f };
    GLfloat top_v3[] = { -1.0f,  0.5f,  1.0f };
    quadtex(top_v0, top_v1, top_v2, top_v3, 0.0f, 1.0f, 0.0f, 1.0f, 50, 50);

    // Bottom face
    GLfloat bottom_v0[] = { -1.0f, -0.5f, -1.0f };
    GLfloat bottom_v1[] = { 1.0f, -0.5f, -1.0f };
    GLfloat bottom_v2[] = { 1.0f, -0.5f,  1.0f };
    GLfloat bottom_v3[] = { -1.0f, -0.5f,  1.0f };
    quadtex(bottom_v0, bottom_v1, bottom_v2, bottom_v3, 0.0f, 1.0f, 0.0f, 1.0f, 50, 50);

    // ROOF
    GLfloat roof_front_v0[] = { -1.0f, 0.5f, 1.0f };    // Bottom left
    GLfloat roof_front_v1[] = { 1.0f, 0.5f, 1.0f };     // Bottom right
    GLfloat roof_front_v2[] = { 0.0f, 1.0f, 1.0f };     // Top center
    quadtex(roof_front_v1, roof_front_v0, roof_front_v2, roof_front_v2, 0.0f, 1.0f, 0.5f, 25, 25);
      
    GLfloat roof_back_v0[] = { -1.0f, 0.5f, -1.0f };    // Bottom left
    GLfloat roof_back_v1[] = { 1.0f, 0.5f, -1.0f };     // Bottom right
    GLfloat roof_back_v2[] = { 0.0f, 1.0f, -1.0f };     // Top center
    quadtex(roof_back_v1, roof_back_v0, roof_back_v2, roof_back_v2, 0.0f, 1.0f, 0.5f, 25, 25);
      
    GLfloat roof_left_v0[] = { -1.0f, 0.5f, -1.0f };    // Bottom back
    GLfloat roof_left_v1[] = { -1.0f, 0.5f, 1.0f };     // Bottom front
    GLfloat roof_left_v2[] = { 0.0f, 1.0f, 1.0f };      // Top front
    GLfloat roof_left_v3[] = { 0.0f, 1.0f, -1.0f };     // Top back
    quadtex(roof_left_v1, roof_left_v0, roof_left_v3, roof_left_v2, 0.0f, 1.0f, 0.0f, 1.0f, 25, 25);
      
    GLfloat roof_right_v0[] = { 1.0f, 0.5f, -1.0f };    // Bottom back
    GLfloat roof_right_v1[] = { 1.0f, 0.5f, 1.0f };     // Bottom front
    GLfloat roof_right_v2[] = { 0.0f, 1.0f, 1.0f };     // Top front
    GLfloat roof_right_v3[] = { 0.0f, 1.0f, -1.0f };    // Top back
    quadtex(roof_right_v1, roof_right_v0, roof_right_v3, roof_right_v2, 0.0f, 1.0f, 0.0f, 1.0f, 25, 25);

    //ROAD
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();

    glPushMatrix();
    glRotatef(90, 1, 0, 0);
    glTranslatef(0, 3.6, 0);
    glScalef(100, 7, 7.5);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex[5]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    GLfloat road_v0[] = { -1.0f, -0.5f, -1.0f };
    GLfloat road_v1[] = { 1.0f, -0.5f, -1.0f };
    GLfloat road_v2[] = { 1.0f, -0.5f,  1.0f };
    GLfloat road_v3[] = { -1.0f, -0.5f,  1.0f };

    quadtex(road_v0, road_v1, road_v2, road_v3, 0.0f, 1.0f, 0.0f, 1.0f, 50, 50);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);

    glPopMatrix();

    glPushMatrix();

    glRotatef(90, 1, 0, 0);
    glTranslatef(0, -5, 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex[6]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    GLfloat test_1[] = { -1.0f, -0.5f, -1.0f };
    GLfloat test_2[] = { 1.0f, -0.5f, -1.0f };
    GLfloat test_3[] = { 1.0f, -0.5f,  1.0f };
    GLfloat test_4[] = { -1.0f, -0.5f,  1.0f };

    quadtex(test_1, test_2, test_3, test_4, 0.0f, 1.0f, 0.0f, 1.0f, 50, 50);
    glPopMatrix();

    glPushMatrix();

    glRotatef(90, 1, 0, 0);
    glTranslatef(0, -5, 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex[7]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    GLfloat _test_1[] = { -1.0f, -0.5f, -1.0f };
    GLfloat _test_2[] = { 1.0f, -0.5f, -1.0f };
    GLfloat _test_3[] = { 1.0f, -0.5f,  1.0f };
    GLfloat _test_4[] = { -1.0f, -0.5f,  1.0f };

    quadtex(_test_1, _test_2, _test_3, _test_4, 0.0f, 1.0f, 0.0f, 1.0f, 50, 50);
    glPopMatrix();

    asteroids_hangar();
}

void draw_field() 
{
    glPushMatrix();
    glPushAttrib(GL_CURRENT_BIT);   

    glEnable(GL_TEXTURE_2D);   
      
    glBindTexture(GL_TEXTURE_2D, tex[1]);
    GLfloat v0[] = { -100.0f, -100.0f, 0.0f };   
    GLfloat v1[] = { 100.0f, -100.0f, 0.0f };    
    GLfloat v2[] = { 100.0f, 100.0f, 0.0f };     
    GLfloat v3[] = { -100.0f, 100.0f, 0.0f };    

    quadtex(v0, v1, v2, v3, 0.0f, 1.0f, 0.0f, 1.0f, 100, 100);
      
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

      
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

      
    glDisable(GL_TEXTURE_2D);   
    glPopMatrix();
    glPopAttrib();   

    glPushMatrix();
    glPushAttrib(GL_CURRENT_BIT);
    glTranslatef(0.0f, 0, -100.5f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex[2]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);

    float size = 100.0f;   

      
    GLfloat front_v0[] = { -size, -size,  size };
    GLfloat front_v1[] = { size, -size,  size };
    GLfloat front_v2[] = { size,  size,  size };
    GLfloat front_v3[] = { -size,  size,  size };
    quadtex(front_v0, front_v1, front_v2, front_v3, 0.0f, 1.0f, 0.0f, 1.0f, 20, 20);

      
    GLfloat back_v0[] = { -size, -size, -size };
    GLfloat back_v1[] = { size, -size, -size };
    GLfloat back_v2[] = { size,  size, -size };
    GLfloat back_v3[] = { -size,  size, -size };
    quadtex(back_v0, back_v1, back_v2, back_v3, 0.0f, 1.0f, 0.0f, 1.0f, 20, 20);

      
    GLfloat top_v0[] = { -size,  size, -size };
    GLfloat top_v1[] = { size,  size, -size };
    GLfloat top_v2[] = { size,  size,  size };
    GLfloat top_v3[] = { -size,  size,  size };
    quadtex(top_v0, top_v1, top_v2, top_v3, 0.0f, 1.0f, 0.0f, 1.0f, 20, 20);

      
    GLfloat bottom_v0[] = { -size, -size, -size };
    GLfloat bottom_v1[] = { size, -size, -size };
    GLfloat bottom_v2[] = { size, -size,  size };
    GLfloat bottom_v3[] = { -size, -size,  size };
    quadtex(bottom_v0, bottom_v1, bottom_v2, bottom_v3, 0.0f, 1.0f, 0.0f, 1.0f, 20, 20);

      
    GLfloat right_v0[] = { size, -size, -size };
    GLfloat right_v1[] = { size, -size,  size };
    GLfloat right_v2[] = { size,  size,  size };
    GLfloat right_v3[] = { size,  size, -size };
    quadtex(right_v0, right_v1, right_v2, right_v3, 0.0f, 1.0f, 0.0f, 1.0f, 20, 20);

      
    GLfloat left_v0[] = { -size, -size, -size };
    GLfloat left_v1[] = { -size, -size,  size };
    GLfloat left_v2[] = { -size,  size,  size };
    GLfloat left_v3[] = { -size,  size, -size };
    quadtex(left_v0, left_v1, left_v2, left_v3, 0.0f, 1.0f, 0.0f, 1.0f, 20, 20);

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
    glPopAttrib();
}

void drawUIImage() {
      
    glEnable(GL_BLEND);
    glDisable(GL_LIGHTING);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex[3]);    
      
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();    
    glLoadIdentity();    
    gluOrtho2D(0.0, 1.0, 0.0, 1.0);    

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();    
    glLoadIdentity();    

    float quadWidth = 1.0f;    
    float quadHeight = 1.0f;    

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);             
    glTexCoord2f(1.0f, 0.0f); glVertex2f(quadWidth, 0.0f);         
    glTexCoord2f(1.0f, 1.0f); glVertex2f(quadWidth, quadHeight);   
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, quadHeight);        
    glEnd();

      
    glPopMatrix();    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();    

      
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_LIGHTING);
      
    glDisable(GL_BLEND);
}

void update_camera()
{
    float leftAngle = yawCopy * PI / 180 + PI / 90.0f;
    float rightAngle = yawCopy * PI / 180 - PI / 90.0f;

    frontX = cos(yawCopy * PI / 180.0f);
    frontZ = tan(pitchCopy * PI / 180.0f);
    frontY = -sin(yawCopy * PI / 180.0f);

    if (thirdPerson)
    {
        gluLookAt(posX - 10 * cos(yaw * PI / 180.0f), posY + 10 * sin(yaw * PI / 180.0f), posZ - 10 * sin(pitch * PI / 180.0f), posX, posY, posZ, 0.0, 0.0, 1.0);

        GLfloat light_position_left[] = { posX + lightDistance * sin(leftAngle), posY + lightDistance * cos(leftAngle) , posZ, 1.0f };
        GLfloat light_direction_left[] = { frontX, frontY, frontZ };

        GLfloat light_position_right[] = { posX - lightDistance * sin(rightAngle), posY - lightDistance * cos(rightAngle) , posZ, 1.0f };
        GLfloat light_direction_right[] = { frontX, frontY, frontZ };

        glLightfv(GL_LIGHT1, GL_POSITION, light_position_left);
        glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, light_direction_left);

        glLightfv(GL_LIGHT2, GL_POSITION, light_position_right);
        glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, light_direction_right);
    }
    else {
        gluLookAt(posX, posY, posZ, posX + frontX, posY + frontY, posZ + frontZ, 0.0, 0.0, 1.0);

        GLfloat light_position_left[] = { posX + lightDistance * sin(leftAngle), posY + lightDistance * cos(leftAngle) , posZ, 1.0f };
        GLfloat light_direction_left[] = { frontX, frontY, frontZ };

        GLfloat light_position_right[] = { posX - lightDistance * sin(rightAngle), posY - lightDistance * cos(rightAngle) , posZ, 1.0f };
        GLfloat light_direction_right[] = { frontX, frontY, frontZ };

        glLightfv(GL_LIGHT1, GL_POSITION, light_position_left);
        glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, light_direction_left);

        glLightfv(GL_LIGHT2, GL_POSITION, light_position_right);
        glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, light_direction_right);
    }

    glutPostRedisplay();
}


void animate(int n)
{
    static bool seeded = false;
    if (!seeded) {
        std::srand(static_cast<unsigned>(std::time(0)));
        seeded = true;
    }

    if (!thirdPerson)
    {
        float frontX = cos(yaw * PI / 180.0f) * cos(pitch * PI / 180.0f);
        float frontZ = sin(pitch * PI / 180.0f);
        float frontY = -sin(yaw * PI / 180.0f) * cos(pitch * PI / 180.0f);

        copyX = frontX;
        copyY = frontY;
        copyZ = frontZ;

        yawCopy = yaw;
        pitchCopy = pitch;

        posX += speed * frontX;
        posY += speed * frontY;
        posZ += speed * frontZ;
    }
    else
    {
        posX += speed * copyX;
        posY += speed * copyY;
        posZ += speed * copyZ;
    }

    for (size_t i = 0; i < lasers.size(); i++) {
        std::vector<float>* l = lasers.at(i);

        float dispersionX = (std::rand() % 200 - 100) / 200.0f;   
        float dispersionY = (std::rand() % 200 - 100) / 200.0f;
        float dispersionZ = (std::rand() % 200 - 100) / 200.0f;

        (*l)[0] += speed * (*l)[5] + laserSpeed * (*l)[5] + dispersionX;
        (*l)[1] += speed * (*l)[6] + laserSpeed * (*l)[6] + dispersionY;
        (*l)[2] += speed * (*l)[7] + laserSpeed * (*l)[7] + dispersionZ;
    }

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
      
    GLfloat light_ambient[] = { 0.20f, 0.20f, 0.20f, 1.0f };    
    GLfloat light_diffuse[] = { 0.6f, 0.4f, 0.7f, 1.0f };    
    GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };   
    GLfloat light_position[] = { 1.0f, .5f, 0.5f, 0.0f };   

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    GLfloat mat_ambient[] = { 0.3f, 0.2f, 0.1f, 1.0f }; 
    GLfloat mat_diffuse[] = { 0.8f, 0.4f, 0.2f, 1.0f };   
    GLfloat mat_specular[] = { 1.0f, 0.8f, 0.6f, 1.0f };   
    GLfloat mat_shininess[] = { 10.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);

    GLfloat ambient_spot[] = { 0.3f, 0.2f, 0.1f, 1.0f };   
    GLfloat diffuse_spot[] = { 0.6f, 0.5f, 0.3f, 1.0f };   
    GLfloat specular_spot[] = { 1.0f, 0.9f, 0.8f, 1.0f };   

    glLightfv(GL_LIGHT1, GL_AMBIENT, ambient_spot);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse_spot);
    glLightfv(GL_LIGHT1, GL_SPECULAR, specular_spot);
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 8.0f);    
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 50.0f);   

    glLightfv(GL_LIGHT2, GL_AMBIENT, ambient_spot);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, diffuse_spot);
    glLightfv(GL_LIGHT2, GL_SPECULAR, specular_spot);
    glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 8.0f);    
    glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 50.0f);   

    glEnable(GL_COLOR_MATERIAL);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);   
    glLoadIdentity();
    update_camera();
    draw_lasers();

    glPushMatrix();
    load_background();
    draw_field();
    asteroid_field();
    draw_hangar();
    check_collision();
    update_explosions();
    if(thirdPerson) drawShip(posX, posY, posZ, yaw, pitch);
    glPopMatrix();

    glDepthMask(GL_FALSE);
    if(cockpit && !thirdPerson) drawUIImage();
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
    if (yaw >= 360) yaw = 0;
    if (yaw <= -360) yaw = 0;

    glutWarpPointer(centerX, centerY);

    lastMouseX = centerX;
    lastMouseY = centerY;
}

void init()
{
    generate_grid();
    loadTexture();
    generate_distribution_asteroids();
    generate_distribution_asteroids_hangar();
    lighting();
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 't':
        thirdPerson = !thirdPerson;
        break;
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
    case 'w':
        auto now = std::chrono::steady_clock::now();
        if (now - lastPressedTime >= triggerInterval) {    
              
            PlaySound(TEXT("assets/plasmashot.wav"), NULL, SND_FILENAME | SND_ASYNC );

            std::vector<float>* laser = new std::vector<float>;
            float frontX = cos(yawCopy * 3.14159 / 180.0f) * cos(pitchCopy * 3.14159 / 180.0f);
            float frontZ = sin(pitchCopy * 3.14159 / 180.0f);
            float frontY = -sin(yawCopy * 3.14159 / 180.0f) * cos(pitchCopy * 3.14159 / 180.0f);
            laser->push_back(posX);
            laser->push_back(posY);
            laser->push_back(posZ);
            laser->push_back(yawCopy);
            laser->push_back(pitchCopy);
            laser->push_back(frontX);
            laser->push_back(frontY);
            laser->push_back(frontZ);
            lasers.push_back(laser);

            lastPressedTime = now;
        }
        break;
    }

}

int main(int argc, char** argv)
{
    std::cout << "a y z aceleran y deceleran la nave. l y c para apagar/encender luces y mostrar cabina respectivamente. w para disparar. t para cambiar el modo a 3a persona. Los asteroides que no son parte de la base SE PUEDEN DESTRUIR CON LOS DISPAROS. Se ha añadido sonido" <<  std::endl;
    FreeImage_Initialise();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(-10, -10);
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
