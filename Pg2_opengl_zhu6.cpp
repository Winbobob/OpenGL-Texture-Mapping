#define GLUT_DISABLE_ATEXIT_HACK
#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <fstream>
#include <Gl/GL.h>
#include <GL/GLU.h>
#include <GL/glut.h>
#include "glm.h"
#include "FreeImage.h"

using namespace std;

//Global Variables
GLMmodel* pmodel = NULL;
unsigned char* data;
unsigned int width, height;
FREE_IMAGE_FORMAT fif = FIF_UNKNOWN; //file format
int bpp = 0;	//bit depth
static GLuint texName=0;
GLdouble projection[16], modelview[16], inverse[16];

float window[2];  //store the parameters from window.txt
float eye[3]={0, 0.5, 2};
float viewUp[3]={0, 0, 0};
float lookAt[3]={0, 1, 0};
GLfloat xt = 0.0f;//glTranslatef parameter 
GLfloat yt = 0.0f;//glTranslatef parameter 
GLfloat zt = 0.0f;//glScalef parameter
GLfloat xs = 1.0f;//glScalef parameter 
GLfloat ys = 1.0f;//glScalef parameter 
GLfloat zs = 1.0f;//glScalef parameter
GLfloat angleX = 0.0f;  //glRotatef parameter
GLfloat angleY = 0.0f;//glRotatef parameter
GLfloat angleZ = 0.0f;  //glRotatef parameter

GLfloat matambient[] = { 0.2, 0.2, 0.2, 1.0 };
GLfloat matdiffuse[] = { 0.8, 0.8, 0.8, 1.0 };
GLfloat matspecular[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat matshininess = 45.0;  //0.0~128.0

//original light source
GLfloat light0_position[]={ 2.0 , 2.0 , 2.0 , 1.0 };
GLfloat	light0_ambient[]={1.0 , 1.0 , 1.0, 1.0 };
GLfloat	light0_diffuse[]={ 1.0 , 1.0 , 1.0, 1.0 };
GLfloat	light0_specular[]={ 1.0 , 1.0 , 1.0, 1.0 };
//add light source
GLfloat addLight_position[4];
GLfloat	addLight_color[4];


void redisplay_all()
{
    glutPostRedisplay();
}

//can load BMP, JPEG, PNG, TIFF, etc.
bool loadPic(const char* path, unsigned char*& data, unsigned int& width, unsigned int& height,FREE_IMAGE_FORMAT& fif, int& bpp)
{
	//initial FreeImage
    FreeImage_Initialise(TRUE);
	//image format
	fif = FIF_UNKNOWN;
	//pointer to the image, once loaded
	FIBITMAP *dib(0);
	//check the file signature and deduce its format
	fif = FreeImage_GetFileType(path, 0);
	//if still unknown, try to guess the file format from the file extension
	if(fif == FIF_UNKNOWN) 
		fif = FreeImage_GetFIFFromFilename(path);
	//if still unkown, return failure
	if(fif == FIF_UNKNOWN)
		return false;

	//check that the plugin has reading capabilities and load the file
	if(FreeImage_FIFSupportsReading(fif))
		dib = FreeImage_Load(fif, path);
	//if the image failed to load, return failure
	if(!dib)
		return false;
	//retrieve the image data
	data = FreeImage_GetBits(dib);
	//get the image width, height and bit depth
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);
	bpp = FreeImage_GetBPP(dib);
	//if this somehow one of these failed (they shouldn't), return failure
	if((data == 0) || (width == 0) || (height == 0))
		return false;
}

