#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h> 
#include "gl/glut.h"

#include "fileReader.h";
#include "histogramTable.h"
#include "boudingBox.h"
#include "distAdjust.h"

#define STRING_SIZE 500
#define MAX_LAYER_LIMIT 12

#define GRID_SIZE 40
#define VOXEL_VIEW_SIZE 5
#define  PI   3.141592653
#define Step  50

typedef int menu_t;
menu_t top_m, BB_m, file_m;


/*----Parameters for isoValue----*/
float frequency[MAX_LAYER_LIMIT];
char num[10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
int flag = 0;
int startIsoVal = 0, endIsoVal = 0;

bool show_waterMarking = false;
bool showHist = true;
bool showSelectedLayer[MAX_LAYER_LIMIT];
bool showLayer[MAX_LAYER_LIMIT];
bool showSelectedBB = false;
bool completeModel = false;

float model_angle[3] = { 0, 0, 0 };
int model_pos[3] = {0, 0, 0};
float bb_angle[3] = { 0, 0, 0 };
float bb_pos[3] = { 0, 0, 0 };

int temp_x = -1, temp_y = -1;

VolumeData_t distData, modelData;
BoundingBox_t *selectionBox;
Point_t* waterMarkingData;
int waterMarkingSize = 0;


/*----Set the color of layers----*/
int layer_color[2][3] = { // 0 - 11 layer
	{255, 255, 255},
	{0, 191, 255}
};
 /*-----Define a unit box--------*/
 /* Vertices of the box */
float  points[][3] = { {-0.5, -0.5, -0.5}, {0.5, -0.5, -0.5},
					  {0.5, 0.5, -0.5}, {-0.5, 0.5, -0.5},
					  {-0.5, -0.5, 0.5}, {0.5, -0.5, 0.5},
					  {0.5, 0.5, 0.5}, {-0.5, 0.5, 0.5} };
/* face of box, each face composing of 4 vertices */
int    face[][4] = { {0, 3, 2, 1}, {0, 1, 5, 4}, {1, 2, 6, 5},
					{4, 5, 6, 7}, {2, 3, 7, 6}, {0, 4, 7, 3} };
/* indices of the box faces */
int    cube[6] = { 0, 1, 2, 3, 4, 5 };

/*-Declare GLU quadric objects, sphere, cylinder, and disk --*/
GLUquadricObj* sphere = NULL, * cylind = NULL, * disk;


/*-----Define window size----*/
int width = 1400, height = 1000;

/*-----Translation and rotations of eye coordinate system---*/
float   eyeDx = 0.0, eyeDy = 0.0, eyeDz = 0.0; 
float   eyeAngx = 0.0, eyeAngy = 0.0, eyeAngz = 0.0;
double  Eye[3] = { 5.0*GRID_SIZE , 7.0* GRID_SIZE, 20.0* GRID_SIZE }, Focus[3] = { 0.0, -20.0* GRID_SIZE, 0.0 }, Vup[3] = { 0.0, 1.0, 0.0 };

float   u[3][3] = { {1.0,0.0,0.0}, {0.0,1.0,0.0}, {0.0,0.0,1.0} };//x y z
float   eye[3];
float   cv, sv; /* cos(5.0) and sin(5.0) */
float   near_dis = 20, far_dis = 200* GRID_SIZE; /*view volume*/
int     show_view_volume = 0;
int		show_global = 0;

/*----get the histogram frequency----*/
void compute() {

	//char infWaterFileName[STRING_SIZE] = "C:/Users/Chin/Desktop/VolumeData/Voxel_01/bell_dist.inf";
	//char rawWaterFileName[STRING_SIZE] = "C:/Users/Chin/Desktop/VolumeData/Voxel_01/bell_dist.raw";
	//readInfFile(infWaterFileName, &waterMarking);
	//readRawFile(rawWaterFileName, &waterMarking);
	//printf("\n");

	char infDistFileName[STRING_SIZE] = "C:/Users/Chin/Desktop/VolumeData/Voxel_01/bell_dist.inf";
	char rawDistFileName[STRING_SIZE] = "C:/Users/Chin/Desktop/VolumeData/Voxel_01/bell_dist.raw";

	char infModelFileName[STRING_SIZE] = "C:/Users/Chin/Desktop/VolumeData/Voxel_01/bell_model.inf";
	char rawModelFileName[STRING_SIZE] = "C:/Users/Chin/Desktop/VolumeData/Voxel_01/bell_model.raw";

	/*char infDistFileName[STRING_SIZE] = "C:/Users/Chin/Desktop/VolumeData/WaterMarking/bell_dist.inf";
	char rawDistFileName[STRING_SIZE] = "C:/Users/Chin/Desktop/VolumeData/WaterMarking/bell_dist_1to1_watermarked.raw";

	char infModelFileName[STRING_SIZE] = "C:/Users/Chin/Desktop/VolumeData/Voxel_04/bell_model.inf";
	char rawModelFileName[STRING_SIZE] = "C:/Users/Chin/Desktop/VolumeData/Voxel_04/bell_model.raw";*/

	readInfFile(infDistFileName, &distData);
	readRawFile(rawDistFileName, &distData);
	printf("\n");
	readInfFile(infModelFileName, &modelData);
	readRawFile(rawModelFileName, &modelData);
	adjustDistValue(&modelData, &distData);

	float* f = computeHistogram(&distData);


	for (int i = 0; i < 12; i++) {
		frequency[i] = *(f + i);
	}

	showLayer[0] = true;
	for (int i = 1; i < MAX_LAYER_LIMIT; i++) {
		showLayer[i] = false;
	}
	for (int i = 0; i < MAX_LAYER_LIMIT; i++) {
		showSelectedLayer[i] = false;
	}

	for (int i = 0; i < 3; i++) {
		model_pos[i] = distData.resolution[i] / 2;
		Focus[i] = model_pos[i];
		Eye[i] = model_pos[i];
	}
	Eye[2] = Eye[2] * (distData.resolution[2]/GRID_SIZE)/2;
}


void draw_scene(void);

/*----------------------------------------------------------
 * Procedure to initialize the working environment.
 */
void  myinit()
{
	glClearColor(0.0, 0.0, 0.0, 1.0);      /*set the background color BLACK */
					 /*Clear the Depth & Color Buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POINT_SIZE);
	/*---Create quadratic objects---*/
	if (sphere == NULL) {
		sphere = gluNewQuadric();
		gluQuadricDrawStyle(sphere, GLU_FILL);
		gluQuadricNormals(sphere, GLU_SMOOTH);
	}
	if (cylind == NULL) {
		cylind = gluNewQuadric();
		gluQuadricDrawStyle(cylind, GLU_FILL);
		gluQuadricNormals(cylind, GLU_SMOOTH);
	}
	if (disk == NULL) {
		disk = gluNewQuadric();
		gluQuadricDrawStyle(disk, GLU_FILL);
		gluQuadricNormals(disk, GLU_SMOOTH);
	}

	/*---- Compute cos(5.0) and sin(5.0) ----*/
	cv = cos(5.0 * PI / 180.0);
	sv = sin(5.0 * PI / 180.0);
	/*---- Copy eye position ---*/
	eye[0] = Eye[0];
	eye[1] = Eye[1];
	eye[2] = Eye[2];
}

/*--------------------------------------------------------
 * Procedure to draw a 1x1x1 cube. The cube is within
 * (-0.5,-0.5,-0.5) ~ (0.5,0.5,0.5)
 */
void draw_cube()
{
	int    i;

	for (i = 0; i < 6; i++) {
		glBegin(GL_POLYGON);  /* Draw the face */
		glVertex3fv(points[face[i][0]]);
		glVertex3fv(points[face[i][1]]);
		glVertex3fv(points[face[i][2]]);
		glVertex3fv(points[face[i][3]]);
		glEnd();
	}
}

/*------------------------------------------------------------------
 * Procedure to draw a sphere.
 */
void draw_sphere(float radius)
{
	if (sphere == NULL) {
		sphere = gluNewQuadric();
		gluQuadricDrawStyle(sphere, GLU_FILL);
	}
	gluSphere(sphere, (GLdouble)radius,   /* radius=1.5 */
		20,            /* composing of 12 slices*/
		12);           /* composing of 12 stacks */
}

/*------------------------------------------------------------------
 * Procedure to draw a cylinder.
 */
void draw_cylinder(float top_r, float bottom_r, float height)
{
	if (cylind == NULL) {
		cylind = gluNewQuadric();
		gluQuadricDrawStyle(cylind, GLU_FILL);
	}
	/*--- Draw a cylinder ---*/
	gluCylinder(cylind, (GLdouble)top_r, (GLdouble)bottom_r, /* radius of top and bottom circle */
		(GLdouble)height,              /* height of the cylinder */
		12,               /* use 12-side polygon approximating circle*/
		3);               /* Divide it into 3 sections */

}

/*---------------------------------------------------------
 * Procedure to draw the floor.
 */
void draw_floor()
{
	float chess_dark_color[3] = { 0.1, 0.1, 0.2 };
	float chess_light_color[3] = { 0.8, 0.8, 0.8 };
	int color_index = 0;

	for (int i = 1; i <= 100; i++) {
		for (int k = 1; k <= 100; k++) {
			if ((i + k) % 2 == 0) glColor3fv(chess_dark_color);
			else glColor3fv(chess_light_color);
			glNormal3f(0.0, 1.0, 0.0);
			glBegin(GL_POLYGON);
			glVertex3f((i - 25.0) * GRID_SIZE, 0.0, (k - 25.0) * GRID_SIZE);
			glVertex3f((i - 25.0) * GRID_SIZE, 0.0, (k - 26.0) * GRID_SIZE);
			glVertex3f((i - 26.0) * GRID_SIZE, 0.0, (k - 26.0) * GRID_SIZE);
			glVertex3f((i - 26.0) * GRID_SIZE, 0.0, (k - 25.0) * GRID_SIZE);
			glEnd();
		}
	}
}

/*-------------------------------------------------------
 * Procedure to draw three axes and the orign
 */
void draw_axes() {
	/*----Draw a white sphere to represent the original-----*/
	glColor3f(0.9, 0.6, 0.9);

	gluSphere(sphere, 0.01*GRID_SIZE,   /* radius=2.0 */
		12,            /* composing of 12 slices*/
		12);           /* composing of 8 stacks */

  /*----Draw three axes in colors, yellow, meginta, and cyan--*/
  /* Draw Z axis  */
	glColor3f(0.0, 0.0, 0.95);
	gluCylinder(cylind, 0.01* GRID_SIZE, 0.01 * GRID_SIZE, /* radius of top and bottom circle */
		10.0* GRID_SIZE,              /* height of the cylinder */
		12,               /* use 12-side polygon approximating circle*/
		3);               /* Divide it into 3 sections */

/* Draw Y axis */
	glPushMatrix();
	glRotatef(-90.0, 1.0, 0.0, 0.0);  /*Rotate about x by -90', z becomes y */
	glColor3f(0.0, 0.95, 0.0);
	gluCylinder(cylind, 0.01 * GRID_SIZE, 0.01 * GRID_SIZE, /* radius of top and bottom circle */
		10.0* GRID_SIZE,             /* height of the cylinder */
		12,               /* use 12-side polygon approximating circle*/
		3);               /* Divide it into 3 sections */
	glPopMatrix();

	/* Draw X axis */
	glColor3f(0.95, 0.0, 0.0);
	glPushMatrix();
	glRotatef(90.0, 0.0, 1.0, 0.0);  /*Rotate about y  by 90', x becomes z */
	gluCylinder(cylind, 0.01 * GRID_SIZE, 0.01 * GRID_SIZE,   /* radius of top and bottom circle */
		10.0* GRID_SIZE,             /* height of the cylinder */
		12,               /* use 12-side polygon approximating circle*/
		3);               /* Divide it into 3 sections */
	glPopMatrix();
	/*-- Restore the original modelview matrix --*/
}

/*-------------------------------------------------------
 * Make viewing matrix
 */
void make_view(int x)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	switch (x) {
	case 4:       /* Perspective */
	  /* In this sample program, eye position and Xe, Ye, Ze are computed
		 by ourselves. Therefore, use them directly; no trabsform is
		 applied upon eye coordinate system
		 */
		gluLookAt(eye[0], eye[1], eye[2],
			eye[0] - u[2][0], eye[1] - u[2][1], eye[2] - u[2][2],
			u[1][0], u[1][1], u[1][2]);
		break;
	case 1:       /* X direction parallel viewing */
		gluLookAt(GRID_SIZE, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
		break;
	case 2:       /* Y direction parallel viewing */
		gluLookAt(0.0, 39.9 * GRID_SIZE, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0);
		break;
	case 3:       /* Z direction parallel viewing */
		gluLookAt(0.0, 0.0, 24.9, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
		break;
	}
}

/*------------------------------------------------------
 * Procedure to make projection matrix
 */
void make_projection(int x){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (x == 4) {
		gluPerspective(45.0, (double)width / (double)height, near_dis, far_dis);
	}
	else {
		if (width > height)
			glOrtho(-40.0 * GRID_SIZE,
				40.0 * GRID_SIZE,
				-40.0 * GRID_SIZE * (float)height / (float)width,
				40.0 * GRID_SIZE * (float)height / (float)width,
				-0.0,
				100.0 * GRID_SIZE);
		else
			glOrtho(-40.0 * GRID_SIZE * (float)width / (float)height,
				40.0 * GRID_SIZE * (float)width / (float)height,
				-40.0 * GRID_SIZE,
				40.0 * GRID_SIZE,
				-0.0,
				100.0 * GRID_SIZE);
	}
	glMatrixMode(GL_MODELVIEW);
}

/*---------------------------------------------------------
 * Procedure to draw distanced model
 */
void draw_model() {
	//glEnable(GL_POINT_SIZE);

	if (distData.distField->layerCoord != NULL) {
		glPushMatrix();
		for (int i = 0; i < MAX_LAYER_LIMIT; i++) {//layer number 0 - 11
			if (showLayer[i]) {
				int layerNum = distData.distField->histTable[i].frequency;
				if (i == 0) { 
					glPointSize(1.0); 
					glColor3f(layer_color[0][0] / 255.0, layer_color[0][1] / 255.0, layer_color[0][2] / 255.0);
				}
				else { 
					glPointSize(10.0); 
					glColor3f(layer_color[1][0] / 255.0, layer_color[1][1] / 255.0, layer_color[1][2] / 255.0);
				}
				glBegin(GL_POINTS);
				for (int k = 0; k < layerNum; k++) { // the voxels amount of layer
					Point_t data = distData.distField->layerCoord[i][k];
					glVertex3f((float)data.x, (float)data.y, (float)data.z);
				}
				glEnd();
			}
		}
		glPopMatrix();
	}
}

void draw_waterMarking() {
	if (waterMarkingSize > 0) {
		glPushMatrix();
		glPointSize(10.0);
		glColor3f(1.0, 0.0, 0.0);

		glBegin(GL_POINTS);
		for (int i = 0; i < waterMarkingSize; i++) {
			glVertex3f((float)waterMarkingData[i].x, (float)waterMarkingData[i].y, (float)waterMarkingData[i].z);
		}
		glEnd();
		glPopMatrix();
	}
}


/*---------------------------------------------------------
* Subprocedure to draw the lines of bounding box
*
* point:coordinate of bounding box
*/
void draw_BB(int point[][3]) {
	for (int i = 0; i < 8; i++) {
		glBegin(GL_POINTS);
		glVertex3f(point[i][0], point[i][1], point[i][2]);
		glEnd();
	}

	for (int i = 0; i < 4; i++) {
		glBegin(GL_LINES);
		glVertex3f(point[i][0], point[i][1], point[i][2]);
		glVertex3f(point[i + 4][0], point[i + 4][1], point[i + 4][2]);
		glEnd();

		glBegin(GL_LINES);
		glVertex3f(point[i][0], point[i][1], point[i][2]);
		glVertex3f(point[(i + 1) % 4][0], point[(i + 1) % 4][1], point[(i + 1) % 4][2]);
		glEnd();
		int k = i + 4;
		glBegin(GL_LINES);
		glVertex3f(point[k][0], point[k][1], point[k][2]);
		if ((k + 1) % 8 == 0) k = 3;
		glVertex3f(point[(k + 1) % 8][0], point[(k + 1) % 8][1], point[(k + 1) % 8][2]);
		glEnd();
	}
}

/*---------------------------------------------------------
* Draw the coordinate of selection bounding box
*/
void draw_selectionBB() {
	if (selectionBox != NULL) {
		int* max, * min;
		if (selectionBox->max != NULL && selectionBox->min != NULL) {
			max = selectionBox->max;
			min = selectionBox->min;
			int point[8][3] = {
				{min[0],min[1],min[2]},{max[0],min[1],min[2]},{max[0],max[1],min[2]},{min[0],max[1],min[2]},
				{min[0],min[1],max[2]},{max[0],min[1],max[2]},{max[0],max[1],max[2]},{min[0],max[1],max[2]}
			};
			glColor3f(0.0, 1.0, 0.0);
			draw_BB(point);

			printf("\nSelecting box\nmin:  ");
			for (int i = 0; i < 3; i++) {
				printf("%d	", min[i]);
			}
			printf("\nmax:  ");
			for (int i = 0; i < 3; i++) {
				printf("%d	", max[i]);
			}
			printf("\n");
		}
	}
}

/*---------------------------------------------------------
* Transfer rectangle and define its height
*
* x:x position
* y:y position
* z:z position
* h:histogram height defined by its frequency
*/
void draw_rectangle(float x, float y, float z, float h) {
	glTranslatef(x, y, z);
	glBegin(GL_POLYGON);
	glVertex2f(0.0, 0.0);
	glVertex2f(36.0, 0.0);
	glVertex2f(36.0, (h * 1000));
	glVertex2f(0.0, (h * 1000));
	glEnd();
}

/*---------------------------------------------------------
* Print number on the screen
*
* nx:x position
* ny:y position
* num:number want to show
*/
void showNum(float nx, float ny, char num) {
	glRasterPos2f(nx, ny);
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, num);
}

/*--------------------------------------------------------
* Draw histogram on the screen
*/
void draw_histogram() {
	float hx = -1550, nx = -1540;
	float hy = 600.0, ny = 550.0;
	int index;

	/*----draw vertical and horizontal line----*/
	glBegin(GL_LINES);
	glColor3f(1.0, 1.0, 1.0);
	glVertex2f(hx - 72, hy);
	glVertex2f(hx + 872, hy);
	glEnd();

	/*----draw all histograms in different position----*/
	for (index = 0; index < 12; index++, hx += 72) {
		glPushMatrix();
		draw_rectangle(hx, hy, 0.0, frequency[index]);
		glPopMatrix();
	}
	/*----print histogram numbers----*/
	for (index = 0; index < 10; index++, nx += 72)
		showNum(nx, ny, num[index]);
	showNum(nx -= 10.0, ny, num[1]);
	showNum(nx += 20.0, ny, num[0]);
	showNum(nx += 55.0, ny, num[1]);
	showNum(nx += 20.0, ny, num[1]);
}

/*--------------------------------------------------------
 * Display callback procedure to draw the scene
 */
void display(){
	/*Clear previous frame and the depth buffer */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, width, height);
	make_view(4);//perspective
	make_projection(4);
	draw_scene();
	
	if (showHist == true) {
		/*----Draw histogram----*/
		make_view(3);//z
		make_projection(3);
		glColor3f(1.0, 1.0, 1.0);
		draw_histogram();
	}

	/*-------Swap the back buffer to the front --------*/
	glutSwapBuffers();
	return;
}

