#include "pch.h"
#include <fstream>
#include <freeglut.h>  
#include <math.h>
#include <string>

using namespace std;

struct BitMapFile
{
	int sizeX;
	int sizeY;
	unsigned char *data;
};

BitMapFile *getbmp(string filename)
{
	int offset, headerSize;

	// Initialize bitmap files for RGB (input) and RGBA (output).
	BitMapFile *bmpRGB = new BitMapFile;
	BitMapFile *bmpRGBA = new BitMapFile;

	// Read input bmp file name.
	ifstream infile(filename.c_str(), ios::binary);

	// Get starting point of image data in bmp file.
	infile.seekg(10);
	infile.read((char *)&offset, 4);

	// Get header size of bmp file.
	infile.read((char *)&headerSize, 4);

	// Get image width and height values from bmp file header.
	infile.seekg(18);
	infile.read((char *)&bmpRGB->sizeX, 4);
	infile.read((char *)&bmpRGB->sizeY, 4);

	// Determine the length of zero-byte padding of the scanlines 
	// (each scanline of a bmp file is 4-byte aligned by padding with zeros).
	int padding = (3 * bmpRGB->sizeX) % 4 ? 4 - (3 * bmpRGB->sizeX) % 4 : 0;

	// Add the padding to determine size of each scanline.
	int sizeScanline = 3 * bmpRGB->sizeX + padding;

	// Allocate storage for image in input bitmap file.
	int sizeStorage = sizeScanline * bmpRGB->sizeY;
	bmpRGB->data = new unsigned char[sizeStorage];

	// Read bmp file image data into input bitmap file.
	infile.seekg(offset);
	infile.read((char *)bmpRGB->data, sizeStorage);

	// Reverse color values from BGR (bmp storage format) to RGB.
	int startScanline, endScanlineImageData, temp;
	for (int y = 0; y < bmpRGB->sizeY; y++)
	{
		startScanline = y * sizeScanline; // Start position of y'th scanline.
		endScanlineImageData = startScanline + 3 * bmpRGB->sizeX; // Image data excludes padding.
		for (int x = startScanline; x < endScanlineImageData; x += 3)
		{
			temp = bmpRGB->data[x];
			bmpRGB->data[x] = bmpRGB->data[x + 2];
			bmpRGB->data[x + 2] = temp;
		}
	}

	// Set image width and height values and allocate storage for image in output bitmap file.
	bmpRGBA->sizeX = bmpRGB->sizeX;
	bmpRGBA->sizeY = bmpRGB->sizeY;
	bmpRGBA->data = new unsigned char[4 * bmpRGB->sizeX*bmpRGB->sizeY];

	// Copy RGB data from input to output bitmap files, set output A to 1.
	for (int j = 0; j < 4 * bmpRGB->sizeY * bmpRGB->sizeX; j += 4)
	{
		bmpRGBA->data[j] = bmpRGB->data[(j / 4) * 3];
		bmpRGBA->data[j + 1] = bmpRGB->data[(j / 4) * 3 + 1];
		bmpRGBA->data[j + 2] = bmpRGB->data[(j / 4) * 3 + 2];
		bmpRGBA->data[j + 3] = 0xFF;
	}

	return bmpRGBA;
}

GLfloat x = 2.0f;
GLfloat y = 2.0f;
GLfloat z = 2.0f;

// variables to move outermost Object Frame (to move all the rendered environment)
GLfloat moveX = 0.0f;
GLfloat moveY = 0.0f;
GLfloat moveZ = 0.0f;

// variables to rotate outermost Object Frame (to move all the rendered environment)
GLfloat rotX = 0.0f;
GLfloat rotY = 0.0f;
GLfloat rotZ = 0.0f;


// For animating the rotation of the objects
GLfloat animateRotation = 0.0f;

//variables to move the camera
GLfloat camY = 0.0f;
GLfloat camX = 0.0f;
GLfloat camZ = 0.0f;