int loadBMP(const char* path, unsigned char*& data, unsigned int& width, unsigned int& height)
{

	// Data read from the header of the BMP file
	unsigned char header[54]; // Each BMP file begins by a 54-bytes header
	unsigned int dataPos;     // Position in the file where the actual data begins
	unsigned int imageSize;   // = width*height*3
	// Open the file in binary mode
	FILE* fp = fopen(path,"rb");
	if (!fp)                              
	{
		cout<<"Image could not be opened\n"; 
		return 0;
	}

	if ( fread(header, 1, 54, fp)!=54 )
	{ 
		// If not 54 bytes read : problem
		cout<<"Not a correct BMP file\n";
		return 0;
	}

	if ( header[0]!='B' || header[1]!='M' )
	{
		cout<<"Not a correct BMP file\n";
		return 0;
	}

	// Read ints from the byte array
	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize==0)    
		imageSize=(width)*(height)*3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos==0)      
		dataPos=54; // The BMP header is done 

	// Create a buffer
	data = new unsigned char [imageSize];

	// Read the actual data from the file into the buffer
	if (fread(data,1,imageSize,fp) != imageSize)
	{
		fclose(fp);
		return 0;
	}

	//Everything is in memory now, the file can be closed
	fclose(fp);
	return 1;
}

// Load different obj files
void Load_Obj_and_Texture(int value)
{
    char* name = 0; //name of obj file
	char* name1 = 0;//name of texture file
    GLint params[2];
    switch (value) {
 
    case 'al':
        name = "inputs/al.obj";
        break;
    case 'cube':
        name = "inputs/cube.obj";
        break;
	case 'ca':
        name = "inputs/castle.obj";
        break;
   case 'do':
        name = "inputs/dolphins.obj";
        break;
    break;
    }
    
    if (name) {
        pmodel = glmReadOBJ(name);
        if (!pmodel) exit(0);
        glmUnitize(pmodel);
        glmFacetNormals(pmodel);
        glmVertexNormals(pmodel, 90.0);
    }

	switch (value) {
 
    case 'koa':
        name1 = "inputs/Koala.bmp";
        break;
    case 'cam':
        name1 = "inputs/camouflage.bmp";
        break;
	case 'jos':
        name1 = "inputs/Joseph Gordon-Levitt.jpg";
        break;
	case 'IU':
        name1 = "inputs/Lee Ji Eun.tif";
        break;
	case 'gri':
        name1 = "inputs/grid.png";
        break;

    break;
    }

    if (name1) {
        if (!loadPic(name1,data, width, height,fif,bpp))
			cout<<"Error loading texture\n";
		else
			cout<<"Loaded texture successfully!\n";
		if((fif == FIF_PNG)||(fif == FIF_BMP && bpp != 24))
		{
			//in order to load the PNG file successfully, we have to use GL_RGBA and GL_BGRA_EXT
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);
		}
    }

	switch (value) {
 
    case 'on':
        glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
        break;
	case 'off':
        glDisable(GL_COLOR_MATERIAL);
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
        break;
	}
    redisplay_all();
}

//read the txt file line number
int lineNum(char file[])
{
	ifstream infile(file);  //The default file open mode is "open" in the form of output
	char str[100];			//N is a constant, the purpose is to read a long enough line
	int n=0;				//line number
	while(!infile.eof())
	{
		infile.getline(str, sizeof(str));//The default identifier for termination is¡®\n¡¯
		n++;
	}
	 return n; 
}

//read window.txt file
void readWindow()
{
	ifstream windowFile;
	windowFile.open("inputs/window.txt");
	for (int j=0;j<2;j++)
	{
		windowFile>>(float)window[j];
	}
	windowFile.close();
}

//read view.txt file
void readView()
{
	ifstream windowFile;
	windowFile.open("inputs/view.txt");
	for (int j=0;j<3;j++)
	{
		windowFile>>(float)eye[j];
	}
	for (int j=0;j<3;j++)
	{
		windowFile>>(float)viewUp[j];
	}
	for (int j=0;j<3;j++)
	{
		windowFile>>(float)lookAt[j];
	}
	windowFile.close();
}