/*---------------------------------------------------------
* Draw the bounding box of model
*/
void draw_model_BB() {
	int resol[3] = { 0 };
	for (int i = 0; i < 3; i++)
		resol[i] = distData.resolution[i];
	int point[8][3] = {
		{0, 0, 0}, {resol[0],0 ,0}, {resol[0],0 ,resol[2]}, {0, 0, resol[2]},
		{0, resol[1], 0}, {resol[0],resol[1] ,0}, {resol[0],resol[1] ,resol[2]}, {0, resol[1], resol[2]}
	};
	glColor3f(1.0, 0.0, 0.0);
	glPointSize(5.0);
	glBegin(GL_POINTS);
	glVertex3f(model_pos[0], model_pos[1], model_pos[2]);
	glEnd();
	glColor3f(0.0, 1.0, 1.0);
	draw_BB(point);
}

/*---------------------------------------------------------
* Draw voxels in the selection bounding box
*/
void draw_selectedLayer() {
	if (selectionBox != NULL) {
		for (int isoVal = startIsoVal; isoVal <= endIsoVal; isoVal++) {
			int layerNum = distData.distField->histTable[isoVal].frequency;
			Point_t* temp = distData.distField->layerCoord[isoVal];
			if (isoVal == 0) {
				glPointSize(1.0);
				glColor3f(layer_color[0][0] / 255.0, layer_color[0][1] / 255.0, layer_color[0][2] / 255.0);
			}
			else {
				glPointSize(3.5);
				glColor3f(layer_color[1][0] / 255.0, layer_color[1][1] / 255.0, layer_color[1][2] / 255.0);
			}
			glBegin(GL_POINTS);
			for (long long int i = 0; i < layerNum; i++) {
				int selected = temp[i].selected;
				if (selected == 1) {
					int x = temp[i].x;
					int y = temp[i].y;
					int z = temp[i].z;
					glVertex3f((float)x, (float)y, (float)z);
				}
			}
			glEnd();
		}
	}
}