// A quadratic object pointer used to draw the sides of the cylinder
GLUquadricObj *qobj;

static unsigned int texture[4]; // Array of texture indices.
static float d = 0.0; // Distance parameter in gluLookAt().

// Load external textures.
void loadExternalTextures()
{
	// Local storage for bmp image data.
	BitMapFile *image[4];

	// Load the images.
	image[0] = getbmp("H:/CS0308 Project/images/bg.bmp");
	image[1] = getbmp("H:/CS0308 Project/images/w1.bmp");
	image[2] = getbmp("H:/CS0308 Project/images/r.bmp");
	image[3] = getbmp("H:/CS0308 Project/images/tt.bmp");

	// Bind grass image to texture object texture[0]. 
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[0]->sizeX, image[0]->sizeY, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, image[0]->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Bind sky image to texture object texture[1]
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[1]->sizeX, image[1]->sizeY, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, image[1]->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[2]->sizeX, image[2]->sizeY, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, image[2]->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


}

//Function to initialize the lighting properties. you can add upto 8 lights in openGL (0-7)
void initLighting() {

	//Decalring the Ambient, Diffuse components of the LIght_0 and the position in the eye coordinate system
	GLfloat L0_Ambient[] = { 0.4, 0.4, 0.4, 1.0 };
	GLfloat L0_Diffuse[] = { 0.7, 0.7, 0.7, 1.0 };
	GLfloat L0_postion[] = { 5, 5, 0, 1.0 };

	glLightfv(GL_LIGHT0, GL_AMBIENT, L0_Ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, L0_Diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, L0_postion);

	GLfloat L1_Ambient[] = { 0.5, 0.5, 0.5, 1.0 };
	GLfloat L1_Diffuse[] = { 0.7, 0.7, 0.7, 1.0 };
	GLfloat L1_Specular[] = { 0.0, 1.0, 0.0, 1.0 };   //Declaration of the specular component of the light_1
	GLfloat L1_postion[] = { -5, 5, -15, 1.0 };

	glLightfv(GL_LIGHT1, GL_AMBIENT, L1_Ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, L1_Diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, L1_Specular);
	glLightfv(GL_LIGHT1, GL_POSITION, L1_postion);


	//Declaration of the ligt reflecting properties for the materials
	GLfloat specularReflectance[] = { 1.0, 1.0, 1.0, 1.0 };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specularReflectance);
	glMateriali(GL_FRONT, GL_SHININESS, 100);


	GLfloat L2_Ambient[] = { 0.5, 0.5, 0.5, 1.0 };
	GLfloat L2_Diffuse[] = { 0.7, 0.7, 0.7, 1.0 };
	GLfloat L2_Specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat L2_postion[] = { 0.0,0.0, 0.0, 1.0 };
	GLfloat L2_SpotDirection[] = { 1.0, 1.0,1.0 };

	glLightfv(GL_LIGHT2, GL_AMBIENT, L2_Ambient);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, L2_Diffuse);
	glLightfv(GL_LIGHT2, GL_SPECULAR, L2_Specular);
	glLightfv(GL_LIGHT2, GL_POSITION, L2_postion);

	//Creating a spotlight from light_2 by declaring the direction vetor and area that the spotligt can shine(fov of the spotlight)
	glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, L2_SpotDirection);
	glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 30.0);


}
void init() {

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);


	glGenTextures(2, texture);
	loadExternalTextures();

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glEnable(GL_LIGHTING);

	glShadeModel(GL_SMOOTH);

	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	initLighting();

	glEnable(GL_COLOR_MATERIAL);

	glEnable(GL_LIGHT0);

	glEnable(GL_NORMALIZE);


}