//read lights.txt file
void readLights()
{
	int LineNum = lineNum("inputs/lights.txt");
	ifstream windowFile;
	windowFile.open("inputs/lights.txt");
	for(int i=1; i<=LineNum; i++)
	{
		for (int j=0;j<4;j++)
		{
			windowFile>>(float)addLight_position[j];
		}
		for (int j=0;j<4;j++)
		{
			windowFile>>(float)addLight_color[j];
		}

		//OpenGL can add at most 8 light sources
		switch (i) {
		case 1:
			glLightfv(GL_LIGHT1, GL_POSITION, addLight_position ); 
			glLightfv(GL_LIGHT1, GL_AMBIENT, addLight_color);
			glLightfv(GL_LIGHT1, GL_DIFFUSE, addLight_color);
			glLightfv(GL_LIGHT1, GL_SPECULAR,addLight_color);
			glEnable(GL_LIGHT1);
			break;
		case 2:
			glLightfv(GL_LIGHT2, GL_POSITION, addLight_position ); 
			glLightfv(GL_LIGHT2, GL_AMBIENT, addLight_color);
			glLightfv(GL_LIGHT2, GL_DIFFUSE, addLight_color);
			glLightfv(GL_LIGHT2, GL_SPECULAR,addLight_color);
			glEnable(GL_LIGHT2);
			break;
		case 3:
			glLightfv(GL_LIGHT3, GL_POSITION, addLight_position ); 
			glLightfv(GL_LIGHT3, GL_AMBIENT, addLight_color);
			glLightfv(GL_LIGHT3, GL_DIFFUSE, addLight_color);
			glLightfv(GL_LIGHT3, GL_SPECULAR,addLight_color);
			glEnable(GL_LIGHT3);
			break;
		case 4:
			glLightfv(GL_LIGHT4, GL_POSITION, addLight_position ); 
			glLightfv(GL_LIGHT4, GL_AMBIENT, addLight_color);
			glLightfv(GL_LIGHT4, GL_DIFFUSE, addLight_color);
			glLightfv(GL_LIGHT4, GL_SPECULAR,addLight_color);
			glEnable(GL_LIGHT4);
			break;
		case 5:
			glLightfv(GL_LIGHT5, GL_POSITION, addLight_position ); 
			glLightfv(GL_LIGHT5, GL_AMBIENT, addLight_color);
			glLightfv(GL_LIGHT5, GL_DIFFUSE, addLight_color);
			glLightfv(GL_LIGHT5, GL_SPECULAR,addLight_color);
			glEnable(GL_LIGHT5);
			break;
		case 6:
			glLightfv(GL_LIGHT6, GL_POSITION, addLight_position ); 
			glLightfv(GL_LIGHT6, GL_AMBIENT, addLight_color);
			glLightfv(GL_LIGHT6, GL_DIFFUSE, addLight_color);
			glLightfv(GL_LIGHT6, GL_SPECULAR,addLight_color);
			glEnable(GL_LIGHT6);
			break;
		case 7:
			glLightfv(GL_LIGHT7, GL_POSITION, addLight_position ); 
			glLightfv(GL_LIGHT7, GL_AMBIENT, addLight_color);
			glLightfv(GL_LIGHT7, GL_DIFFUSE, addLight_color);
			glLightfv(GL_LIGHT7, GL_SPECULAR,addLight_color);
			glEnable(GL_LIGHT7);
			break;
		}
	}
	windowFile.close();
}

/*  planes for texture coordinate generation  */
static GLfloat plane1[] = {1.0,0.0,1.0,0.0};
static GLfloat plane2[] = {0.0,1.0,1.0,0.0};

static GLfloat *currentCoeff;
static GLenum currentPlane;
static GLint currentGenMode;