/*---------------------------------------------------------
 * This procedure draw the car
 */
void draw_scene(){
	
	glRotatef(-90, 1, 0, 0);

	glTranslated(model_pos[0], model_pos[1], model_pos[2]);
	glRotatef(model_angle[0], 1, 0, 0);
	glRotatef(model_angle[1], 0, 1, 0);
	glRotatef(model_angle[2], 0, 0, 1);

	glPushMatrix();
	glTranslated(-model_pos[0], -model_pos[1], -model_pos[2]);
	if (show_global == 1) {
		draw_axes();
	}
	if (show_waterMarking) draw_waterMarking();
	draw_model_BB();
	draw_selectionBB();
	if (showSelectedLayer) draw_selectedLayer();
	draw_model();
	glPopMatrix();
	
}

/*---------------------------------------------------------
 * Reshape callback function which defines the size
 * of the main window when a reshape event occurrs.
 */
void my_reshape(int w, int h){

	width = w;
	height = h;

	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90.0, (double)w / (double)h, near_dis, far_dis);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/*---------------------------------------------------------
* Change the show value
*/
void changeShowLayer(bool * layer) {
	if (*layer == false) *layer = true;
	else *layer = false;
}

/*--------------------------------------------------
 * Keyboard callback func. When a 'q' key is pressed,
 * the program is aborted.
 */