void DrawGrid() {
	GLfloat ext = 40.0f;
	GLfloat step = 1.0f;
	GLfloat yGrid = -0.4f;
	GLint line;

	glBegin(GL_LINES);
	for (line = -ext; line <= ext; line += step) {
		glVertex3f(line, yGrid, ext);
		glVertex3f(line, yGrid, -ext);
		glColor3f(1.0, 1.0, 1.0);
		glVertex3f(ext, yGrid, line);
		glVertex3f(-ext, yGrid, line);
	}
	glEnd();
}

void drawRoofBar()
{
	glLineWidth(1.8);
	glBegin(GL_TRIANGLE_STRIP);
	glLineWidth(5);
	glVertex3f(-12.0, 0.0, 0.0);
	glVertex3f(-11.0, 1.0, 0.0);
	glVertex3f(-10.0, 0.0, 0.0);
	glVertex3f(-9.0, 1.0, 0.0);
	glVertex3f(-8.0, 0.0, 0.0);
	glVertex3f(-7.0, 1.0, 0.0);
	glVertex3f(-6.0, 0.0, 0.0);
	glVertex3f(-5.0, 1.0, 0.0);
	glVertex3f(-4.0, 0.0, 0.0);
	glVertex3f(-3.0, 1.0, 0.0);
	glVertex3f(-2.0, 0.0, 0.0);
	glVertex3f(-1.0, 1.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(1.0, 1.0, 0.0);
	glVertex3f(2.0, 0.0, 0.0);
	glVertex3f(3.0, 1.0, 0.0);
	glVertex3f(4.0, 0.0, 0.0);
	glVertex3f(5.0, 1.0, 0.0);
	glVertex3f(6.0, 0.0, 0.0);
	glVertex3f(7.0, 1.0, 0.0);
	glVertex3f(8.0, 0.0, 0.0);
	glVertex3f(9.0, 1.0, 0.0);
	glVertex3f(10.0, 0.0, 0.0);
	glVertex3f(11.0, 1.0, 0.0);
	glVertex3f(12.0, 0.0, 0.0);


	glEnd();
}

void roof()
{

	for (GLint i = -45; i <= 25; i += 3) {
		glPushMatrix();
		glTranslatef(-10.0, 28.7, i);
		glRotatef(20.0, 0.0, 0.0, 1.0);
		drawRoofBar();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(10.0, 28.7, i);
		glRotatef(-20.0, 0.0, 0.0, 1.0);
		drawRoofBar();
		glPopMatrix();
	}

}



void drawCuboid(GLfloat x, GLfloat y, GLfloat z, GLfloat rt, GLfloat gt, GLfloat bt, GLfloat rb, GLfloat gb, GLfloat bb) {


	glColor3f(0.3, 0.3, 0.3);
	glBegin(GL_QUADS);
	glVertex3f(x, y, z);
	glVertex3f(x, -y, z);
	glVertex3f(-x, -y, z);
	glVertex3f(-x, y, z);
	glEnd();

	// FRONT
	glBegin(GL_QUADS);
	glVertex3f(x, y, -z);
	glVertex3f(-x, y, -z);
	glVertex3f(-x, -y, -z);
	glVertex3f(x, -y, -z);
	glEnd();

	// LEFT
	glBegin(GL_QUADS);
	glVertex3f(-x, -y, -z);
	glVertex3f(-x, y, -z);
	glVertex3f(-x, y, z);
	glVertex3f(-x, -y, z);
	glEnd();

	//Right
	glBegin(GL_QUADS);
	glVertex3f(x, y, z);
	glVertex3f(x, -y, z);
	glVertex3f(x, -y, -z);
	glVertex3f(x, y, -z);
	glEnd();

	//Top
	glBegin(GL_QUADS);
	glColor3f(rt, gt, bt);
	glVertex3f(x, y, -z);
	glVertex3f(x, y, z);
	glVertex3f(-x, y, z);
	glVertex3f(-x, y, -z);
	glEnd();

	//Bottom
	glBegin(GL_QUADS);
	glColor3f(rb, gb, bb);
	glVertex3f(x, -y, z);
	glVertex3f(x, -y, -z);
	glVertex3f(-x, -y, -z);
	glVertex3f(-x, -y, z);
	glEnd();

}
void drawBeam() {

	for (GLint i = -24; i <= 25; i += 6) {
		glPushMatrix();
		glTranslatef(-18.0, 7.0, i);
		drawCuboid(0.7, 7.0, 0.7, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(18.0, 7.0, i);
		drawCuboid(0.7, 7.0, 0.7, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2);
		glPopMatrix();
	}
}
GLfloat windowW = 6;
GLfloat windowH = 2;
GLfloat xstep = 0.05;
GLfloat ystep = 0.05;
GLfloat x1 = -3.0;


void Timer(int value) {

	if (x1 >= windowW  || x1 <= -windowW)
		xstep = -xstep;
	/*if (y_1 >= windowH - ssize || y_1 <= -windowH)
		ystep = -ystep;
	*/
	x1 += xstep;
	//y_1 += ystep;

	glutPostRedisplay();                // invoke a recall of the display function
	glutTimerFunc(30, Timer, 1);        // inorder to continue animating the Timer function is called within itself

}

void animateBall()
{
	
	glTranslatef(0.0, 0.2, 0.0);
	glutSolidSphere(0.1, 100, 100);

}

void drawTable() {

	drawCuboid(3.0, 0.1, 1.0, 0.0, 0.0, 0.2, 0.0, 0.0, 0.0);
	glTranslatef(0.0,0.2,0.0);
	glutSolidSphere(0.1,100,100);
	glTranslatef(-2.5, -1.2, -0.5);
	drawCuboid(0.1, 1.0, 0.1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
	glTranslatef(5.0, 0.0, 0.0);
	drawCuboid(0.1, 1.0, 0.1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
	glTranslatef(0.0, 0.0, 1.0);
	drawCuboid(0.1, 1.0, 0.1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
	glTranslatef(-5.0, 0.0, 0.0);
	drawCuboid(0.1, 1.0, 0.1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
	glTranslatef(2.5, 1.2, 0.5);
}

void display() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



	glPushMatrix();

	// camera orientation (eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ)
	gluLookAt(0.0 + camX, 2.0 + camY, 5.0 + camZ, 0, 0, 0, 0, 1.0, 0);

	// move the object frame using keyboard keys
	glTranslatef(moveX, moveY, moveZ);
	glRotatef(rotX, 1.0f, 0.0f, 0.0f);
	glRotatef(rotY, 0.0f, 1.0f, 0.0f);
	glRotatef(rotZ, 0.0f, 0.0f, 1.0f);

	//DrawGrid();
	//tables

	glPushMatrix();
	glTranslatef(-6.0, 3.0, 15.0);
	drawTable();
	glTranslatef(0.0, 0.0, 2.5);
	drawTable();
	glTranslatef(12, 0.0, -1.0);
	drawTable();
	glTranslatef(0.0, 0.0, -4.5);
	drawTable();
	glPopMatrix();


	glEnable(GL_TEXTURE_2D);
	//ground
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glBegin(GL_POLYGON);

	glTexCoord2f(0.0, 0.0); glVertex3f(-40.0, 0.0, 35.0);
	glTexCoord2f(1.0, 0.0);glVertex3f(40.0, 0.0, 35.0);
	glTexCoord2f(1.0, 1.0);glVertex3f(40.0, 0.0, -55.0);
	glTexCoord2f(0.0, 1.0);glVertex3f(-40.0, 0.0, -55.0);
	glEnd();

	glPushMatrix();
	glTranslatef(0.0, 0.0, 10.0);
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glBegin(GL_POLYGON);
	//court
	glTexCoord2f(0.0, 1.0); glVertex3f(-7.5, 1.0, 0.0);
	glTexCoord2f(0.0, 0.0);glVertex3f(7.5, 1.0, 0.0);
	glTexCoord2f(1.0, 0.0);glVertex3f(7.5, 1.0, -26.0);
	glTexCoord2f(1.0, 1.0);glVertex3f(-7.5, 1.0, -26.0);
	glEnd();


	//DrawGrid();
	glPopMatrix();

	glPushMatrix();
	//roof
	glTranslatef(11.0, 29.5, 0.0);
	glRotatef(-20.0, 0.0, 0.0, 1.0);

	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glBegin(GL_POLYGON);

	glTexCoord2f(0.0, 1.0); glVertex3f(-12, 0.0, 26.0);
	glTexCoord2f(0.0, 0.0);glVertex3f(12, 0.0, 26.0);
	glTexCoord2f(1.0, 0.0);glVertex3f(12, 0.0, -46.0);
	glTexCoord2f(1.0, 1.0);glVertex3f(-12, 0.0, -46.0);
	glEnd();

	glTranslatef(-21.0, -8.0, 0.0);
	glRotatef(40.0, 0.0, 0.0, 1.0);
	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glBegin(GL_POLYGON);

	glTexCoord2f(0.0, 1.0); glVertex3f(-12, 0.0, 25.5);
	glTexCoord2f(0.0, 0.0);glVertex3f(12, 0.0, 25.5);
	glTexCoord2f(1.0, 0.0);glVertex3f(12, 0.0, -46.5);
	glTexCoord2f(1.0, 1.0);glVertex3f(-12, 0.0, -46.5);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	glPushMatrix();


	glColor3f(0.2, 0.2, 0.2);
	roof();

	//floor
	glPushMatrix();
	glTranslatef(0.0, 0.0, -10.0);
	glColor3f(0.3, 0.3, 0.3);
	drawCuboid(20.0, 0.4, 36.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

	//backwall
	glTranslatef(0.0, 12.6, 34.0);
	glRotatef(-90.0, 1.0, 0.0, 0.0);
	drawCuboid(18.0, 1.0, 13.0, 0.0, 0.24, 0.0, 0.89, 0.8, 0.69);

	//office
	glTranslatef(0.0, 55.0, -5.5);
	drawCuboid(5.5, 5.0, 7.0, 0.0, 0.24, 0.0, 0.0, 0.24, 0.0);

	//front wall
	glTranslatef(-12.0, 13.0, 5.5);
	drawCuboid(6.0, 1.0, 13.0, 0.89, 0.8, 0.69, 0.0, 0.24, 0.0);
	glTranslatef(24.0, 0.0, 0.0);
	drawCuboid(6.0, 1.0, 13.0, 0.89, 0.8, 0.69, 0.0, 0.24, 0.0);

	glTranslatef(-12.0, 0.0, 6.0);
	drawCuboid(7.0, 1.0, 7.0, 0.89, 0.8, 0.69, 0.0, 0.24, 0.0);
	glPopMatrix();


	//walls
	glPushMatrix();
	glRotatef(-90.0, 0.0, 0.0, 1.0);
	glTranslatef(-7.0, 0.0, 0.0);
	glTranslatef(0.0, 12.0, 0.0);
	drawCuboid(7.0, 0.7, 24.0, 0.89, 0.8, 0.69, 0.0, 0.24, 0.0);
	glTranslatef(0.0, 6.0, -34.0);
	drawCuboid(7.0, 0.7, 10.0, 0.89, 0.8, 0.69, 0.0, 0.24, 0.0);
	glPopMatrix();

	glPushMatrix();
	glRotatef(-90.0, 0.0, 0.0, 1.0);
	glTranslatef(-7.0, 0.0, 0.0);
	glTranslatef(0.0, -12.0, 0.0);
	drawCuboid(7.0, 0.7, 24.0, 0.0, 0.24, 0.0, 0.89, 0.8, 0.69);
	glTranslatef(0.0, -6.0, -34.0);
	drawCuboid(7.0, 0.7, 10.0, 0.0, 0.24, 0.0, 0.89, 0.8, 0.69);
	glPopMatrix();

	//beams

	drawBeam();

	//second floor 
	glPushMatrix();
	glTranslatef(14.7, 14.0, 0.0);
	drawCuboid(5.0, 0.4, 24.25, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
	glTranslatef(-29.4, 0.0, 0.0);
	drawCuboid(5.0, 0.4, 24.25, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
	glPopMatrix();

	//sitting stairs
	glPushMatrix();
	glTranslatef(14.43, 15.0, 0.0);
	drawCuboid(3.0, 0.7, 24.25, 0.48, 0.0, 0.0, 0.0, 0.0, 0.0);
	glTranslatef(-29.1, 0.0, 0.0);
	drawCuboid(3.0, 0.7, 24.25, 0.48, 0.0, 0.0, 0.0, 0.0, 0.0);

	glTranslatef(30.0, 1.1, 0.0);
	drawCuboid(2.0, 0.7, 24.25, 0.48, 0.0, 0.0, 0.2, 0.2, 0.2);
	glTranslatef(-30.6, 0.0, 0.0);
	drawCuboid(2.0, 0.7, 24.25, 0.48, 0.0, 0.0, 0.2, 0.2, 0.2);

	glTranslatef(31.5, 1.4, 0.0);
	drawCuboid(1.2, 0.7, 24.25, 0.48, 0.0, 0.0, 0.2, 0.2, 0.2);
	glTranslatef(-32.5, 0.0, 0.0);
	drawCuboid(1.2, 0.7, 24.25, 0.48, 0.0, 0.0, 0.2, 0.2, 0.2);

	glTranslatef(33.0, 1.7, 0.0);
	drawCuboid(0.5, 0.7, 24.25, 0.48, 0.0, 0.0, 0.2, 0.2, 0.2);
	glTranslatef(-33.0, 0.0, 0.0);
	drawCuboid(0.5, 0.7, 24.25, 0.48, 0.0, 0.0, 0.2, 0.2, 0.2);
	
	glPopMatrix();

	//stairs
	glPushMatrix();
	glTranslatef(14.0, 7.0, -26.5);
	drawCuboid(4.0, 7.0, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
	glTranslatef(-28.0, 0.0, 0.0);
	drawCuboid(4.0, 7.2, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

	glTranslatef(14, 7.0, -8.0);
	drawCuboid(12.0, 0.3, 10.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
	glTranslatef(-14.0, -7.0, 0.0);
	glTranslatef(0.0, 0.0, 8.0);

	glTranslatef(28.5, -1.2, -2.8);
	drawCuboid(3.0, 6.0, 0.8, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
	glTranslatef(-28.5, 0.0, 0.0);
	drawCuboid(3.0, 6.0, 0.8, .0, 0.0, 0.0, 0.0, 0.0, 0.0);

	glTranslatef(28.5, -1.2, -1.6);
	drawCuboid(3.0, 5.0, 0.8, .0, 0.0, 0.0, 0.0, 0.0, 0.0);
	glTranslatef(-28.5, 0.0, 0.0);
	drawCuboid(3.0, 5.0, 0.8, .0, 0.0, 0.0, 0.0, 0.0, 0.0);

	glTranslatef(28.5, -1.0, -1.6);
	drawCuboid(3.0, 4.0, 0.8, .0, 0.0, 0.0, 0.0, 0.0, 0.0);
	glTranslatef(-28.5, 0.0, 0.0);
	drawCuboid(3.0, 4.0, 0.8, .0, 0.0, 0.0, 0.0, 0.0, 0.0);

	glTranslatef(28.5, -1.0, -1.6);
	drawCuboid(3.0, 3.0, 0.8, .0, 0.0, 0.0, 0.0, 0.0, 0.0);
	glTranslatef(-28.5, 0.0, 0.0);
	drawCuboid(3.0, 3.0, 0.8, .0, 0.0, 0.0, 0.0, 0.0, 0.0);

	glTranslatef(28.5, -1.0, -1.6);
	drawCuboid(3.0, 2.0, 0.8, .0, 0.0, 0.0, 0.0, 0.0, 0.0);
	glTranslatef(-28.5, 0.0, 0.0);
	drawCuboid(3.0, 2.0, 0.8, .0, 0.0, 0.0, 0.0, 0.0, 0.0);

	glTranslatef(28.5, -1.0, -1.6);
	drawCuboid(3.0, 1.2, 0.8, .0, 0.0, 0.0, 0.0, 0.0, 0.0);
	glTranslatef(-28.5, 0.0, 0.0);
	drawCuboid(3.0, 1.2, 0.8, .0, 0.0, 0.0, 0.0, 0.0, 0.0);

	glPopMatrix();



	//second floor walls
	glPushMatrix();
	glRotatef(-90.0, 0.0, 0.0, 1.0);

	glTranslatef(-19.6, -18.0, -10.0);
	drawCuboid(6.0, 0.7, 34.0, 0.0, 0.24, 0.0, 0.89, 0.8, 0.69);
	glTranslatef(0.0, 36.0, 0.0);
	drawCuboid(6.0, 0.7, 34.0, 0.89, 0.8, 0.69, 0.0, 0.24, 0.0);
	glPopMatrix();


	glPopMatrix();

	glutSwapBuffers();

}

void keyboardSpecial(int key, int x, int y) {
	if (key == GLUT_KEY_UP)
		moveY += 1;

	if (key == GLUT_KEY_DOWN)
		moveY -= 1;

	if (key == GLUT_KEY_LEFT)
		rotY -= 1.0;

	if (key == GLUT_KEY_RIGHT)
		rotY += 1.0;

	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
	if (key == 'w')
		moveZ += 0.5;
	if (key == 's')
		moveZ -= 0.5;


	if (key == 'a')
		moveX += 0.5;
	if (key == 'd')
		moveX -= 0.5;

	if (key == 'q')
		camY += 0.5;
	if (key == 'e')
		camY -= 0.5;



	if (key == 'c')
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (key == 'v')
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	if (key == 'k')
		glDisable(GL_LIGHT0);
	if (key == 'K')
		glEnable(GL_LIGHT0);
	if (key == 'l')
		glDisable(GL_LIGHT1);
	if (key == 'L')
		glEnable(GL_LIGHT1);
	if (key == 'j')
		glDisable(GL_LIGHT2);
	if (key == 'J')
		glEnable(GL_LIGHT2);


	glutPostRedisplay();

}

/*void Timer(int x) {
	animateRotation += animateRotation >= 360.0 ? -animateRotation : 5;
	glutPostRedisplay();

	glutTimerFunc(60, Timer, 1);
}
*/


void changeSize(GLsizei w, GLsizei h) {
	glViewport(0, 0, w, h);
	GLfloat aspect_ratio = h == 0 ? w / 1 : (GLfloat)w / (GLfloat)h;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//Define the Perspective projection frustum 
	// (FOV_in_vertical, aspect_ratio, z-distance to the near plane from the camera position, z-distance to far plane from the camera position)
	gluPerspective(120.0, aspect_ratio, 1.0, 100.0);

	/*if (w <= h)
		glOrtho(-55.0, 55.0, -55 / aspect_ratio, 55 / aspect_ratio, 55.0, -55.0);

	else
		glOrtho(-55.0* aspect_ratio, 55.0* aspect_ratio, -55.0, 55.0, 55.0, -55.0);
	*/
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


}

int main(int argc, char** argv) {

	glutInit(&argc, argv);
	glutCreateWindow("Gymnasium");
	glutInitWindowSize(100, 100);
	glutInitWindowPosition(150, 150);
	init();
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);

	glutDisplayFunc(display);
	glutReshapeFunc(changeSize);
	// keyboard function activation
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboardSpecial);

	glutTimerFunc(60.0, Timer, 1);
	glutMainLoop();


	return 0;
}