void init()
{
	//initial FreeImage
    FreeImage_Initialise(TRUE);

	//the defination of light
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matdiffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matspecular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, matshininess);	

	glLightfv(GL_LIGHT0, GL_POSITION, light0_position ); 
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR,light0_specular);
	//Attenuation
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.3);        // c Index
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.8);         // l Index
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.8);     // q Index

	glEnable(GL_COLOR_MATERIAL);
	//glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//add aditional light sources
	readLights();

	//load bmp picture
	if(!data)
	{
		if (!loadPic("inputs/Lee Ji Eun.tif",data, width, height,fif, bpp))
			cout<<"Error loading texture\n";
		else
			cout<<"Loaded texture successfully!\n";
	}

	//Texture generate
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_SMOOTH);	//Gouraud Shading Model 
	glEnable(GL_DEPTH_TEST);	
	//specify pixels are byte-aligned while unpacking from memory
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &texName);//generate
	glBindTexture(GL_TEXTURE_2D, texName);//bind to a 2-D texture

	//wrap the image along s coordinate (width)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//wrap the image along t coordinate (height)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//how to treat magnification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//how to treat de-magnification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//set texture mapping method
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	//bmp stores BGR instead of RGB
	if((fif == FIF_PNG)||(fif == FIF_BMP && bpp != 24))
	{
		//in order to load the PNG file successfully, we have to use GL_RGBA and GL_BGRA_EXT
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, data);
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);
	}

	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	currentGenMode = GL_OBJECT_LINEAR;
	currentPlane = GL_OBJECT_PLANE;
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, currentGenMode);
	glTexGenfv(GL_S, currentPlane, plane1);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, currentGenMode);
	glTexGenfv(GL_T, currentPlane, plane2);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_GEN_S);
 	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);

}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (!pmodel) {
        pmodel = glmReadOBJ("inputs/castle.obj");
        if (!pmodel) exit(0);
        glmUnitize(pmodel);
        glmFacetNormals(pmodel);
        glmVertexNormals(pmodel, 90.0); //calculate smooth vertex normals of the model

    }

	glPushMatrix();	//glBegin and glEnd functions should put inside between glPushMatrix and glPopMatrix
	//set transform
	glScalef(xs,ys,zs);
	glTranslatef(xt,yt,zt); //change the position of image
	glRotatef(angleX,1.0f,0.0f,0.0f);
	glRotatef(angleY,0.0f,1.0f,0.0f);
	glRotatef(angleZ,0.0f,0.0f,1.0f);
	glmDraw(pmodel, GLM_SMOOTH | GLM_MATERIAL);

	//set texture
	//glEnable(GL_TEXTURE_2D);
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
	//glBindTexture(GL_TEXTURE_2D, texName);
	//glBegin(GL_QUADS);
	//
	//	glTexCoord2f(0.0, 0.0); glVertex3f(-1.0f,-1.0f, 1.0f);
	//	glTexCoord2f(1.0, 0.0); glVertex3f( 1.0f,-1.0f, 1.0f);
	//	glTexCoord2f(1.0, 1.0); glVertex3f( 1.0f, 1.0f, 1.0f);
	//	glTexCoord2f(0.0, 1.0); glVertex3f(-1.0f, 1.0f, 1.0f);
	//
	//
	//	
	//glEnd();

	glPopMatrix();
	glFlush();
	glutSwapBuffers();
}

void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);//the upper left point coordinate, the width and height of window
	glMatrixMode(GL_PROJECTION);//Projection Matrix: when we want to change projection, we have to use GL_PROJECTION
	glLoadIdentity();//change current matrix to identity matrix, move original point to the center of screen
	gluPerspective(60, (GLfloat) w/(GLfloat) h, 0.1, 5000.0);//void gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble near, GLdouble far)
	//fovy is view angle on YZ-plane£¬its valus is in [0.0,180.0]; aspect is screen ratio, width divided by height
	glMatrixMode(GL_MODELVIEW);//when we want to draw sth. on the screen, we have to use GL_MODELVIEW
	glLoadIdentity();
	gluLookAt(eye[0],eye[1],eye[2],viewUp[0],viewUp[1],viewUp[2],lookAt[0],lookAt[1],lookAt[2]);
}

void keyboard (unsigned char key, int x, int y)
{
	switch (key) 
	{
		case 27:    //Use "Esc" to quit
			exit(0);
			break;
		case 'z':   //Use "z" to zoom-in/scale-up the object
			xs += 0.1f;
			ys += 0.1f;
			zs += 0.1f;
			break;
		case 'x':   //Use "x" to zoom-out/scale-down the object
			xs -= 0.1f;
			ys -= 0.1f;
			zs -= 0.1f;
			break;
		case '[':   //Use "[" to move object in +Z
			zt += 0.1f;  
			break;
		case ']':   //Use "]" to move object in -Z
			zt -= 0.1f;  
			break;
		case 'q':   //Use "q" to rotate the view in clockwise direction(Z axis)
			angleZ -= 5.0f;  
			break;
		case 'w':   //Use "w" to rotate the view in anti-clockwise direction(Z axis)
			angleZ += 5.0f;  
			break;
		case 'e':   //Use "e" to rotate the view in clockwise direction(Y axis)
			angleY -= 5.0f;  
			break;
		case 'r':   //Use "r" to rotate the view in anti-clockwise direction(Y axis)
			angleY += 5.0f;  
			break;
		case 't':   //Use "t" to rotate the view in clockwise direction(X axis)
			angleX -= 5.0f;  
			break;
		case 'y':   //Use "y" to rotate the view in anti-clockwise direction(X axis)
			angleX += 5.0f;  
			break;
		default:
			break;
	}redisplay_all();
}