void my_keyboard(unsigned char key, int ix, int iy){
	int    i;
	float  x[3], y[3], z[3];

	if (key == 'Q' || key == 'q') exit(0);
	/*------transform the EYE coordinate system ------*/
	else if (key == 'u') {            /* rolling */
		eyeAngz += 5.0 * GRID_SIZE;
		if (eyeAngz > 360.0) eyeAngz -= 360.0;
		for (i = 0; i < 3; i++) {
			x[i] = cv * u[0][i] - sv * u[1][i];
			y[i] = sv * u[0][i] + cv * u[1][i];
		}
		for (i = 0; i < 3; i++) {
			u[0][i] = x[i];
			u[1][i] = y[i];
		}
	}
	else if (key == 'U') {
		eyeAngz += -5.0 * GRID_SIZE;
		if (eyeAngz < 0.0) eyeAngz += 360.0;
		for (i = 0; i < 3; i++) {
			x[i] = cv * u[0][i] + sv * u[1][i];
			y[i] = -sv * u[0][i] + cv * u[1][i];
		}
		for (i = 0; i < 3; i++) {
			u[0][i] = x[i];
			u[1][i] = y[i];
		}
	}
	else if (key == 'v') { /* change the near clipping windows*/
		if (near_dis <= 10) {
			near_dis += 0.2;
		}
	}
	else if (key == 'V') {
		if (near_dis >= 1.5) {
			near_dis -= 0.2;
		}
	}
	else if (key == 'g') {
		if (show_global == 0) {
			show_global = 1;
		}
		else {
			show_global = 0;
		}
	}
	else if (key == '0') {
		changeShowLayer(&showLayer[0]);
	}
	else if (key == '1') {
		changeShowLayer(&showLayer[1]);
	}
	else if (key == '2') {
		changeShowLayer(&showLayer[2]);
	}
	else if (key == '3') {
		changeShowLayer(&showLayer[3]);
	}
	else if (key == '4') {
		changeShowLayer(&showLayer[4]);
	}
	else if (key == '5') {
		changeShowLayer(&showLayer[5]);
	}
	else if (key == '6') {
		changeShowLayer(&showLayer[6]);
	}
	else if (key == '7') {
		changeShowLayer(&showLayer[7]);
	}
	else if (key == '8') {
		changeShowLayer(&showLayer[8]);
	}
	else if (key == '9') {
		changeShowLayer(&showLayer[9]);
	}
	else if (key == '/') {
		changeShowLayer(&showLayer[10]);
	}
	else if (key == '*') {
		changeShowLayer(&showLayer[11]);
	}
	
	if (selectionBox != NULL) {
		if (key == 'z') {//large z-dir 1 unit
			if (selectionBox->max[2] + 1 < distData.resolution[2])
				selectionBox->max[2] += 1;
		}
		else if (key == 'Z') {//shrink z-dir 1 unit
			if (selectionBox->max[2] - 1 > selectionBox->min[2])
				selectionBox->max[2] -= 1;
		}
		else if (key == 'x') {//large x-dir 1 unit
			if (selectionBox->max[0] + 1 < distData.resolution[0])
				selectionBox->max[0] += 1;
		}
		else if (key == 'X') {//shrink x-dir 1 unit
			if (selectionBox->max[0] - 1 > selectionBox->min[0])
				selectionBox->max[0] -= 1;
		}
		else if (key == 'c') {//large y-dir 1 unit
			if (selectionBox->max[1] + 1 < distData.resolution[1])
				selectionBox->max[1] += 1;
		}
		else if (key == 'C') {//shrink y-dir 1 unit
			if (selectionBox->max[1] - 1 > selectionBox->min[1])
				selectionBox->max[1] -= 1;
		}
		else if (key == 'd' || key == 'D') {//move + x-dir 1 unit
			if (selectionBox->max[0] + 1 < distData.resolution[0]) {
				selectionBox->max[0] += 1;
				selectionBox->min[0] += 1;
			}
		}
		else if (key == 'a' || key == 'A') {//move - x-dir 1 unit
			if (selectionBox->min[0] - 1 > 0) {
				selectionBox->max[0] -= 1;
				selectionBox->min[0] -= 1;
			}
		}
		else if (key == 'w' || key == 'W') {//move + y-dir 1 unit
			if (selectionBox->max[1] + 1 < distData.resolution[1]) {
				selectionBox->max[1] += 1;
				selectionBox->min[1] += 1;
			}
		}
		else if (key == 's' || key == 'S') {//move - y-dir 1 unit
			if (selectionBox->min[1] - 1 > 0) {
				selectionBox->max[1] -= 1;
				selectionBox->min[1] -= 1;
			}
		}
		else if (key == 'r' || key == 'R') {//move + z-dir 1 unit
			if (selectionBox->max[2] + 1 < distData.resolution[2]) {
				selectionBox->max[2] += 1;
				selectionBox->min[2] += 1;
			}
		}
		else if (key == 'f' || key == 'F') {//move - z-dir 1 unit
			if (selectionBox->min[2] - 1 > 0) {
				selectionBox->max[2] -= 1;
				selectionBox->min[2] -= 1;
			}
		}
	}


	display();
}

void special_keyboard(int key, int x, int y) {
	int i;
	if (key == GLUT_KEY_UP) {
		eyeDy += 0.5;       /* move up */
		for (i = 0; i < 3; i++) eye[i] += 0.5 * u[1][i] * GRID_SIZE;
	}
	else if (key == GLUT_KEY_DOWN) {
		eyeDy += -0.5;       /* move down */
		for (i = 0; i < 3; i++) eye[i] -= 0.5 * u[1][i] * GRID_SIZE;
	}
	else if (key == GLUT_KEY_LEFT) {
		eyeDx += -0.5;       /* move left*/
		for (i = 0; i < 3; i++) eye[i] -= 0.5 * u[0][i] * GRID_SIZE;
	}
	else if (key == GLUT_KEY_RIGHT) {
		eyeDx += 0.5;        /* move right */
		for (i = 0; i < 3; i++) eye[i] += 0.5 * u[0][i] * GRID_SIZE;
	}

	display();
}

void my_mouse(int button, int state, int x, int y) {
	int i;
	
	if (button == 3 && state == GLUT_DOWN) {//scroll up
		eyeDz += 0.5;        /* zoom in */
		for (i = 0; i < 3; i++) eye[i] -= 0.5 * u[2][i] * GRID_SIZE;
	}
	else if (button == 4 && state == GLUT_DOWN) {//scroll down
		eyeDz += -0.5;       /* zoom out */
		for (i = 0; i < 3; i++) eye[i] += 0.5 * u[2][i] * GRID_SIZE;
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		temp_x = x;
		temp_y = y;
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		temp_x = temp_y = -1;
	}


	display();
}