void specialkeyboard (int key, int x, int y)
{
	switch (key) 
	{
		case GLUT_KEY_LEFT:

			 xt -= 0.1f;  
			 break;
		case GLUT_KEY_RIGHT:
			 xt += 0.1f;  
			 break;
		case GLUT_KEY_UP:
			 yt += 0.1f;  
			 break;
		case GLUT_KEY_DOWN:
			 yt -= 0.1f;  
			 break;
		default:
			break;
	}redisplay_all();
}

int main(int argc, char** argv)
{
	printf("Mouse right kick for load different objects and textures \n\n");
	printf("Use 'Esc' to quit \n");
	printf("Use 'z' to zoom-in/scale-up the object \n");
	printf("Use 'x' to zoom-out/scale-down the object \n");
	printf("Use '[' to move object in +Z \n");
	printf("Use ']' to move object in -Z \n");
	printf("Use 'q' to rotate the view in clockwise direction (Z axis)\n");
	printf("Use 'w' to rotate the view in anti-clockwise direction (Z axis) \n");
	printf("Use 'e' to rotate the view in clockwise direction (Y axis)\n");
	printf("Use 'r' to rotate the view in anti-clockwise direction (Y axis) \n");
	printf("Use 't' to rotate the view in clockwise direction (X axis)\n");
	printf("Use 'y' to rotate the view in anti-clockwise direction (X axis) \n");
	printf("move with arrow keys\n");

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE); //set display mode:RGBA, Depth Buffer, double buffering
	readWindow();//read external window.txt
	readView();  //read external view.txt
	glutInitWindowSize(window[0],window[1]); //set window size
	glutInitWindowPosition(700, 80);
	glutCreateWindow("Pg2_opengl_zhu6"); //set window title
	init();
	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);//void glutReshapeFunc(void (*func)(int width, int height));
	//we can define a function, receiving 2 parameters in order to avoid content distortion when changing the window size
	
	
	glutKeyboardFunc(keyboard);//void glutKeyboardFunc(void (*func)(unsigned char key, int x, int y));
	//it has 3 parameters. 1st is to receive the ASCII value of keyboard, 2nd and 3rd is the mouse position on the screen
	glutSpecialFunc(specialkeyboard);

	//add menu
	glutCreateMenu(Load_Obj_and_Texture);
	glutAddMenuEntry("---------Load---------", ' ');
    glutAddMenuEntry("al", 'al');
    glutAddMenuEntry("cube", 'cube');
    glutAddMenuEntry("castle", 'ca');
    glutAddMenuEntry("dolphins", 'do');

	glutAddMenuEntry("---------Texture---------", ' ');
   
	glutAddMenuEntry("grid.png", 'gri');
    glutAddMenuEntry("Lee Ji Eun.tif", 'IU');
    glutAddMenuEntry("Joseph Gordon.jpg", 'jos');
    glutAddMenuEntry("Koala(24-bit).bmp", 'koa');
    glutAddMenuEntry("camouflage(32-bit).bmp", 'cam');
	
	glutAddMenuEntry("-----------Light----------", ' ');
    glutAddMenuEntry("Turn on", 'on');
    glutAddMenuEntry("Turn off", 'off');
    glutAttachMenu(GLUT_RIGHT_BUTTON);

	redisplay_all();
	glutMainLoop();
	
	delete data;
	return 0; 
}