void my_motion(int x, int y) {
	
	if (temp_x != -1 && temp_y != -1) {
		int x_offset = temp_x - x;
		int y_offset = temp_y - y;
		model_angle[2] -= 0.01 * x_offset;
		model_angle[0] -= 0.01 * y_offset;
		if (model_angle[2] >= 360.0) model_angle[2] = 0.0;
		else if(model_angle[2]<=0.0f) model_angle[2] = 360.0;
		if (model_angle[0] >= 360.0) model_angle[0] = 0.0;
		else if (model_angle[0] <= 0.0f) model_angle[0] = 360.0;
	}
	else {
		temp_x = x, temp_y = y;
	}

	display();
}

void reset() {
	startIsoVal = 0;
	endIsoVal = 0;
	showLayer[0] = true;
	for (int i = 1; i < MAX_LAYER_LIMIT; i++)
		showLayer[i] = false;
	for (int i = 0; i < MAX_LAYER_LIMIT; i++)
		showSelectedLayer[i] = false;
}

void selection(int index) {
	switch (index){
	case 1://Input the isoVal
		reset();
		printf("\nPlease input the layer number (0 - 11) of histogram: \n(startLayer, endLayer) =  ");
		scanf("%d %d", &startIsoVal, &endIsoVal);
		while (startIsoVal > 11 || endIsoVal > 11) {
			printf("\nValue over 11, please input (0 - 11) again: \n(startLayer, endLayer) =");
			scanf("%d %d", &startIsoVal, &endIsoVal);
		}

		printf("\nChoose the ( %d , %d)layer\n", startIsoVal, endIsoVal);
		for (int i = 0; i < MAX_LAYER_LIMIT; i++) {
			showLayer[i] = false;
		}
		for (int i = startIsoVal; i <= endIsoVal; i++) {
			showLayer[i] = true;
		}
		break;
	case 2:/*------Choose the layer of model and creat BB ------*/
		if (selectionBox == NULL)
			selectionBox = (BoundingBox_t*)malloc(sizeof(BoundingBox_t));

		createBoundingBox(&distData, selectionBox);
		break;
	case 3: //Show selected layers
		if (selectionBox != NULL) {
			float *rp = checkValue(selectionBox, startIsoVal, endIsoVal, &distData);
			free(rp);
			for (int i = 0; i < MAX_LAYER_LIMIT; i++){
				showLayer[i] = false;
				showSelectedLayer[i] = false;
			}
			for (int i = startIsoVal; i < endIsoVal; i++) {
				showSelectedLayer[i] = true;
			}
		}
		else printf("\nPlease create selecting box and input isoVal first\n");
		break;
	default:
		break;
	}
	display();
}

void main_menu(int index) {
	switch (index){
	case 0://reset
		reset();
		break;
	case 1: //save raw file, type : float
		if (selectionBox != NULL) {
			char outputRawFileName[STRING_SIZE];
			printf("\nPlease input the output file name: ");
			scanf("%s", outputRawFileName);
			
			float *rp = checkValue(selectionBox, startIsoVal, endIsoVal, &distData);
			writeFloatRaw(&distData, rp, outputRawFileName);
			free(rp);
		}
		else printf("\nPlease create selecting box and input isoVal first\n");
	break;
	case 2:
		exit(0);
		break;
	case 3://show histogram or not
		showHist = (showHist) ? false : true;
		break;
	case 4://show waterMarking
		if (waterMarkingData == NULL) {
			waterMarkingData = getWaterMarkingPos(&distData);
			waterMarkingSize = getWaterMarkingSize(&distData);
		}
		show_waterMarking = (show_waterMarking) ? false : true;
		break;
	case 5://generate completed model, type : unsigned char
		completeModel = (completeModel) ? false : true;
		if (completeModel) {
			char outputRawFileName[STRING_SIZE];
			printf("\nPlease input the output file name: ");
			scanf("%s", outputRawFileName);
			unsigned char* rp = generateModel(&distData);
			writeCharRaw(&distData, rp, outputRawFileName);
			free(rp);
		}
		break;
	default:
		break;
	}
	display();
}

/*---------------------------------------------------
 * Main procedure which defines the graphics environment,
 * the operation style, and the callback func's.
 */
void main(int argc, char** argv){
	compute();
	/*-----Initialize the GLUT environment-------*/
	glutInit(&argc, argv);

	/*-----Depth buffer is used, be careful !!!----*/
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(width, height);
	glutCreateWindow("Show distanced model");

	myinit();      /*---Initialize other state varibales----*/

	/*----Associate callback func's whith events------*/
	glutDisplayFunc(display);
	glutReshapeFunc(my_reshape);
	glutKeyboardFunc(my_keyboard);
	glutSpecialFunc(special_keyboard);
	glutMouseFunc(my_mouse);
	glutMotionFunc(my_motion);

	/*----Create selection menu----*/
	int selection_m = glutCreateMenu(selection);
	glutAddMenuEntry("Input iso value", 1);
	glutAddMenuEntry("Set selecting box", 2);
	glutAddMenuEntry("Show selected layer", 3);

	/*----Create menu----*/
	int menu = glutCreateMenu(main_menu);
	glutAddSubMenu("Selection", selection_m);
	glutAddMenuEntry("Show histogram", 3);
	glutAddMenuEntry("Show waterMarking", 4);
	glutAddMenuEntry("Generate model", 5);
	glutAddMenuEntry("Reset", 0);
	glutAddMenuEntry("Save raw file", 1);
	glutAddMenuEntry("Quit", 2);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
	
	glutMainLoop();
}